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
#include "volumes.h"

/* ----------------------- Default directories --------------------- */

static char s_default_raiddir[PATH_MAX];
static char s_default_trashdir[PATH_MAX];

void set_default_raiddir(char *raiddir) {
	strncpy(s_default_raiddir, raiddir, PATH_MAX-1);
}

void set_default_trashdir(char *trashdir) {
	strncpy(s_default_trashdir, trashdir, PATH_MAX-1);
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

static void destroy_volume_node(VolumeNode_t *node) {
	free(node);
}

static VolumeNode_t *list_add_volume(VolumeNode_t **list, RaidVolume_t *volume) {
	VolumeNode_t *node = (VolumeNode_t*)malloc(sizeof(VolumeNode_t));
	if (!node) return NULL;
	
	node->next = *list;
	node->volume = volume;
	*list = node;
	return node;
}

static RaidVolume_t *list_remove_volume(VolumeNode_t **list, const char *basepath) {
	VolumeNode_t *node = *list;
	RaidVolume_t *volume = NULL;
	
	/* Empty list */
	if (!node) return volume;
	
	/* First element? */
	if (!strncmp(node->volume->basepath, basepath, PATH_MAX)) {
		volume = node->volume;
		*list = node->next;
		destroy_volume_node(node);
		return volume;
	}
	
	/* Others */
	while (node->next) {
		if (!strncmp(node->next->volume->basepath, basepath, PATH_MAX)) {
			volume = node->next->volume;
			node->next = node->next->next;
			destroy_volume_node(node->next);
			return volume;
		}
		node = node->next;
	}
	
	return volume;
}

static RaidVolume_t *list_lookup_volume_by_alias(VolumeNode_t *list, const char *alias) {
	VolumeNode_t *node = list;
	while (node) {
		if (!strncmp(node->volume->alias, alias, PATH_MAX)) {
			return node->volume;
		}
		node = node->next;
	}
	return NULL;
}

static RaidVolume_t *list_lookup_volume_by_basepath(VolumeNode_t *list, const char *basepath) {
	VolumeNode_t *node = list;
	while (node) {
		if (!strncmp(node->volume->basepath, basepath, PATH_MAX)) {
			return node->volume;
		}
		node = node->next;
	}
	return NULL;
}


/* --------------------- */

void set_volume_active(RaidVolume_t *volume, int active) {
	if (active) {
		RaidVolume_t *t = list_remove_volume(&inactive_volumes, volume->basepath);
		if (t) volume = t;
		list_add_volume(&active_volumes, volume);
	} else {
		RaidVolume_t *t = list_remove_volume(&active_volumes, volume->basepath);
		if (t) volume = t;
		list_add_volume(&inactive_volumes, volume);
	}
}


