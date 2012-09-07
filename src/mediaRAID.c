//
//  mediaRAID.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/6/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <stddef.h>

#include "mediaRAID.h"
#include "fuse_multiplex.h"


/* Application options */
ApplicationOptions_t g_application_options;

#define CUSTOM_OPT_KEY(t, p, v) { t, offsetof(ApplicationOptions_t, p), v }

static struct fuse_opt app_opts[] =
{
	CUSTOM_OPT_KEY("-port %d", server_port, 14256),
	CUSTOM_OPT_KEY("-log %s", log_file, 0),
	FUSE_OPT_END
};


/* Logging */
FILE *g_logfile = NULL;


/* Main */

int main(int argc, const char * argv[]) {
	
	/* Timing debug */
	Timing_MarkStartTime();
			
	int ret;

	struct fuse_args args = FUSE_ARGS_INIT(argc, (char**) argv);
	
	/* clear structure that holds our options */
	memset(&g_application_options, 0, sizeof(ApplicationOptions_t));
	
	if (fuse_opt_parse(&args, &g_application_options, app_opts, NULL) == -1) {
		/* error parsing options */
		return -1;
	}
	
	/* If log file defined, initialize it */
	if (g_application_options.log_file) {
		g_logfile = fopen(g_application_options.log_file, "ab");
	}
	
	EXLog(ANY, DBG, "------------------------- mediaRAID starting up -----------------------");
	
	/* Begin main loop */
	ret = fuse_main(args.argc, args.argv, &fuse_oper_struct, NULL);
	
	if (ret) {
		EXLog(ANY, ERR, "error starting fuse [%d]", ret);
	}
	
	/* free arguments */
	fuse_opt_free_args(&args);
	
    return ret;
}

