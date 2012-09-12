//
//  replication.h
//  mediaRAID
//
//  Created by Jason Fieldman on 9/12/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#ifndef mediaRAID_replication_h
#define mediaRAID_replication_h

#include "volumes.h"

typedef enum {
	
	OP_PRI_CRITICAL         = 0,
	OP_PRI_SYNC_DIR         = 2,
	OP_PRI_SYNC_FILE        = 4,
	OP_PRI_LAZY             = 6,
	
} OperationPriority_t;

typedef enum {
	
	/* Bring file closer to the redundancy number across active volumes */
	REP_OP_BALANCE_FILE,
	
	/* Ensure that the directory is mirrored properly across active directories.
	   The operation automatically queues subdirectories, so you only to manually add the parent */
	REP_OP_MIRROR_DIRECTORY,
	
	/* Ensures that the specified RaidVolume_t has proper volume base directories created.  If not, it is set to inactive.
	   A subprocess can queue this up with priority if it encounters an unusual read/write error on the volume. */
	REP_OP_VERIFY_VOLUME_DIRS,
	
} ReplicationOpcode_t;

typedef struct {
	
	ReplicationOpcode_t   opcode;
	
	/* Path to operate on */
	char                  path[PATH_MAX];
	
	/* Optionally specify a volume to operate on */
	char                  volume_basepath[PATH_MAX];
	
} ReplicationTask_t;


/* -------------- Engine --------------- */

void replication_start();
void replication_pause();

/* Set priority=1 to queue in front, 0 for back.  Memory in *task will be copied, so need not be malloc'd */
void replication_queue_task(ReplicationTask_t *task, OperationPriority_t priority, int front);

#endif
