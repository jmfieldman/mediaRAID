//
//  exlog.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/6/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <sys/time.h>
#include <sys/types.h>
#include "exlog.h"


#ifdef EXDEBUGENABLED

static double start_time = 0;

void Timing_MarkStartTime() {
	struct timeval tp;
	gettimeofday( &tp, NULL );
	start_time = tp.tv_sec + ( (double)(tp.tv_usec) / 1E6 );
}

double Timing_GetElapsedTime() {
	struct timeval tp;
	gettimeofday( &tp, NULL );
	double cur_time = tp.tv_sec + ( (double)(tp.tv_usec) / 1E6 );
	return (cur_time - start_time);
}

#endif
