//
//  replication.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/12/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <pthread.h>
#include <unistd.h>
#include "data_structs.h"
#include "replication.h"
#include "exlog.h"


static TieredPriorityQueue_t *s_replication_queue = NULL;
static volatile int           s_replication_queue_paused = 0;
static pthread_t              s_replication_thread;

void *replication_thread(void *arg);

/* ------------------------- Replication Start/Pause ------------------------------ */

void replication_start() {
	
	/* Unpause replication thread */
	s_replication_queue_paused = 0;
	
	if (!s_replication_queue) {
		s_replication_queue = tiered_priority_queue_create();
		pthread_create(&s_replication_thread, NULL, replication_thread, NULL);
	}
	
}

void replication_pause() {
	/* Signal replication thread to pause */
	s_replication_queue_paused = 1;
}

void replication_queue_task(ReplicationTask_t *task, OperationPriority_t priority, int front) {
	ReplicationTask_t *task_to_queue = (ReplicationTask_t*)malloc(sizeof(ReplicationTask_t));
	memcpy(task_to_queue, task, sizeof(ReplicationTask_t));
	
	tiered_priority_queue_push(s_replication_queue, priority, front, task_to_queue);
}


/* ---------------------------- Replication Operations ------------------------------- */

void replication_task_mirror_directory(ReplicationTask_t *task) {
	
}

void replication_task_mirror_file(ReplicationTask_t *task) {
	
}

void replication_task_verify_raid_dir(ReplicationTask_t *task) {
	
}

/* ---------------------------- Replication Thread ------------------------------- */

void *replication_thread(void *arg) {
	
	EXLog(REPL, INFO, "Replication thread started");
	
	while (1) {
		
		ReplicationTask_t *next_task = (ReplicationTask_t*)tiered_priority_queue_pop(s_replication_queue);
		
		if (next_task) {
		
			switch (next_task->opcode) {
				case REP_OP_MIRROR_DIRECTORY:               replication_task_mirror_directory(next_task);                     break;
				case REP_OP_BALANCE_FILE:                   replication_task_mirror_file(next_task);                          break;
				case REP_OP_VERIFY_VOLUME_DIRS:             replication_task_verify_raid_dir(next_task);                      break;
			}
			
			/* Free the task structure */
			free(next_task);
			
			/* Let other threads run */
			usleep(0);
			
		} else {
			
			/* Sleep a second before checking for a non-empty task stack */
			sleep(1);
		}		
	}
	
	pthread_exit(NULL);
}

