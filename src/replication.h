//
//  replication.h
//  mediaRAID
//
//  Created by Jason Fieldman on 9/12/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#ifndef mediaRAID_replication_h
#define mediaRAID_replication_h

typedef enum {
	
	REP_OP_MIRROR_FILE,
	REP_OP_MIRROR_DIRECTORY,
		
} ReplicationOpcode_t;

typedef struct {
	
	ReplicationOpcode_t   opcode;
	char                 *path;
	
} ReplicationTask_t;


/* -------------- Engine --------------- */

void replication_start();
void replication_pause();


#endif
