//
//  mediaRAID.h
//  mediaRAID
//
//  Created by Jason Fieldman on 9/6/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#ifndef mediaRAID_mediaRAID_h
#define mediaRAID_mediaRAID_h

#include <stdio.h>

#include <fuse.h>
#include <fuse_opt.h>
#include <microhttpd.h>

#include "exlog.h"

/* options for fuse_opt.h */
typedef struct {
	int server_port;
	char *log_file;
} ApplicationOptions_t ;

extern ApplicationOptions_t g_application_options;


#endif
