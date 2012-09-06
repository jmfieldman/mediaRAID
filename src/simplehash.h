//
//  simplehash.h
//  mediaRAID
//
//  Created by Jason Fieldman on 9/6/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#ifndef mediaRAID_simplehash_h
#define mediaRAID_simplehash_h

void init_open_fh_table();

/* Sending -1 as FH removes entry */
void set_open_fh_for_path(const char *path, int64_t fh);

/* Returns -1 if doesn't exist */
int64_t get_open_fh_for_path(const char *path);

#endif
