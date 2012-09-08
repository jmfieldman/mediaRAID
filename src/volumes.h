//
//  volumes.h
//  mediaRAID
//
//  Created by Jason Fieldman on 9/6/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#ifndef mediaRAID_volumes_h
#define mediaRAID_volumes_h

#include <stdint.h>
#include <limits.h>
#include <pthread.h>
#include <jansson.h>

/* -------------------- Defines --------------------------- */

typedef enum {
	VOLUME_DNE      = 0,
	VOLUME_ACTIVE   = 1,
	VOLUME_INACTIVE = 2,
} VolumeState_t;

/* -------------------- RaidVolume_t ---------------------- */

typedef struct {
	
	/* Alias */
	
	/* Alias used to identiy a volume.  Defaults to basepath */
	char alias[PATH_MAX];
	
	/* Paths */
	
	/* The root path to the base mount point of this volume */
	char basepath[PATH_MAX];
	
	/* The root path to the RAID files (typically base + directory) */
	char raidpath[PATH_MAX];
	
	/* The root path to the trash files (typically base + directory) */
	char trashpath[PATH_MAX];
	
	/* raidpath concatination accelerators */
	
	char            concatpath[PATH_MAX];
	size_t          concatpath_baselen;
	pthread_mutex_t concatpath_mutex;
	
	/* Capacity */
	
	/* Volume capacity in bytes */
	int64_t capacity_total;
	
	/* Free space in bytes */
	int64_t capacity_free;
	
	/* Used space in bytes */
	int64_t capacity_used;
	
	
} RaidVolume_t;

/* ----------------------- API Lock -------------------------------- */

extern pthread_mutex_t g_volume_api_mutex;
static inline void volume_api_lock()   { pthread_mutex_lock(&g_volume_api_mutex);   }
static inline void volume_api_unlock() { pthread_mutex_unlock(&g_volume_api_mutex); }

/* ----------------------- Default directories --------------------- */

void set_default_raiddir(char *raiddir);
void set_default_trashdir(char *trashdir);

/* ----------------------- Volume tools ---------------------------- */

RaidVolume_t *volume_with_alias(const char *alias);
RaidVolume_t *volume_with_basepath(const char *basepath);
RaidVolume_t *volume_with_name(const char *basepath, const char *alias);
VolumeState_t volume_state_with_alias(const char *alias);
VolumeState_t volume_state_with_basepath(const char *basepath);

RaidVolume_t *create_volume(const char *alias, const char *basepath, const char *custom_raidpath, const char *custom_trashpath);
void set_volume_active(RaidVolume_t *volume, int active);

/* ----------------------- JSON ------------------------------------ */

json_t *volume_json_object(RaidVolume_t *volume);

#endif
