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




/* ----------- Hash ------------ */

static unsigned hash_str(const char *s, int modu) {
	unsigned int hashval;
	for (hashval = 0; *s != 0; s++) {
		hashval = *s + 31 * hashval;
	}
	return hashval % modu;
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
	memcpy(ptr->key, key, len);
	
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
			memcpy(np->str_value, val, len);
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
		memcpy(np->str_value, val, len);
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
			memcpy(np->str_value, str_val, len);
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
		memcpy(np->str_value, str_val, len);
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
			*value = np->int_value;
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
			*int_value = np->int_value;
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
