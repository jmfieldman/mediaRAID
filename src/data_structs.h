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
#include <pthread.h>

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
	pthread_mutex_t   mutex;
	
} Dictionary_t;

Dictionary_t *dictionary_create_with_size(int buckets);
void dictionary_set_int(Dictionary_t *dic, const char *key, int64_t value);
void dictionary_set_str(Dictionary_t *dic, const char *key, const char *val);
void dictionary_set_int_str(Dictionary_t *dic, const char *key, int64_t int_value, const char *str_val);
int dictionary_get_int(Dictionary_t *dic, const char *key, int64_t *value);
int dictionary_get_str(Dictionary_t *dic, const char *key, char *value);
int dictionary_get_int_str(Dictionary_t *dic, const char *key, int64_t *int_value, char *str_value);
void dictionary_remove_item(Dictionary_t *dic, const char *key);
void dictionary_destroy(Dictionary_t *dic);


/* Linked List */

struct list_node {
	struct list_node  *next;
	void              *data;
};

typedef struct {
	
	struct list_node  *front;
	struct list_node  *back;
	pthread_mutex_t    mutex;
	
} LinkedList_t;

LinkedList_t *linked_list_create();
void linked_list_destroy(LinkedList_t *list);
void linked_list_push(LinkedList_t *list, int front, void *data);
void *linked_list_pop_front(LinkedList_t *list); /* caller must free() response void* */
void *linked_list_peek(LinkedList_t *list, int front); /* caller must NOT free() response */


#endif
