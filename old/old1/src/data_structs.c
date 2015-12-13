//
//  data_structs.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/10/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "data_structs.h"
#include "exlog.h"



/* ----------- Hash ------------ */

static unsigned hash_str(const char *s, int modu) {
	unsigned int hashval;
	for (hashval = 0; *s != 0; s++) {
		hashval = *s + 31 * hashval;
	}
	int result = hashval % modu;
	return result;
}

static unsigned hash_int(int64_t i, int modu) {
	return (i * 31) % modu;
}


/* ------------------------------------- Dictionary ------------------------------------ */


Dictionary_t *dictionary_create_with_size(int buckets) {
	Dictionary_t *dic = (Dictionary_t*)malloc(sizeof(Dictionary_t));
	if (!dic) return dic;
	dic->buckets = malloc(buckets * sizeof(struct dic_node *));
	if (!dic->buckets) return NULL;
	memset(dic->buckets, 0, buckets * sizeof(struct dic_node *));
	dic->bucket_count = buckets;
	pthread_mutex_init(&dic->mutex, NULL);
	return dic;
}

static void __destroy_dic_node(struct dic_node *node) {
	if (!node) return;
	if (node->str_value) free(node->str_value);
	if (node->key) free(node->key);
	free(node);
}

static void __destroy_bucket(struct dic_node *bucket) {
	if (!bucket) return;
	__destroy_bucket(bucket->next);
	__destroy_dic_node(bucket);
}

static inline struct dic_node *__new_dic_node(const char *key) {
	struct dic_node *ptr = (struct dic_node *) malloc(sizeof(struct dic_node));
	if (!ptr) return ptr;
	memset(ptr, 0, sizeof(struct dic_node));
	
	size_t len = strlen(key);
	if (!len) return NULL;
	
	ptr->key = malloc(len+1);
	memcpy(ptr->key, key, len+1);
	
	return ptr;
}

static struct dic_node *__get_dictionary_node_with_key(Dictionary_t *dic, const char *key) {
	struct dic_node *np;
	for (np = dic->buckets[hash_str(key, dic->bucket_count)]; np != NULL; np = np->next) {
		if (strcmp(key, np->key) == 0) {
			return np; /* found */
		}
	}
	return NULL; /* not found */
}


void dictionary_set_int(Dictionary_t *dic, const char *key, int64_t value) {
	pthread_mutex_lock(&dic->mutex);
	struct dic_node *np = __get_dictionary_node_with_key(dic, key);
	if (np) {
		np->int_value = value;
		pthread_mutex_unlock(&dic->mutex);
		return;
	}
	
	np = __new_dic_node(key);
	if (!np) {
		pthread_mutex_unlock(&dic->mutex);
		return;
	}
	
	np->int_value = value;
	
	unsigned int hashval = hash_str(key, dic->bucket_count);
	np->next = dic->buckets[hashval];
	dic->buckets[hashval] = np;
	
	pthread_mutex_unlock(&dic->mutex);
}

void dictionary_set_str(Dictionary_t *dic, const char *key, const char *val) {
	pthread_mutex_lock(&dic->mutex);
	struct dic_node *np = __get_dictionary_node_with_key(dic, key);
	if (np) {
		if (np->str_value) free(np->str_value);
		np->str_value = NULL;
		if (val) {
			size_t len = strlen(val);
			np->str_value = malloc(len+1);
			memcpy(np->str_value, val, len+1);
		}		
		pthread_mutex_unlock(&dic->mutex);
		return;
	}
	
	np = __new_dic_node(key);
	if (!np) {
		pthread_mutex_unlock(&dic->mutex);
		return;
	}
	
	if (val) {
		size_t len = strlen(val);
		np->str_value = malloc(len+1);
		memcpy(np->str_value, val, len+1);
	}
	
	unsigned int hashval = hash_str(key, dic->bucket_count);
	np->next = dic->buckets[hashval];
	dic->buckets[hashval] = np;
	pthread_mutex_unlock(&dic->mutex);
}

void dictionary_set_int_str(Dictionary_t *dic, const char *key, int64_t int_value, const char *str_val) {
	pthread_mutex_lock(&dic->mutex);
	struct dic_node *np = __get_dictionary_node_with_key(dic, key);
	if (np) {
		np->int_value = int_value;
		if (np->str_value) free(np->str_value);
		np->str_value = NULL;
		if (str_val) {
			size_t len = strlen(str_val);
			np->str_value = malloc(len+1);
			memcpy(np->str_value, str_val, len+1);
		}
		pthread_mutex_unlock(&dic->mutex);
		return;
	}
	
	np = __new_dic_node(key);
	if (!np) {
		pthread_mutex_unlock(&dic->mutex);
		return;
	}
	
	np->int_value = int_value;
	
	if (str_val) {
		size_t len = strlen(str_val);
		np->str_value = malloc(len+1);
		memcpy(np->str_value, str_val, len+1);
	}
	
	unsigned int hashval = hash_str(key, dic->bucket_count);
	np->next = dic->buckets[hashval];
	dic->buckets[hashval] = np;
	pthread_mutex_unlock(&dic->mutex);
}


int dictionary_get_int(Dictionary_t *dic, const char *key, int64_t *value) {
	pthread_mutex_lock(&dic->mutex);
	struct dic_node *np;
	for (np = dic->buckets[hash_str(key, dic->bucket_count)]; np != NULL; np = np->next) {
		if (strcmp(key, np->key) == 0) {
			if (value) *value = np->int_value;
			pthread_mutex_unlock(&dic->mutex);
			return 1; /* found */
		}
	}
	pthread_mutex_unlock(&dic->mutex);
	return 0; /* not found */
}

int dictionary_get_str(Dictionary_t *dic, const char *key, char *value) {
	pthread_mutex_lock(&dic->mutex);
	struct dic_node *np;
	for (np = dic->buckets[hash_str(key, dic->bucket_count)]; np != NULL; np = np->next) {
		if (strcmp(key, np->key) == 0) {
			if (value) strcpy(value, np->str_value);
			pthread_mutex_unlock(&dic->mutex);
			return 1; /* found */
		}
	}
	pthread_mutex_unlock(&dic->mutex);
	return 0; /* not found */
}

int dictionary_get_int_str(Dictionary_t *dic, const char *key, int64_t *int_value, char *str_value) {
	pthread_mutex_lock(&dic->mutex);
	struct dic_node *np;
	for (np = dic->buckets[hash_str(key, dic->bucket_count)]; np != NULL; np = np->next) {
		if (strcmp(key, np->key) == 0) {
			if (int_value) *int_value = np->int_value;
			if (str_value) strcpy(str_value, np->str_value);
			pthread_mutex_unlock(&dic->mutex);
			return 1; /* found */
		}
	}
	pthread_mutex_unlock(&dic->mutex);
	return 0; /* not found */
}

void dictionary_remove_item(Dictionary_t *dic, const char *key) {
	unsigned int hashval = hash_str(key, dic->bucket_count);
	struct dic_node *np = dic->buckets[hashval];
	
	if (np == NULL) return;
	
	if (!strcmp(np->key, key)) {
		dic->buckets[hashval] = np->next;
		__destroy_dic_node(np);
		return;
	}
	
	while (np->next) {
		if (!strcmp(np->next->key, key)) {
			struct dic_node *t = np->next;
			np->next = np->next->next;
			__destroy_dic_node(t);
			return;
		}
		np = np->next;
	}
}

void dictionary_destroy(Dictionary_t *dic) {
	if (!dic) return;
	for (int i = 0; i < dic->bucket_count; i++) {
		__destroy_bucket(dic->buckets[i]);
	}
	free(dic);
}




/* ------------------------------------- Linked List ------------------------------------ */

LinkedList_t *linked_list_create() {
	LinkedList_t *list = (LinkedList_t*)malloc(sizeof(LinkedList_t));
	if (!list) return list;
	
	list->front = NULL;
	list->back  = NULL;
	pthread_mutex_init(&list->mutex, NULL);
	
	return list;
}

void linked_list_init(LinkedList_t *list) {
	list->front = NULL;
	list->back = NULL;
	pthread_mutex_init(&list->mutex, NULL);
}

void __list_node_destroy(struct list_node *node) {
	if (!node) return;
	__list_node_destroy(node->next);
	if (node->data) free(node->data);
}

void linked_list_destroy(LinkedList_t *list) {
	if (!list) return;
	pthread_mutex_lock(&list->mutex);
	__list_node_destroy(list->front);
	list->front = NULL;
	list->back = NULL;
	pthread_mutex_unlock(&list->mutex);
	free(list);
	
}

void linked_list_push(LinkedList_t *list, int front, void *data) {
	if (!list) return;
	
	struct list_node *node = (struct list_node*)malloc(sizeof(struct list_node));
	if (!node) return;
	
	pthread_mutex_lock(&list->mutex);
	
	node->next = NULL;
	node->data = data;
	
	if (front || !list->front) {
		node->next = list->front;
		list->front = node;
		if (!list->back) list->back = node;
	} else {
		list->back->next = node;
		list->back = node;
	}
	
	pthread_mutex_unlock(&list->mutex);
}

void *linked_list_pop_front(LinkedList_t *list) {
	if (!list) return NULL;
	
	void *response_data = NULL;
	
	pthread_mutex_lock(&list->mutex);
	
	if (!list->front) {
		pthread_mutex_unlock(&list->mutex);
		return NULL;
	}
	
	response_data = list->front->data;
	struct list_node *old_front = list->front;
	if (list->back == list->front) {
		list->back = NULL;
	}
	list->front = list->front->next;
	free(old_front);
	
	pthread_mutex_unlock(&list->mutex);
	return response_data;
}


void *linked_list_peek(LinkedList_t *list, int front) {
	if (!list) return NULL;
	
	void *response_data = NULL;
	pthread_mutex_lock(&list->mutex);
	
	if (!list->front) {
		pthread_mutex_unlock(&list->mutex);
		return NULL;
	}
	
	if (front) {
		response_data = list->front->data;
	} else {
		response_data = list->back->data;
	}
	
	pthread_mutex_unlock(&list->mutex);
	return response_data;
}




/* ------------------------------------- Tiered Priority Queue ------------------------------------ */

TieredPriorityQueue_t *tiered_priority_queue_create() {
	TieredPriorityQueue_t *queue = (TieredPriorityQueue_t*)malloc(sizeof(TieredPriorityQueue_t));
	if (!queue) return queue;
	
	for (int i = 0; i < TIERED_PRIORITY_QUEUE_LEVELS; i++) {
		linked_list_init(&queue->lists[i]);
	}
	
	pthread_mutex_init(&queue->mutex, NULL);
	
	return queue;
}

void tiered_priority_queue_push(TieredPriorityQueue_t *queue, int priority, int front, void *data) {
	if (priority >= TIERED_PRIORITY_QUEUE_LEVELS) priority = TIERED_PRIORITY_QUEUE_LEVELS-1;
	if (priority < 0) priority = 0;
	pthread_mutex_lock(&queue->mutex);
	linked_list_push(&queue->lists[priority], front, data);
	pthread_mutex_unlock(&queue->mutex);
}

void *tiered_priority_queue_pop(TieredPriorityQueue_t *queue) {
	void *resp_data = NULL;
	pthread_mutex_lock(&queue->mutex);
	for (int i = 0; i < TIERED_PRIORITY_QUEUE_LEVELS; i++) {
		resp_data = linked_list_pop_front(&queue->lists[i]);
		if (resp_data) {
			break;
		}
	}
	pthread_mutex_unlock(&queue->mutex);
	return resp_data;
}

void *tiered_priority_queue_pop_priority(TieredPriorityQueue_t *queue, int priority) {
	if (priority >= TIERED_PRIORITY_QUEUE_LEVELS) priority = TIERED_PRIORITY_QUEUE_LEVELS-1;
	if (priority < 0) priority = 0;
	
	pthread_mutex_lock(&queue->mutex);
	void *resp_data = linked_list_pop_front(&queue->lists[priority]);
	pthread_mutex_unlock(&queue->mutex);
	return resp_data;
}
