//
//  fuse_multiplex.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/6/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <stdlib.h>
#include <errno.h>

#include "fuse_multiplex.h"
#include "mediaRAID.h"
#include "simplehash.h"
#include "volumes.h"
#include "httpd.h"
#include "data_structs.h"
#include "files.h"
#include "replication.h"

struct fuse_operations fuse_oper_struct = {
	.init           = multiplex_init,
	.getattr        = multiplex_getattr,
	.fgetattr       = multiplex_fgetattr,
	.readdir        = multiplex_readdir,
	.mknod          = multiplex_mknod,
	.create         = multiplex_create,
	.open           = multiplex_open,
	.read           = multiplex_read,
	.write          = multiplex_write,
	.release        = multiplex_release,
	.rename         = multiplex_rename,
	.unlink         = multiplex_unlink,
	.rmdir          = multiplex_rmdir,
	.mkdir          = multiplex_mkdir,
	.chmod          = multiplex_chmod,
	.chown          = multiplex_chown,
	.access         = multiplex_access,
	.truncate       = multiplex_truncate,
	.utimens        = multiplex_utimens,
	.setxattr       = multiplex_setxattr,
	.getxattr       = multiplex_getxattr,
	.listxattr      = multiplex_listxattr,
	.removexattr    = multiplex_removexattr,
	.statfs         = multiplex_statfs,
};


void *multiplex_init(struct fuse_conn_info *conn) {

	/* Init random number gen */
	srand((unsigned int)time(0));
	
	/* Initialization of open table lookup */
	init_open_fh_table();
	
	/* Initialize httpd daemon */
	start_httpd_daemon(g_application_options.server_port);
	
	/* Initialize replication thread */
	replication_start();
	
	EXLog(FUSE, INFO, "multiplex_init finished");
	
	return 0;
}

int multiplex_getattr(const char *path, struct stat *stbuf) {
	memset(stbuf, 0, sizeof(struct stat));
	
	EXLog(FUSE, INFO, "multiplex_getattr [%s]", path);
	
	/* Check if the file is open, if so, it's the active file */
	char tmppath[PATH_MAX];
	int64_t fh;
	if (get_open_fh_for_path(path, &fh, tmppath)) {
		/* We should return the stat for the active file */
		RaidVolume_t *active_volume = volume_with_basepath(tmppath);
		volume_full_path_for_raid_path(active_volume, path, tmppath);
		if (!stat(tmppath, stbuf)) {
			return 0;
		}
	}
	
	/* If file is closed, we have a helper function that pulls the stat struct from the most recently modified file in the active volume list */
	return volume_most_recently_modified_instance(path, NULL, NULL, stbuf);
}

int multiplex_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi) {
	EXLog(FUSE, INFO, "multiplex_fgetattr wrapper called [%s]", path);
	return multiplex_getattr(path, stbuf);
}

int multiplex_statfs(const char *path, struct statvfs *statbuf) {
	EXLog(FUSE, INFO, "multiplex_statfs [%s]", path);
	return volume_statvfs(path, statbuf);
}

int multiplex_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
					  off_t offset, struct fuse_file_info *fi) {
	
	EXLog(FUSE, INFO, "multiplex_readdir [%s]", path);

	/* Fill out basic dot dirs */
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	/* Get all the DIR entries for the active volumes */
	DIR **entries = volume_active_dir_entries(path);
	DIR **entries_start = entries;
	if (entries) {
		/* create a hash table that will help detect duplicate entries */
		Dictionary_t *dic = dictionary_create_with_size(512);
		int64_t tmp;
		while (*entries) {
			/* For each DIR, iterate through and grab entries */
			DIR *entry = *entries;
			struct dirent *dent = readdir(entry);
			while (dent) {
				
				if (strcmp("..", dent->d_name) && strcmp(".", dent->d_name)) {
					/* For each entry, add it to the list if it doesn't exist in the hash table */
					if (!dictionary_get_int(dic, dent->d_name, &tmp)) {
						dictionary_set_int(dic, dent->d_name, tmp);
						EXLog(FUSE, DBG, "Adding dir entry [%s]", dent->d_name);
						filler(buf, dent->d_name, NULL, 0);
					}
				}
				
				dent = readdir(entry);
			}
			closedir(entry); /* Don't forget to close the DIR after use! */
			entries++;
		}
		/* Free memory */
		dictionary_destroy(dic);
		free(entries_start);
	}
	
	return 0;
}

int multiplex_mknod(const char *path, mode_t mode, dev_t dev) {
	
	EXLog(FUSE, DBG, "multiplex_mknod [%s]", path);
	
	/* Check if the file exists */
	if (!volume_most_recently_modified_instance(path, NULL, NULL, NULL)) {
		return -EEXIST;
	}
	
	/* Get the volume with the most free space */
	RaidVolume_t *volume = volume_with_most_bytes_free();
	if (!volume) {
		return -ENOENT;
	}
	
	char fullpath[PATH_MAX];
	volume_full_path_for_raid_path(volume, path, fullpath);
	return mknod(fullpath, mode, dev);
}

int multiplex_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
	
	EXLog(FUSE, DBG, "multiplex_create [%s]", path);
	
	/* Check if the file exists */
	if (!volume_most_recently_modified_instance(path, NULL, NULL, NULL)) {
		return -EEXIST;
	}
	
	/* Get the volume with the most free space */
	RaidVolume_t *volume = volume_with_most_bytes_free();
	if (!volume) {
		return -ENOENT;
	}
	
	char fullpath[PATH_MAX];
	volume_full_path_for_raid_path(volume, path, fullpath);
	int64_t fh = open(fullpath, O_CREAT | fi->flags, mode);
	fi->fh = fh;
	
	/* Add fh to dictionary */
	if (fh >= 0) {
		set_open_fh_for_path(path, fh, volume->basepath);
	}
	
	return 0;
}

int multiplex_open(const char *path, struct fuse_file_info *fi) {
	
	EXLog(FUSE, DBG, "multiplex_open [%s]", path);
	
	/* If already open, return the existing */
	int64_t fh;
	if (get_open_fh_for_path(path, &fh, NULL)) {
		fi->fh = fh;
		return 0;
	}
		
	/* Otherwise get the most recently modified version */
	char fullpath[PATH_MAX];
	RaidVolume_t *volume;
	if (volume_most_recently_modified_instance(path, &volume, fullpath, NULL)) {
		/* No file found! */
		return -ENOENT;
	}
	
	/* We have the full filepath */
	fh = open(fullpath, fi->flags);
	fi->fh = fh;
	
	if (fh >= 0) {
		/* Open was successful, let's put it in the hash */
		set_open_fh_for_path(path, fh, volume->basepath);
		
		/* If we're opening the file, then halt replication */
		replication_halt_replication_of_file(path);
	}
			
	return 0;
}

int multiplex_read(const char *path, char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	
	EXLog(FUSE, DBG, "multiplex_read [%s | %d]", path, (int)size);
	
	/* Assume fh is valid in fi */
	if (lseek((int)fi->fh, offset, SEEK_SET) < 0) {
		close((int)fi->fh);
		set_open_fh_for_path(path, -1, NULL);
		return -1;
	}
	
	/* Otherwise read */
	int res = (int)read((int)fi->fh, buf, size);
	if (res < 0) {
		return -errno;
	}
	return res;
}

int multiplex_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	
	EXLog(FUSE, DBG, "multiplex_write [%s | %d]", path, (int)size);
	
	/* Assume fh is valid in fi */
	if (lseek((int)fi->fh, offset, SEEK_SET) < 0) {
		close((int)fi->fh);
		set_open_fh_for_path(path, -1, NULL);
		return -1;
	}
	
	/* Otherwise write */
	int res = (int)write((int)fi->fh, buf, size);
	if (res < 0) {
		return -errno;
	}
	return res;
}

int multiplex_release(const char *path, struct fuse_file_info *fi) {
	
	EXLog(FUSE, DBG, "multiplex_release [%s]", path);
	
	/* Close file handle and remove from dictionary */
	set_open_fh_for_path(path, -1, NULL);
	
	int res = close((int)fi->fh);
	
	/* Queue task to balance the file */
	ReplicationTask_t task;
	replication_task_init(&task);
	task.opcode = REP_OP_BALANCE_FILE;
	strncpy(task.path, path, PATH_MAX);
	replication_queue_task(&task, OP_PRI_SYNC_FILE, 1);
	
	return res;
}

int multiplex_rename(const char *oldpath, const char *newpath) {
	
	EXLog(FUSE, DBG, "multiplex_rename [%s -> %s]", oldpath, newpath);
	
	/* halt replication */
	replication_halt_replication_of_file(oldpath);
	replication_halt_replication_of_file(newpath);
	
	return volume_rename_path_on_active_volumes(oldpath, newpath);
}

int multiplex_unlink(const char *path) {
	
	EXLog(FUSE, DBG, "multiplex_unlink [%s]", path);
	
	/* halt replication */
	replication_halt_replication_of_file(path);
	
	/* We have a helper for this */
	return volume_unlink_path_from_active_volumes(path);
}

int multiplex_access(const char *path, int amode) {
	
	EXLog(FUSE, DBG, "multiplex_access [%s]", path);
	
	/* We have a helper for this */
	return volume_access_path_from_active_volumes(path, amode);
}

int multiplex_rmdir(const char *path) {
	
	EXLog(FUSE, DBG, "multiplex_rmdir [%s]", path);
	
	/* We have a helper for this */
	return volume_rmdir_path_from_active_volumes(path);
}

int multiplex_mkdir(const char *path, mode_t mode) {
	
	EXLog(FUSE, DBG, "multiplex_mkdir [%s]", path);
	
	/* We have a helper for this */
	return volume_mkdir_path_on_active_volumes(path, mode);
}

int multiplex_chmod(const char *path, mode_t mode) {

	EXLog(FUSE, DBG, "multiplex_chmod [%s]", path);
	
	/* halt replication */
	replication_halt_replication_of_file(path);
	
	/* We have a helper for this */
	return volume_chmod_path_on_active_volumes(path, mode);
}

int multiplex_chown(const char *path, uid_t uid, gid_t gid) {

	EXLog(FUSE, DBG, "multiplex_chown [%s]", path);
	
	/* halt replication */
	replication_halt_replication_of_file(path);
	
	/* We have a helper for this */
	return volume_chown_path_on_active_volumes(path, uid, gid);
}

int multiplex_truncate(const char *path, off_t length) {
	
	EXLog(FUSE, DBG, "multiplex_truncate [%s | %lld]", path, length);
	
	/* If already open, attempt to truncate the existing */
	int64_t fh;
	char basepath[PATH_MAX];
	char fullpath[PATH_MAX];
	if (get_open_fh_for_path(path, &fh, basepath)) {
		if (!ftruncate((int)fh, length)) {
			return 0;
		}
		
		/* File open for reading?  Regardless, it's open on a filesystem so let's try to truncate that one */
		RaidVolume_t *volume = volume_with_basepath(basepath);
		if (!volume) return -1;
				
		volume_full_path_for_raid_path(volume, path, fullpath);
		int res = truncate(fullpath, length);
		if (res < 0) res = -errno;
		return res;
	} else {
		/* halt replication of possible background copy */
		replication_halt_replication_of_file(path);
	}
	
	/* Otherwise manually truncate it from whichever was modified last */
	volume_most_recently_modified_instance(path, NULL, fullpath, NULL);
	int res = truncate(fullpath, length);
	if (res < 0) res = -errno;
	
	/* Queue task to balance the file */
	ReplicationTask_t task;
	replication_task_init(&task);
	task.opcode = REP_OP_BALANCE_FILE;
	strncpy(task.path, path, PATH_MAX);
	replication_queue_task(&task, OP_PRI_SYNC_FILE, 1);
	
	return res;
}

int multiplex_utimens(const char *path, const struct timespec tv[2]) {
	
	EXLog(FUSE, DBG, "multiplex_utimens [%s]", path);
	
	/* halt replication */
	replication_halt_replication_of_file(path);
	
	/* We have a helper for this */
	return volume_utimens_path_on_active_volumes(path, tv);
}

int multiplex_setxattr(const char *path, const char *name, const char *value, size_t size, int options __APPLE_XATTR_POSITION__ ) {

	EXLog(FUSE, DBG, "multiplex_setxattr [%s | %s | %s]", path, name, value);
	
	/* halt replication */
	replication_halt_replication_of_file(path);
	
	/* We have a helper for this */
	return volume_setxattr_path_on_active_volumes(path, name, value, size, options __APPLE_XATTR_POSITION_P__ );
}

int multiplex_getxattr(const char *path, const char *name, char *value, size_t size __APPLE_XATTR_POSITION__ ) {
	
	EXLog(FUSE, DBG, "multiplex_getxattr [%s | %s]", path, name);
	
	/* We have a helper for this */
	return volume_getxattr_path_on_active_volumes(path, name, value, size, 0 __APPLE_XATTR_POSITION_P__ );
}

int multiplex_listxattr(const char *path, char *namebuf, size_t size) {

	EXLog(FUSE, DBG, "multiplex_listxattr [%s]", path);
		
	/* We have a helper for this */
	return volume_listxattr_path_on_active_volumes(path, namebuf, size);
}

int multiplex_removexattr(const char *path, const char *name) {
	
	EXLog(FUSE, DBG, "multiplex_removexattr [%s | %s]", path, name);
	
	/* halt replication */
	replication_halt_replication_of_file(path);
	
	/* We have a helper for this */
	return volume_removexattr_path_on_active_volumes(path, name);
}












