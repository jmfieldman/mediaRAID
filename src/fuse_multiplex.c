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
#include <errno.h>

#include "fuse_multiplex.h"
#include "mediaRAID.h"
#include "simplehash.h"
#include "volumes.h"
#include "httpd.h"
#include "data_structs.h"
#include "files.h"


struct fuse_operations fuse_oper_struct = {
	.init      = multiplex_init,
	.getattr   = multiplex_getattr,
	.readdir   = multiplex_readdir,
	.open      = multiplex_open,
	.read      = multiplex_read,
	.write     = multiplex_write,
	.release   = multiplex_release,
};


void *multiplex_init(struct fuse_conn_info *conn) {

	/* Initialization of open table lookup */
	init_open_fh_table();
	
	/* Initialize httpd daemon */
	start_httpd_daemon(g_application_options.server_port);
	
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
	return (int)read((int)fi->fh, buf, size);
}

int multiplex_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi) {
	
	EXLog(FUSE, DBG, "multiplex_write [%s | %d]", path, (int)size);
	
	/* Assume fh is valid in fi */
	if (lseek((int)fi->fh, offset, SEEK_SET) < 0) {
		close((int)fi->fh);
		set_open_fh_for_path(path, -1, NULL);
		return -1;
	}
	
	/* Otherwise read */
	return (int)write((int)fi->fh, buf, size);
}

int multiplex_release(const char *path, struct fuse_file_info *fi) {
	
	EXLog(FUSE, DBG, "multiplex_release [%s]", path);
	
	/* Close file handle and remove from dictionary */
	set_open_fh_for_path(path, -1, NULL);
	return close((int)fi->fh);
}



