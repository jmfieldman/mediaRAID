//
//  files.h
//  mediaRAID
//
//  Created by Jason Fieldman on 9/11/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#ifndef mediaRAID_files_h
#define mediaRAID_files_h

#include "data_structs.h"


void init_open_fh_table();

/* Sending -1 as FH removes entry */
void set_open_fh_for_path(const char *path, int64_t fh, char *volume_basepath);

/* Returns 0 if doesn't exist; 1 if does.  volume_basepath must be a pre-allocated buffer than gets the path, or NULL if unwanted */
int get_open_fh_for_path(const char *path, int64_t *fh, char *volume_basepath);


#endif
