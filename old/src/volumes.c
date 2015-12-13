//
//  volumes.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/6/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <libgen.h>
#include <limits.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <sys/xattr.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <dirent.h>
#include <unistd.h>
#include "volumes.h"
#include "exlog.h"
#include "replication.h"

// Jannson 2.3.1 fix
#ifndef json_boolean
#define json_boolean(val)      ((val) ? json_true() : json_false())
#endif

/* ----------------------- File Mode Helpers ----------------------- */

#define NUM_FILE_MODES 7
static int         s_file_mode_types[NUM_FILE_MODES] = { S_IFIFO, S_IFCHR, S_IFDIR, S_IFBLK, S_IFREG, S_IFLNK, S_IFSOCK };
static const char *s_file_mode_ext[NUM_FILE_MODES]   = { "fifo",  "chr",   "dir",   "blk",   "file",  "lnk",   "sock"   };
static char       *s_file_conflict_appendix          = ".mediaRAID-conflict";

/* ----------------------- Default directories --------------------- */

static char s_default_raiddir[PATH_MAX]  = "/mediaRAID";
static char s_default_trashdir[PATH_MAX] = "/.mediaRAID-trash";
static char s_default_workdir[PATH_MAX]  = "/.mediaRAID-work";

void set_default_raiddir(char *raiddir) {
	strncpy(s_default_raiddir, raiddir, PATH_MAX-1);	
}

void set_default_trashdir(char *trashdir) {
	strncpy(s_default_trashdir, trashdir, PATH_MAX-1);
}

void set_default_workdir(char *workdir) {
	strncpy(s_default_workdir, workdir, PATH_MAX-1);
}


/* ---------------------- Volume Byte Counting ------------------- */

void update_volume_byte_counters(RaidVolume_t *volume) {
	if (!volume) return;
	
	struct statvfs fsinfo;
	if (statvfs(volume->basepath, &fsinfo)) {
		return;
	}
	
	volume->capacity_total = fsinfo.f_frsize * fsinfo.f_blocks;
	volume->capacity_free  = fsinfo.f_frsize * fsinfo.f_bavail;
	volume->capacity_used  = volume->capacity_total - volume->capacity_free;
	volume->percent_free   = volume->capacity_total ? (volume->capacity_free * 100) / volume->capacity_total : 0;
}


void clear_active_volume_counters(RaidVolume_t *volume) {
	if (!volume) return;
	
	volume->capacity_total = 0;
	volume->capacity_free  = 0;
	volume->capacity_used  = 0;
	volume->percent_free   = 0;
}


/* -------------------------------------------------------------- */
/* ---------------------- Volume Management --------------------- */
/* -------------------------------------------------------------- */

typedef struct volume_node {
	struct volume_node *next;
	RaidVolume_t *volume;
} VolumeNode_t;

static VolumeNode_t *active_volumes   = NULL;
static VolumeNode_t *inactive_volumes = NULL;
static pthread_mutex_t volume_list_mutex  = PTHREAD_MUTEX_INITIALIZER;
       pthread_mutex_t g_volume_api_mutex = PTHREAD_MUTEX_INITIALIZER;

static void destroy_volume_node(VolumeNode_t *node) {
	if (node) {
		free(node);
	}
}

static VolumeNode_t *list_add_volume(VolumeNode_t **list, RaidVolume_t *volume) {
	VolumeNode_t *node = (VolumeNode_t*)malloc(sizeof(VolumeNode_t));
	if (!node) return NULL;
	
	pthread_mutex_lock(&volume_list_mutex);
	node->next = *list;
	node->volume = volume;
	*list = node;
	pthread_mutex_unlock(&volume_list_mutex);
	return node;
}

static RaidVolume_t *list_remove_volume(VolumeNode_t **list, const char *basepath) {
	pthread_mutex_lock(&volume_list_mutex);
	VolumeNode_t *node = *list;
	RaidVolume_t *volume = NULL;
	
	/* Empty list */
	if (!node) {
		pthread_mutex_unlock(&volume_list_mutex);
		return volume;
	}
	
	/* First element? */
	if (!strncmp(node->volume->basepath, basepath, PATH_MAX)) {
		volume = node->volume;
		*list = node->next;
		destroy_volume_node(node);
		pthread_mutex_unlock(&volume_list_mutex);
		return volume;
	}
	
	/* Others */
	while (node->next) {
		if (!strncmp(node->next->volume->basepath, basepath, PATH_MAX)) {
			VolumeNode_t *oldnode = node->next;
			volume = node->next->volume;
			node->next = node->next->next;
			destroy_volume_node(oldnode);
			pthread_mutex_unlock(&volume_list_mutex);
			return volume;
		}
		node = node->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return volume;
}

static RaidVolume_t *list_lookup_volume_by_alias(VolumeNode_t *list, const char *alias) {
	pthread_mutex_lock(&volume_list_mutex);
	VolumeNode_t *node = list;
	while (node) {
		if (!strncmp(node->volume->alias, alias, PATH_MAX)) {
			pthread_mutex_unlock(&volume_list_mutex);
			return node->volume;
		}
		node = node->next;
	}
	pthread_mutex_unlock(&volume_list_mutex);
	return NULL;
}

static RaidVolume_t *list_lookup_volume_by_basepath(VolumeNode_t *list, const char *basepath) {
	pthread_mutex_lock(&volume_list_mutex);
	VolumeNode_t *node = list;
	while (node) {
		if (!strncmp(node->volume->basepath, basepath, PATH_MAX)) {
			pthread_mutex_unlock(&volume_list_mutex);
			return node->volume;
		}
		node = node->next;
	}
	pthread_mutex_unlock(&volume_list_mutex);
	return NULL;
}

static int count_volumes_in_list_nonatomic(VolumeNode_t *list) {
	int count = 0;
	while (list) {
		list = list->next;
		count++;
	}
	return count;
}

void volume_get_all(RaidVolume_t **active, int *active_count, RaidVolume_t **inactive, int *inactive_count) {
	pthread_mutex_lock(&volume_list_mutex);
	VolumeNode_t *node = active_volumes;
	int max_active = *active_count;
	*active_count = 0;
	while (node && (*active_count < max_active)) {
		*active = node->volume;
		*active_count = *active_count + 1;
		active++;
		node = node->next;
	}
	node = inactive_volumes;
	int max_inactive = *inactive_count;
	*inactive_count = 0;
	while (node && (*inactive_count < max_inactive)) {
		*inactive = node->volume;
		*inactive_count = *inactive_count + 1;
		inactive++;
		node = node->next;
	}
	pthread_mutex_unlock(&volume_list_mutex);
}

/* --------------------- */

static pthread_mutex_t active_switch_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Moves the volume from inactive/active lists, or places it there if not already on the list */
void volume_set_active(RaidVolume_t *volume, int active) {
	pthread_mutex_lock(&active_switch_mutex);
	volume->active = active;
	if (active) {
		RaidVolume_t *t = list_remove_volume(&inactive_volumes, volume->basepath);
		if (t) volume = t;
		if (!list_lookup_volume_by_basepath(active_volumes, volume->basepath)) {
			list_add_volume(&active_volumes, volume);
		}
	} else {
		/* Remove replication tasks */
		replication_queue_kill_all_tasks(NULL, volume->basepath);
		replication_halt_replication_of_file_emergency();
		
		RaidVolume_t *t = list_remove_volume(&active_volumes, volume->basepath);
		if (t) volume = t;
		clear_active_volume_counters(volume);
		if (!list_lookup_volume_by_basepath(inactive_volumes, volume->basepath)) {
			list_add_volume(&inactive_volumes, volume);
		}
	}
	volume->active = active;
	pthread_mutex_unlock(&active_switch_mutex);
}

/* -------------------- */

int volume_is_raid_ready(RaidVolume_t *volume) {
	if (!volume) return 0;
	
	DIR *dir = opendir(volume->raidpath);
	if (!dir) return 0;
	
	closedir(dir);
	return 1;
}

int volume_is_trash_ready(RaidVolume_t *volume) {
	if (!volume) return 0;
	
	DIR *dir = opendir(volume->trashpath);
	if (!dir) return 0;
	
	closedir(dir);
	return 1;
}

/* -------------------- */

/* Creates a RaidVolume_t struct from the given parameters.
   basepath is required. 
 */
RaidVolume_t *create_volume(const char *alias, const char *basepath, const char *custom_raidpath, const char *custom_trashpath, const char *custom_workpath) {
	RaidVolume_t *volume = (RaidVolume_t*)malloc(sizeof(RaidVolume_t));
	if (!volume) return NULL;
	
	/* Reference counting */
	volume->reference_count = 1;
	pthread_mutex_init(&volume->reference_count_mutex, NULL);
	
	/* Copy basepath */
	strncpy(volume->basepath, basepath, PATH_MAX-1);
	
	/* Take alias if present */
	if (alias) {
		strncpy(volume->alias, alias, PATH_MAX-1);
	} else {
		strncpy(volume->alias, basepath, PATH_MAX-1);
	}
	
	/* Other paths */
	if (!custom_raidpath  || *custom_raidpath == 0  ) custom_raidpath  = s_default_raiddir;
	if (!custom_trashpath || *custom_trashpath == 0 ) custom_trashpath = s_default_trashdir;
	if (!custom_workpath  || *custom_workpath == 0  ) custom_workpath  = s_default_workdir;
	
	snprintf(volume->raidpath,  PATH_MAX-1, "%s%s", basepath, custom_raidpath);
	snprintf(volume->trashpath, PATH_MAX-1, "%s%s", basepath, custom_trashpath);
	snprintf(volume->workpath,  PATH_MAX-1, "%s%s", basepath, custom_workpath);
	
	/* concat helpers */
	strncpy(volume->concatpath, volume->raidpath, PATH_MAX-1);
	volume->concatpath_baselen = strnlen(volume->concatpath, PATH_MAX-1);
	
	/* Update byte counters */
	update_volume_byte_counters(volume);
	
	/* Set volume status */
	VOLUME_UPDATE_REPLICATION_STATUS_STRING(volume, "Idle");
	
	/* Done */
	return volume;
}

/* Destroys the volume pointer and anything in the volume struct that was dynamically allocated */
void destroy_volume_ptr(RaidVolume_t *volume) {
	free (volume);
}

void volume_remove(RaidVolume_t *volume) {
	replication_halt_replication_of_file_emergency();
	volume_api_lock();
	
	RaidVolume_t *v = list_remove_volume(&active_volumes, volume->basepath);
	if (v) {
		destroy_volume_ptr(v);
		volume_api_unlock();
		return;
	}
	
	v = list_remove_volume(&inactive_volumes, volume->basepath);
	if (v) destroy_volume_ptr(v);
	
	volume_api_unlock();
}

/* Volume reference counting */
void volume_retain(RaidVolume_t *volume) {
	if (!volume) return;
	pthread_mutex_lock(&volume->reference_count_mutex);
	volume->reference_count++;
	pthread_mutex_unlock(&volume->reference_count_mutex);
}

void volume_release(RaidVolume_t *volume) {
	if (!volume) return;
	pthread_mutex_lock(&volume->reference_count_mutex);
	volume->reference_count--;
	int ref = volume->reference_count;
	pthread_mutex_unlock(&volume->reference_count_mutex);
	if (ref <= 0) {
		destroy_volume_ptr(volume);
	}
}


/* ----------------------- Volume tools ---------------------------- */

int volume_count(int active) {
	int response = 0;
	pthread_mutex_lock(&volume_list_mutex);
	response = count_volumes_in_list_nonatomic(active ? active_volumes : inactive_volumes);
	pthread_mutex_unlock(&volume_list_mutex);
	return response;
}

RaidVolume_t *volume_with_alias(const char *alias) {
	if (!alias) return NULL;
	RaidVolume_t *v = list_lookup_volume_by_alias(active_volumes, alias);
	if (v) return v;
	return list_lookup_volume_by_alias(inactive_volumes, alias);
}

RaidVolume_t *volume_with_basepath(const char *basepath) {
	if (!basepath) return NULL;
	RaidVolume_t *v = list_lookup_volume_by_basepath(active_volumes, basepath);
	if (v) return v;
	return list_lookup_volume_by_basepath(inactive_volumes, basepath);
}

RaidVolume_t *volume_with_name(const char *basepath, const char *alias) {
	if (basepath) {
		return volume_with_basepath(basepath);
	} else if (alias) {
		return volume_with_alias(alias);
	}	
	return NULL;
}


VolumeState_t volume_state_with_alias(const char *alias) {
	if (list_lookup_volume_by_alias(active_volumes, alias)) {
		return VOLUME_ACTIVE;
	}
	
	if (list_lookup_volume_by_alias(inactive_volumes, alias)) {
		return VOLUME_INACTIVE;
	}
	
	return VOLUME_DNE;
}

VolumeState_t volume_state_with_basepath(const char *basepath) {
	if (list_lookup_volume_by_basepath(active_volumes, basepath)) {
		return VOLUME_ACTIVE;
	}
	
	if (list_lookup_volume_by_basepath(inactive_volumes, basepath)) {
		return VOLUME_INACTIVE;
	}
	
	return VOLUME_DNE;
}


const char *volume_full_path_for_raid_path(RaidVolume_t *volume, const char *volume_path, char *buffer) {
	snprintf(buffer, PATH_MAX-1, "%s%s", volume->raidpath, volume_path );
	return buffer;
}

const char *volume_full_path_for_trash_path(RaidVolume_t *volume, const char *volume_path, char *buffer) {
	snprintf(buffer, PATH_MAX-1, "%s%s", volume->trashpath, volume_path );
	return buffer;
}

const char *volume_full_path_for_work_path(RaidVolume_t *volume, const char *volume_path, char *buffer) {
	snprintf(buffer, PATH_MAX-1, "%s%s", volume->workpath, volume_path );
	return buffer;
}

int volume_avaialble_work_path(RaidVolume_t *volume, char *buffer) {
	int attempt = 0;
	do {
		/* Try random filepath */
		snprintf(buffer, PATH_MAX-1, "%s/%d", volume->workpath, (int)(rand() % 10000000));
		struct stat stbuf;
		if (stat(buffer, &stbuf)) {
			/* If stat returns -1, no file present and we can return this buffer */
			return 1;
		}
		attempt++;
	} while (attempt < 100);
	return 0;
}


DIR **volume_active_dir_entries(const char *relative_raid_path) {
	pthread_mutex_lock(&volume_list_mutex);
	int active_volume_count= count_volumes_in_list_nonatomic(active_volumes);
	if (!active_volume_count) {
		pthread_mutex_unlock(&volume_list_mutex);
		return NULL;
	}
	
	DIR **entries = (DIR**)malloc((active_volume_count+2) * sizeof(DIR*));
	if (!entries) {
		pthread_mutex_unlock(&volume_list_mutex);
		return entries;
	}
	
	VolumeNode_t *volume = active_volumes;
	int cur_entry = 0;
	while (volume) {
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		DIR *entry = opendir(fullpath);
		if (entry) {
			entries[cur_entry] = entry;
			//EXLog(VOLUME, DBG, "Got entry for [%s]", fullpath);
			cur_entry++;
		}
		
		volume = volume->next;
	}
	entries[cur_entry] = 0;
	
	pthread_mutex_unlock(&volume_list_mutex);
	return entries;
}


int volume_most_recently_modified_instance(const char *relative_raid_path, RaidVolume_t **which_volume, char *fullpath, struct stat *stbuf) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	struct stat tmp_stbuf;
	
	VolumeNode_t *most_recent_volume = NULL;
	time_t        most_recent_time   = -1;
	int           err_resp           = -1;
	
	while (volume) {
		/* Go through all volumes and find the volume w/ the latest modification date */
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);

		int s = stat(fullpath, &tmp_stbuf);
		if (s == 0) {
			if (STAT_MTIME(tmp_stbuf) > most_recent_time) {
				most_recent_time   = STAT_MTIME(tmp_stbuf);
				most_recent_volume = volume;
				if (stbuf) {
					EXLog(VOLUME, DBG, " > got stat on [%s]", fullpath);
					memcpy(stbuf, &tmp_stbuf, sizeof(struct stat));
				}
			}
		} else {
			err_resp = -errno;
		}
		
		volume = volume->next;
	}
	
	/* No files? return failure */
	if (most_recent_time == -1) {
		pthread_mutex_unlock(&volume_list_mutex);
		EXLog(VOLUME, DBG, " > stat failed with [%d]", errno);
		return err_resp;
	}
	
	/* Otherwise, set data */
	if (which_volume) *which_volume = most_recent_volume->volume;
	if (fullpath)     volume_full_path_for_raid_path(most_recent_volume->volume, relative_raid_path, fullpath);
		
	pthread_mutex_unlock(&volume_list_mutex);
	return 0;
}

int volume_kill_aged_instances_of_path(const char *relative_raid_path) {
	pthread_mutex_lock(&volume_list_mutex);
	
	int killed_instances = 0;
	int instances        = 0;
	
	VolumeNode_t *volume = active_volumes;
	struct stat tmp_stbuf;

	VolumeNode_t *most_recent_volume = NULL;
	time_t        most_recent_time   = -1;
	
	while (volume) {
		/* Go through all volumes and find the volume w/ the latest modification date */
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		int s = stat(fullpath, &tmp_stbuf);
		if (s == 0) {
			instances++;
			if (STAT_MTIME(tmp_stbuf) > most_recent_time) {
				most_recent_time   = STAT_MTIME(tmp_stbuf);
				most_recent_volume = volume;
			}
		}
		volume = volume->next;
	}
	
	/* 0 or 1 files? return */
	if (most_recent_time == -1 || (instances < 2)) {
		pthread_mutex_unlock(&volume_list_mutex);
		return 0;
	}

	/* Now unlink any files out of date */
	volume = active_volumes;
	
	while (volume) {
		/* Go through all volumes and find the volume out of date */
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		int s = stat(fullpath, &tmp_stbuf);
		if (s == 0) {
			if (STAT_MTIME(tmp_stbuf) < most_recent_time) {
				unlink(fullpath);
				EXLog(VOLUME, DBG, " > Unlinked as out of date [%s]", fullpath);
				killed_instances++;
			}
		}
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return killed_instances;
}

int volume_statvfs(const char *relative_raid_path, struct statvfs *statbuf) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -1;
	while (volume) {
		
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		int ret = statvfs(fullpath, statbuf);
		if (!ret) {
			pthread_mutex_unlock(&volume_list_mutex);
			return ret;
		} else {
			master_ret = -errno;
		}
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return master_ret;
}

int volume_rename_path_on_active_volumes(const char *oldpath, const char *newpath) {
	
	/* TODO: This is not an ideal solution; but it will work for now */
	/*       It would be best to rename one at a time, and if there is a success, go back and unlink only the newpath on volumes w/ rename failure */
	/*       Right now we'll just unlink everything in newpath first */

	volume_unlink_path_from_active_volumes(newpath);
	
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -1;
	while (volume) {
		
		char oldfullpath[PATH_MAX];
		char newfullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, oldpath, oldfullpath);
		volume_full_path_for_raid_path(volume->volume, newpath, newfullpath);
		
		int ret = rename(oldfullpath, newfullpath);
		if (!ret) master_ret = 0;
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return (master_ret == 0) ? 0 : -errno;	
}

int volume_unlink_path_from_active_volumes(const char *relative_raid_path) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -1;
	while (volume) {

		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
	
		int ret = unlink(fullpath);
		if (!ret) master_ret = 0;
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return (master_ret == 0) ? 0 : -errno;
}

int volume_access_path_from_active_volumes(const char *relative_raid_path, int amode) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -1;
	while (volume) {
		
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		int ret = access(fullpath, amode);
		if (!ret) {
			pthread_mutex_unlock(&volume_list_mutex);
			return 0;
		} else {
			master_ret = -errno;
		}
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return master_ret;
}

int volume_rmdir_path_from_active_volumes(const char *relative_raid_path) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -1;
	while (volume) {

		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		int ret = rmdir(fullpath);
		if (!ret) master_ret = 0;
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return (master_ret == 0) ? 0 : -errno;
}

int volume_mkdir_path_on_active_volumes(const char *relative_raid_path, mode_t mode) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -1;
	while (volume) {
		
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		int ret = mkdir(fullpath, mode);
		if (!ret) master_ret = 0;
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return (master_ret == 0) ? 0 : -errno;
}

int volume_chown_path_on_active_volumes(const char *relative_raid_path, uid_t uid, gid_t gid) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -1;
	while (volume) {
		
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		int ret = chown(fullpath, uid, gid);
		if (!ret) master_ret = 0;
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return (master_ret == 0) ? 0 : -errno;
}

int volume_chmod_path_on_active_volumes(const char *relative_raid_path, mode_t mode) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -1;
	while (volume) {
		
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		int ret = chmod(fullpath, mode);
		if (!ret) master_ret = 0;
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return (master_ret == 0) ? 0 : -errno;
}

int volume_utimens_path_on_active_volumes(const char *relative_raid_path, const struct timespec tv[2]) {
	
	struct timeval utv[2];
	utv[0].tv_sec = tv[0].tv_sec;
	utv[0].tv_usec = (suseconds_t)(tv[0].tv_nsec / 1000);
	utv[1].tv_sec = tv[1].tv_sec;
	utv[1].tv_usec = (suseconds_t)(tv[1].tv_nsec / 1000);
	
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -1;
	while (volume) {
		
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		int ret = utimes(fullpath, utv);
		if (!ret) master_ret = 0;
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return (master_ret == 0) ? 0 : -errno;
}

int volume_setxattr_path_on_active_volumes(const char *relative_raid_path, const char *name, const char *value, size_t size, int options __APPLE_XATTR_POSITION__ ) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -EACCES;
	struct stat stbuf;
	
	while (volume) {
		
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		/* If no file exists, do not attempt */
		if (stat(fullpath, &stbuf)) {
			volume = volume->next;
			continue;
		}

		/* Dunno what the fuck this flag does, but it causes problems */
		#ifdef __APPLE__
		if (options & XATTR_NOSECURITY) options -= XATTR_NOSECURITY;
		#endif
		
		int ret = setxattr(fullpath, name, value, size __APPLE_XATTR_POSITION_P__ , options);
		
		EXLog(FUSE, DBG, " > setxattr [%d] [%s]", ret, fullpath);
		
		if (!ret) {
			master_ret = 0;
		} else if (master_ret) {
			master_ret = -errno;
		}
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	EXLog(FUSE, DBG, "    > returned %d", master_ret);
	return master_ret;
}

int volume_getxattr_path_on_active_volumes(const char *relative_raid_path, const char *name, char *value, size_t size, int options __APPLE_XATTR_POSITION__ ) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -EACCES;
	struct stat stbuf;
	
	while (volume) {
		
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		/* If no file exists, do not attempt */
		if (stat(fullpath, &stbuf)) {
			volume = volume->next;
			continue;
		}
		
		int ret = (int)getxattr(fullpath, name, value, (ssize_t)size __APPLE_XATTR_POSITION_P2__ );
		
		EXLog(FUSE, DBG, " > getxattr [%d] [%s]", ret, fullpath);
		
		if (ret >= 0) {
			//EXLog(FUSE, DBG, "   > %s", value);
			pthread_mutex_unlock(&volume_list_mutex);
			EXLog(FUSE, DBG, "    > returned %d", ret);
			return ret;
		} else {
			master_ret = -errno;
		}
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	EXLog(FUSE, DBG, "    > returned %d", master_ret);
	return master_ret;
}

int volume_listxattr_path_on_active_volumes(const char *relative_raid_path, char *name, size_t size) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -EACCES;
	struct stat stbuf;
	
	while (volume) {
		
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		/* If no file exists, do not attempt */
		if (stat(fullpath, &stbuf)) {
			volume = volume->next;
			continue;
		}
		
		int ret = (int)listxattr(fullpath, name, size __APPLE_XATTR_POSITION_P3__ );
		if (ret > 0) {
			pthread_mutex_unlock(&volume_list_mutex);
			return ret;
		} else if (ret == 0) {
			master_ret = 0;
		} else if (master_ret) {
			master_ret = -errno;
		}
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return master_ret;
}

int volume_removexattr_path_on_active_volumes(const char *relative_raid_path, const char *name) {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	int master_ret = -EACCES;
	struct stat stbuf;
	
	while (volume) {
		
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);
		
		/* If no file exists, do not attempt */
		if (stat(fullpath, &stbuf)) {
			volume = volume->next;
			continue;
		}
		
		int ret = removexattr(fullpath, name __APPLE_XATTR_POSITION_P3__);
		if (!ret) {
			master_ret = 0;
		} else if (master_ret) {
			master_ret = -errno;
		}
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return master_ret;
}


void volume_update_all_byte_counters() {
	pthread_mutex_lock(&volume_list_mutex);
	VolumeNode_t *volume = active_volumes;
	while (volume) {
		update_volume_byte_counters(volume->volume);
		volume = volume->next;
	}
	pthread_mutex_unlock(&volume_list_mutex);
}


RaidVolume_t *volume_with_most_bytes_free() {
	pthread_mutex_lock(&volume_list_mutex);
	
	VolumeNode_t *volume = active_volumes;
	RaidVolume_t *response = NULL;
	
	while (volume) {
		
		update_volume_byte_counters(volume->volume);
		if (!response) {
			response = volume->volume;
		} else {
			if (response->capacity_free < volume->volume->capacity_free) {
				response = volume->volume;
			}
		}
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
	return response;
}

/* Adds to counters only for unique filesystems */
void volume_get_raid_counters(int64_t *total_bytes, int64_t *used_bytes) {
	pthread_mutex_lock(&volume_list_mutex);
	VolumeNode_t *volume = active_volumes;
	
	*total_bytes = 0;
	*used_bytes  = 0;
	
	const int max_fsids = 256;
	int num_fsids = 0;
	
	fsid_t fsids[max_fsids];
	struct statfs sfs;
	
	while (volume) {
		
		statfs(volume->volume->basepath, &sfs);
		
		int ignore_volume = 0;
		for (int i = 0; i < num_fsids; i++) {
			if (!memcmp(&sfs.f_fsid, &fsids[i], sizeof(fsid_t))) {
				ignore_volume = 1;
				break;
			}
		}
		
		if (ignore_volume) {
			volume = volume->next;
			continue;
		}
		
		update_volume_byte_counters(volume->volume);
		*total_bytes = *total_bytes + volume->volume->capacity_total;
		*used_bytes  = *used_bytes  + volume->volume->capacity_used;
		
		memcpy(&fsids[num_fsids], &sfs.f_fsid, sizeof(fsid_t));
		num_fsids++;
		
		if (num_fsids == max_fsids) break;
		
		volume = volume->next;
	}
	
	pthread_mutex_unlock(&volume_list_mutex);
}

/* All arguments aside from path should be pre-allocated buffers for return values, or NULL if you don't care */
void volume_diagnose_raid_file_posession(const char *path,
										 int *instances, int *absences,
										 mode_t *mode_or, int *mode_conflict,
										 RaidVolume_t **posessing_volume_with_least_affinity, char *fullpath_to_possessing,
										 RaidVolume_t **unposessing_volume_with_most_affinity, char *fullpath_to_unpossessing) {
	
	/* Sanity check */
	if (!path) return;
	
	/* Counters */
	int    instance_count = 0;
	int    absence_count  = 0;
	mode_t mode_result    = 0;
	int    mode_conf_res  = 0;
	
	RaidVolume_t *poss_volume_with_less_aff = NULL;
	RaidVolume_t *unpo_volume_with_most_aff = NULL;
	int64_t       poss_volume_with_less_aff_free = INT_MAX;
	int64_t       unpo_volume_with_most_aff_free = 0;
	int64_t       poss_modified_time             = LLONG_MAX;
	
	pthread_mutex_lock(&volume_list_mutex);

	VolumeNode_t *volume = active_volumes;
	while (volume) {
		
		/* Check presence of file in each volume */
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, path, fullpath);
		struct stat stbuf;
		
		if (!stat(fullpath, &stbuf)) {
			/* If stat succeeds, file is "present" */
			instance_count++;
			
			/* Or-in the mode */
			if (mode_result && ((mode_result & S_IFMT) != (stbuf.st_mode & S_IFMT))) {
				mode_conf_res = 1;
			}
			mode_result |= stbuf.st_mode;
			
			/* Volume calcs: we want the possessing volume w/ least percent free */
			if ((poss_volume_with_less_aff_free > volume->volume->percent_free) && (STAT_MTIME(stbuf) < poss_modified_time)) {
				poss_volume_with_less_aff_free = volume->volume->percent_free;
				poss_volume_with_less_aff      = volume->volume;
				poss_modified_time             = STAT_MTIME(stbuf);
			}
		} else {
			absence_count++;
			
			/* If stat failed, calc for the volume with most bytes free */
			if (unpo_volume_with_most_aff_free < volume->volume->capacity_free) {
				unpo_volume_with_most_aff_free = volume->volume->capacity_free;
				unpo_volume_with_most_aff      = volume->volume;
			}
		}
				
		volume = volume->next;
	}
	
	/* Fill out pertinent variables */
	if (instances)     *instances     = instance_count;
	if (absences)      *absences      = absence_count;
	if (mode_or)       *mode_or       = mode_result;
	if (mode_conflict) *mode_conflict = mode_conf_res;
	if (posessing_volume_with_least_affinity)  *posessing_volume_with_least_affinity  = poss_volume_with_less_aff;
	if (unposessing_volume_with_most_affinity) *unposessing_volume_with_most_affinity = unpo_volume_with_most_aff;
	if (fullpath_to_possessing   && poss_volume_with_less_aff) volume_full_path_for_raid_path(poss_volume_with_less_aff, path, fullpath_to_possessing);
	if (fullpath_to_unpossessing && unpo_volume_with_most_aff) volume_full_path_for_raid_path(unpo_volume_with_most_aff, path, fullpath_to_unpossessing);
	
	pthread_mutex_unlock(&volume_list_mutex);
	return;
}


/* Assumes a check has already been made to detect mode conflicts.  This renames each element to resolve conflicts */
void volume_resolve_conflicting_modes(const char *path) {

	/* Sanity */
	if (!path) return;
	
	/* Allocate conflict resolution paths */
	char *conflict_resolution_paths[NUM_FILE_MODES];
	for (int i = 0; i < NUM_FILE_MODES; i++) {
		conflict_resolution_paths[i] = (char*)malloc(PATH_MAX);
	}
	
	/* Determine resolution paths */
	for (int mode = 0; mode < NUM_FILE_MODES; mode++) {
		
		/* Iterate "attempt" until we find a unique path on the filesystem */
		int attempt = 0;
		while (1) {
			snprintf(conflict_resolution_paths[mode], PATH_MAX-1, "%s%s.%d.%s", path, s_file_conflict_appendix, attempt, s_file_mode_ext[mode]);
			
			/* Non-zero if the file doesn't exist on the raid */
			if (volume_most_recently_modified_instance(conflict_resolution_paths[mode], NULL, NULL, NULL)) {
				break;
			}
			
			attempt++;
		}
	}
	
	/* Now we just iterate and rename */
	pthread_mutex_lock(&volume_list_mutex);
	VolumeNode_t *volume = active_volumes;
	while (volume) {
	
		char oldpath[PATH_MAX];
		char newpath[PATH_MAX];
		struct stat stbuf;
		
		volume_full_path_for_raid_path(volume->volume, path, oldpath);
		int stat_resp = stat(oldpath, &stbuf);
		if (!stat_resp) {
			/* File found */
			int which_mode = 4;
			for (int mode = 0; mode < NUM_FILE_MODES; mode++) {
				if ((stbuf.st_mode & S_IFMT) == s_file_mode_types[mode]) {
					which_mode = mode;
					break;
				}
			}
			
			/* rename based on mode; do not rename directories */
			if (which_mode != 2) {
				volume_full_path_for_raid_path(volume->volume, conflict_resolution_paths[which_mode], newpath);
				rename(oldpath, newpath);
			}
		}
		
		volume = volume->next;
	}
	pthread_mutex_unlock(&volume_list_mutex);
	
	/* Clean paths */
	for (int i = 0; i < NUM_FILE_MODES; i++) {
		free(conflict_resolution_paths[i]);
	}
	
	return;
}


/* ----------------------- JSON ------------------------------------ */

json_t *volume_json_object(RaidVolume_t *volume) {
	if (!volume) return NULL;
	
	json_t *obj = json_object();
	
	json_object_set_new(obj, "active",   json_boolean( volume->active ));
	
	json_object_set_new(obj, "alias",    json_string(volume->alias));
	json_object_set_new(obj, "basepath", json_string(volume->basepath));
	json_object_set_new(obj, "raiddir",  json_string(basename(volume->raidpath)));
	json_object_set_new(obj, "trashdir", json_string(basename(volume->trashpath)));
	
	if (!volume->active) return obj;
	
	json_object_set_new(obj, "capacity_total",  json_integer(volume->capacity_total));
	json_object_set_new(obj, "capacity_free",   json_integer(volume->capacity_free));
	json_object_set_new(obj, "capacity_used",   json_integer(volume->capacity_used));
	json_object_set_new(obj, "percent_free",    json_integer(volume->percent_free));
	
	json_object_set_new(obj, "raid_available",  json_boolean( volume_is_raid_ready(volume) ));
	json_object_set_new(obj, "trash_available", json_boolean( volume_is_trash_ready(volume) ));
	
	return obj;
}


