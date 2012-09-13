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

void replication_task_init(ReplicationTask_t *task) {
	task->path[0] = 0;
	task->volume_basepath[0] = 0;
}

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
	
	int    absences      = 0;
	mode_t mode          = 0;
	int    mode_conflict = 0;
	
	EXLog(REPL, DBG, "Task: mirror directory [%s]", task->path);
	
	/* Perform diagnostics on the path */
	volume_diagnose_raid_file_posession(task->path, NULL, &absences, &mode, &mode_conflict, NULL, NULL, NULL, NULL);
	
	/* If this isn't a directory, just exit */
	if (!mode_conflict && (mode & S_IFMT) != S_IFDIR) {
		return;
	}
	
	/* Conflict?  Resolve */
	if (mode_conflict) {
		volume_resolve_conflicting_modes(task->path);
		
		/* Perform diagnostics on the path again */
		volume_diagnose_raid_file_posession(task->path, NULL, &absences, &mode, &mode_conflict, NULL, NULL, NULL, NULL);
		
		/* If this isn't a directory, just exit */
		if (!mode_conflict && (mode & S_IFMT) != S_IFDIR) {
			return;
		}
	}
	
	/* Absences? Make directories */
	if (absences) {
		volume_mkdir_path_on_active_volumes(task->path, (mode & 0xFFFF) | S_IFDIR);
	}
	
	/* Now we should queue up child directories */
	/* Get all the DIR entries for the active volumes */
	DIR **entries = volume_active_dir_entries(task->path);
	DIR **entries_start = entries;
	
	ReplicationTask_t child_task;
	replication_task_init(&child_task);
	child_task.opcode = REP_OP_MIRROR_DIRECTORY;
	child_task.volume_basepath[0] = 0;
	
	if (entries) {
		/* create a hash table that will help detect duplicate entries */
		Dictionary_t *dic = dictionary_create_with_size(512);
		int64_t tmp;
		while (*entries) {
			/* For each DIR, iterate through and grab entries */
			DIR *entry = *entries;
			struct dirent *dent = readdir(entry);
			while (dent) {
				
				if (dent->d_type == DT_DIR && strcmp("..", dent->d_name) && strcmp(".", dent->d_name)) {
					/* For each entry, add it to the list if it doesn't exist in the hash table */
					if (!dictionary_get_int(dic, dent->d_name, &tmp)) {
						dictionary_set_int(dic, dent->d_name, tmp);
						snprintf(child_task.path, PATH_MAX-1, "%s/%s", task->path, dent->d_name);
						replication_queue_task(&child_task, OP_PRI_SYNC_DIR, 1);
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

