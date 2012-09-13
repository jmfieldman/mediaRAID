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

#ifndef __APPLE_XATTR_POSITION__
#ifdef __APPLE__
#define __APPLE_XATTR_POSITION__ , u_int32_t position
#define __APPLE_XATTR_POSITION_P__ , position
#define __APPLE_XATTR_POSITION_P2__ , position , options
#define __APPLE_XATTR_POSITION_P3__ , 0
#else
#define __APPLE_XATTR_POSITION__
#define __APPLE_XATTR_POSITION_P__
#define __APPLE_XATTR_POSITION_P2__ 
#define __APPLE_XATTR_POSITION_P3__ 
#endif
#endif

extern struct fuse_operations fuse_oper_struct;

void *multiplex_init(struct fuse_conn_info *conn);
int multiplex_getattr(const char *path, struct stat *stbuf);
int multiplex_fgetattr(const char *path, struct stat *stbuf, struct fuse_file_info *fi);
int multiplex_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
					  off_t offset, struct fuse_file_info *fi);
int multiplex_mknod(const char *path, mode_t mode, dev_t dev);
int multiplex_access(const char *path, int amode);
int multiplex_create(const char *path, mode_t mode, struct fuse_file_info *fi);
int multiplex_open(const char *path, struct fuse_file_info *fi);
int multiplex_read(const char *path, char *buf, size_t size, off_t offset,
				   struct fuse_file_info *fi);
int multiplex_write(const char *path, const char *buf, size_t size, off_t offset, struct fuse_file_info *fi);
int multiplex_release(const char *path, struct fuse_file_info *fi);
int multiplex_unlink(const char *path);
int multiplex_rmdir(const char *path);
int multiplex_mkdir(const char *path, mode_t mode);
int multiplex_truncate(const char *path, off_t length);
int multiplex_chmod(const char *path, mode_t mode);
int multiplex_chown(const char *path, uid_t uid, gid_t gid);
int multiplex_utimens(const char *path, const struct timespec tv[2]);
int multiplex_setxattr(const char *path, const char *name, const char *value, size_t size, int options __APPLE_XATTR_POSITION__ );
int multiplex_getxattr(const char *path, const char *name, char *value, size_t size __APPLE_XATTR_POSITION__ );
int multiplex_listxattr(const char *path, char *namebuf, size_t size);
int multiplex_removexattr(const char *path, const char *name);
int multiplex_statfs(const char *path, struct statvfs *statbuf);


#endif
