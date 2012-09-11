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
	
	/* We have a helper function that pulls the stat struct from the first file in the active volume list */
	return volume_stat_of_any_active_file(path, stbuf);
		 
	return 0;
}

int multiplex_readdir(const char *path, void *buf, fuse_fill_dir_t filler,
					  off_t offset, struct fuse_file_info *fi) {
	
	EXLog(FUSE, INFO, "multiplex_readdir [%s]", path);

	/* Fill out basic dot dirs */
	filler(buf, ".", NULL, 0);
	filler(buf, "..", NULL, 0);

	/* Get all the DIR entries for the active volumes */
	DIR **entries = volume_active_dir_entries(path);
	DIR **entries_start = entries;
	if (entries) {
		/* create a hash table that will help detect duplicate entries */
		Dictionary_t *dic = dictionary_create_with_size(512);
		int64_t tmp;
		while (*entries) {
			/* For each DIR, iterate through and grab entries */
			DIR *entry = *entries;
			struct dirent *dent = readdir(entry);
			while (dent) {
				
				if (strcmp("..", dent->d_name) && strcmp(".", dent->d_name)) {
					/* For each entry, add it to the list if it doesn't exist in the hash table */
					if (!dictionary_get_int(dic, dent->d_name, &tmp)) {
						dictionary_set_int(dic, dent->d_name, tmp);
						EXLog(FUSE, DBG, "Adding dir entry [%s]", dent->d_name);
						filler(buf, dent->d_name, NULL, 0);
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
	
	return 0;
}

int multiplex_open(const char *path, struct fuse_file_info *fi) {
	
	return 0;
}

int multiplex_read(const char *path, char *buf, size_t size, off_t offset,
				   struct fuse_file_info *fi) {
	return 0;
}

