//
//  fuse_multiplex.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/6/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <string.h>
#include <syslog.h>

#include "fuse_multiplex.h"
#include "mediaRAID.h"
#include "simplehash.h"


struct fuse_operations fuse_oper_struct = {
	.init      = multiplex_init,
	.getattr   = multiplex_getattr,
	.readdir   = multiplex_readdir,
	.open      = multiplex_open,
	.read      = multiplex_read,
	
};


void *multiplex_init(struct fuse_conn_info *conn) {

	/* Initialization of open table lookup */
	init_open_fh_table();
	
	EXLog(FUSE, DBG, "Hello %d", 10);

	
	return 0;
}

int multiplex_getattr(const char *path, struct stat *stbuf) {
	return 0;
}

int multiplex_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
					  off_t offset, struct fuse_file_info *fi) {
	return 0;
}

int multiplex_open(const char *path, struct fuse_file_info *fi) {
	
	return 0;
}

int multiplex_read(const char *path, char *buf, size_t size, off_t offset,
				   struct fuse_file_info *fi) {
	return 0;
}

