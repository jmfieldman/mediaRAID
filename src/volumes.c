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
#include <errno.h>
#include <sys/types.h>
#include <sys/statvfs.h>
#include <dirent.h>
#include <unistd.h>
#include "volumes.h"
#include "exlog.h"

// Jannson 2.3.1 fix
#ifndef json_boolean
#define json_boolean(val)      ((val) ? json_true() : json_false())
#endif

/* ----------------------- Default directories --------------------- */

static char s_default_raiddir[PATH_MAX]  = "/mediaRAID";
static char s_default_trashdir[PATH_MAX] = "/mediaRAID-trash";

void set_default_raiddir(char *raiddir) {
	strncpy(s_default_raiddir, raiddir, PATH_MAX-1);
}

void set_default_trashdir(char *trashdir) {
	strncpy(s_default_trashdir, trashdir, PATH_MAX-1);
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
			volume = node->next->volume;
			node->next = node->next->next;
			destroy_volume_node(node->next);
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

/* --------------------- */

static pthread_mutex_t active_switch_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Moves the volume from inactive/active lists, or places it there if not already on the list */
void set_volume_active(RaidVolume_t *volume, int active) {
	pthread_mutex_lock(&active_switch_mutex);
	volume->active = active;
	if (active) {
		RaidVolume_t *t = list_remove_volume(&inactive_volumes, volume->basepath);
		if (t) volume = t;
		if (!list_lookup_volume_by_basepath(active_volumes, volume->basepath)) {
			list_add_volume(&active_volumes, volume);
		}
	} else {
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
RaidVolume_t *create_volume(const char *alias, const char *basepath, const char *custom_raidpath, const char *custom_trashpath) {
	RaidVolume_t *volume = (RaidVolume_t*)malloc(sizeof(RaidVolume_t));
	if (!volume) return NULL;
	
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
	
	snprintf(volume->raidpath,  PATH_MAX-1, "%s%s", basepath, custom_raidpath);
	snprintf(volume->trashpath, PATH_MAX-1, "%s%s", basepath, custom_trashpath);
	
	/* concat helpers */
	strncpy(volume->concatpath, volume->raidpath, PATH_MAX-1);
	volume->concatpath_baselen = strnlen(volume->concatpath, PATH_MAX-1);
	
	/* Update byte counters */
	update_volume_byte_counters(volume);
	
	/* Done */
	return volume;
}

/* Destroys the volume pointer and anything in the volume struct that was dynamically allocated */
void destroy_volume_ptr(RaidVolume_t *volume) {
	free (volume);
}


/* ----------------------- Volume tools ---------------------------- */

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
	int           stat_resp;
	
	while (volume) {
		/* Go through all volumes and find the volume w/ the latest modification date */
		char fullpath[PATH_MAX];
		volume_full_path_for_raid_path(volume->volume, relative_raid_path, fullpath);

		int s = stat(fullpath, &tmp_stbuf);
		if (s == 0) {
			if (tmp_stbuf.st_mtimespec.tv_sec > most_recent_time) {
				most_recent_time   = tmp_stbuf.st_mtimespec.tv_sec;
				most_recent_volume = volume;
				if (stbuf) {
					memcpy(stbuf, &tmp_stbuf, sizeof(struct stat));
				}
			}
		} else {
			stat_resp = -errno;
		}
		
		volume = volume->next;
	}
	
	/* No files? return failure */
	if (most_recent_time == -1) {
		pthread_mutex_unlock(&volume_list_mutex);
		return stat_resp;
	}
	
	/* Otherwise, set data */
	if (which_volume) *which_volume = most_recent_volume->volume;
	if (fullpath)     volume_full_path_for_raid_path(most_recent_volume->volume, relative_raid_path, fullpath);
		
	pthread_mutex_unlock(&volume_list_mutex);
	return 0;
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
	return master_ret;
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
	return master_ret;
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


