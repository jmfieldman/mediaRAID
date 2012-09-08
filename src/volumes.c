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
#include <sys/statvfs.h>
#include "volumes.h"

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
	free(node);
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


/* --------------------- */

static pthread_mutex_t active_switch_mutex = PTHREAD_MUTEX_INITIALIZER;

/* Moves the volume from inactive/active lists, or places it there if not already on the list */
void set_volume_active(RaidVolume_t *volume, int active) {
	pthread_mutex_lock(&active_switch_mutex);
	if (active) {
		RaidVolume_t *t = list_remove_volume(&inactive_volumes, volume->basepath);
		if (t) volume = t;
		if (!list_lookup_volume_by_basepath(active_volumes, volume->basepath)) {
			list_add_volume(&active_volumes, volume);
		}
	} else {
		RaidVolume_t *t = list_remove_volume(&active_volumes, volume->basepath);
		if (t) volume = t;
		volume->capacity_total = 0;
		volume->capacity_free  = 0;
		volume->capacity_used  = 0;
		if (!list_lookup_volume_by_basepath(inactive_volumes, volume->basepath)) {
			list_add_volume(&inactive_volumes, volume);
		}
	}
	pthread_mutex_unlock(&active_switch_mutex);
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
	pthread_mutex_init(&volume->concatpath_mutex, NULL);
	
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


/* ----------------------- JSON ------------------------------------ */

json_t *volume_json_object(RaidVolume_t *volume) {
	if (!volume) return NULL;
	
	json_t *obj = json_object();
	
	json_object_set_new(obj, "alias",    json_string(volume->alias));
	json_object_set_new(obj, "basepath", json_string(volume->basepath));
	json_object_set_new(obj, "raiddir",  json_string(basename(volume->raidpath)));
	json_object_set_new(obj, "trashdir", json_string(basename(volume->trashpath)));
	
	json_object_set_new(obj, "capacity_total", json_integer(volume->capacity_total));
	json_object_set_new(obj, "capacity_free",  json_integer(volume->capacity_free));
	json_object_set_new(obj, "capacity_used",  json_integer(volume->capacity_used));
	
	return obj;
}


