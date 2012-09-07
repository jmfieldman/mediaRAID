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

/* ----------------------- Default directories --------------------- */

void set_default_raiddir(char *raiddir);
void set_default_trashdir(char *trashdir);


#endif