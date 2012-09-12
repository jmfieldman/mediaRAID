//
//  replication.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/12/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include "data_structs.h"
#include "replication.h"
#include "exlog.h"


static LinkedList_t *s_replication_queue = NULL;
static volatile int  s_replication_queue_paused = 0;
static pthread_t     s_replication_thread;

void *replication_thread(void *arg);

void replication_start() {
	
	/* Unpause replication thread */
	s_replication_queue_paused = 0;
	
	if (!s_replication_queue) {
		s_replication_queue = linked_list_create();
		pthread_create(&s_replication_thread, NULL, replication_thread, NULL);
	}
	
}

void replication_pause() {
	/* Signal replication thread to pause */
	s_replication_queue_paused = 1;
}


void *replication_thread(void *arg) {
	
	EXLog(REPL, INFO, "Replication thread started");
	
	while (1) {
		
		ReplicationTask_t *next_task = (ReplicationTask_t*)linked_list_pop_front(s_replication_queue);
		
		if (next_task) {
			
			
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

