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
#include "volumes.h"
#include "httpd.h"
#include "data_structs.h"


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
	
	/* Initialize httpd daemon */
	start_httpd_daemon(g_application_options.server_port);
	
	EXLog(FUSE, INFO, "multiplex_init finished");
	
	return 0;
}

int multiplex_getattr(const char *path, struct stat *stbuf) {
	memset(stbuf, 0, sizeof(struct stat));
	stbuf->st_mode = S_IFREG | 0444;
	stbuf->st_nlink = rand();
	stbuf->st_size = rand();
	
	return 0;
}

int multiplex_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
					  off_t offset, struct fuse_file_info *fi) {
	
	EXLog(FUSE, INFO, "multiplex_readdir [%s]", path);
	
	DIR **entries = volume_active_dir_entries(path);
	if (entries) {
		Dictionary_t *dic = dictionary_create_with_size(512);
		int64_t tmp;
		while (*entries) {
			DIR *entry = *entries;
			struct dirent *dent = readdir(entry);
			while (dent) {
				
				if (strcmp("..", dent->d_name)) {
					if (!dictionary_get_int(dic, dent->d_name, &tmp)) {
						dictionary_set_int(dic, dent->d_name, tmp);
						filler(buf, dent->d_name, NULL, 0);
					}
				}
				
				dent = readdir(entry);
			}
			entries++;
		}
		dictionary_destroy(dic);
		free(entries);
	}
	
	if (strncmp("..", path, 3)) {
		filler(buf, "..", NULL, 0);
	}
	
	
	return 0;
}

int multiplex_open(const char *path, struct fuse_file_info *fi) {
	
	return 0;
}

int multiplex_read(const char *path, char *buf, size_t size, off_t offset,
				   struct fuse_file_info *fi) {
	return 0;
}

