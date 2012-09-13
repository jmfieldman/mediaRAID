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
#include <sys/stat.h>
#include <pthread.h>
#include <jansson.h>
#include <dirent.h>

/* -------------------- Defines --------------------------- */

typedef enum {
	VOLUME_DNE      = 0,
	VOLUME_ACTIVE   = 1,
	VOLUME_INACTIVE = 2,
} VolumeState_t;

/* -------------------- RaidVolume_t ---------------------- */

typedef struct {
	
	/* Active */
	int active;
	
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
	
	/* The root path to the work files (typically base + directory) */
	char workpath[PATH_MAX];
	
	/* raidpath concatination accelerators */
	
	char            concatpath[PATH_MAX];
	size_t          concatpath_baselen;
	
	/* Capacity */
	
	/* Volume capacity in bytes */
	int64_t capacity_total;
	
	/* Free space in bytes */
	int64_t capacity_free;
	int64_t percent_free;
	
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
void set_default_workdir(char *workdir);

/* ----------------------- Volume tools ---------------------------- */

RaidVolume_t *volume_with_alias(const char *alias);
RaidVolume_t *volume_with_basepath(const char *basepath);
RaidVolume_t *volume_with_name(const char *basepath, const char *alias);
VolumeState_t volume_state_with_alias(const char *alias);
VolumeState_t volume_state_with_basepath(const char *basepath);

RaidVolume_t *create_volume(const char *alias, const char *basepath, const char *custom_raidpath, const char *custom_trashpath, const char *custom_workpath);
void set_volume_active(RaidVolume_t *volume, int active);

const char *volume_full_path_for_raid_path(RaidVolume_t *volume, const char *volume_path, char *buffer);
const char *volume_full_path_for_trash_path(RaidVolume_t *volume, const char *volume_path, char *buffer);
const char *volume_full_path_for_work_path(RaidVolume_t *volume, const char *volume_path, char *buffer);

int volume_avaialble_work_path(RaidVolume_t *volume, char *buffer);

DIR **volume_active_dir_entries(const char *relative_raid_path);

/* Pass in pre-allocated mem for which_volume and fullpath. Returns 0 on success */
int volume_most_recently_modified_instance(const char *relative_raid_path, RaidVolume_t **which_volume, char *fullpath, struct stat *stbuf);

int volume_unlink_path_from_active_volumes(const char *relative_raid_path);
int volume_rmdir_path_from_active_volumes(const char *relative_raid_path);
int volume_mkdir_path_on_active_volumes(const char *relative_raid_path, mode_t mode);
int volume_chown_path_on_active_volumes(const char *relative_raid_path, uid_t uid, gid_t gid);
int volume_chmod_path_on_active_volumes(const char *relative_raid_path, mode_t mode);
int volume_utimens_path_on_active_volumes(const char *relative_raid_path, const struct timespec tv[2]);

void volume_update_all_byte_counters();
RaidVolume_t *volume_with_most_bytes_free();

/* All arguments aside from path should be pre-allocated buffers for return values, or NULL if you don't care */
void volume_diagnose_raid_file_posession(const char *path,
										 int *instances, int *absences,
										 mode_t *mode_or, int *mode_conflict,
										 RaidVolume_t **posessing_volume_with_least_affinity, char *fullpath_to_possessing,
										 RaidVolume_t **unposessing_volume_with_most_affinity, char *fullpath_to_unpossessing);

/* Assumes a check has already been made to detect mode conflicts.  This renames each element to resolve conflicts */
void volume_resolve_conflicting_modes(const char *path);



/* ----------------------- JSON ------------------------------------ */

json_t *volume_json_object(RaidVolume_t *volume);

#endif
