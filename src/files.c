//
//  files.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/11/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <pthread.h>

#include "files.h"

/* File handle operations */
static Dictionary_t *open_files;
pthread_mutex_t open_files_mutex = PTHREAD_MUTEX_INITIALIZER;


void init_open_fh_table() {
	open_files = dictionary_create_with_size(512);
}

void set_open_fh_for_path(const char *path, int64_t fh, char *volume_basepath) /* Sending -1 as FH removes entry */ {
	pthread_mutex_lock(&open_files_mutex);
	if (fh < 0) {
		dictionary_remove_item(open_files, path);
	} else {
		dictionary_set_int_str(open_files, path, fh, volume_basepath);
	}
	pthread_mutex_unlock(&open_files_mutex);
}

int get_open_fh_for_path(const char *path, int64_t *fh, char *volume_basepath) {
	pthread_mutex_lock(&open_files_mutex);
	int ret = dictionary_get_int_str(open_files, path, fh, volume_basepath);
	pthread_mutex_unlock(&open_files_mutex);
	return ret;
}

