//
//  fuse_multiplex.h
//  mediaRAID
//
//  Created by Jason Fieldman on 9/6/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#ifndef mediaRAID_fuse_multiplex_h
#define mediaRAID_fuse_multiplex_h

#include <fuse.h>

extern struct fuse_operations fuse_oper_struct;

int multiplex_getattr(const char *path, struct stat *stbuf);
int multiplex_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
					  off_t offset, struct fuse_file_info *fi);
int multiplex_open(const char *path, struct fuse_file_info *fi);
int multiplex_read(const char *path, char *buf, size_t size, off_t offset,
				   struct fuse_file_info *fi);


#endif