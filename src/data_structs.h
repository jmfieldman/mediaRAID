//
//  data_structs.h
//  mediaRAID
//
//  Created by Jason Fieldman on 9/10/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#ifndef mediaRAID_data_structs_h
#define mediaRAID_data_structs_h

#include <stdint.h>


/* dictionary */

struct dic_node {
	struct dic_node  *next;
	char             *key;
	char             *str_value;
	int64_t           int_value;
	
};

typedef struct {
	
	struct dic_node **buckets;
	int               bucket_count;
	
} Dictionary_t;

Dictionary_t *dictionary_create_with_size(int buckets);
void dictionary_set_int(Dictionary_t *dic, const char *key, int64_t value);
void dictionary_set_str(Dictionary_t *dic, const char *key, const char *val);
int dictionary_get_int(Dictionary_t *dic, const char *key, int64_t *value);
const char *dictionary_get_str(Dictionary_t *dic, const char *key);
void dictionary_remove_item(Dictionary_t *dic, const char *key);
void dictionary_destroy(Dictionary_t *dic);

#endif
