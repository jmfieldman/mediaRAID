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
	struct dic_node *np = __get_dictionary_node_with_key(dic, key);
	if (np) {
		np->int_value = value;
		return;
	}
	
	np = __new_dic_node(key);
	if (!np) return;
	
	np->int_value = value;
	
	unsigned int hashval = hash_str(key, dic->bucket_count);
	np->next = dic->buckets[hashval];
	dic->buckets[hashval] = np;
	
}

void dictionary_set_str(Dictionary_t *dic, const char *key, const char *val) {
	struct dic_node *np = __get_dictionary_node_with_key(dic, key);
	if (np) {
		if (np->str_value) free(np->str_value);
		size_t len = strlen(val);
		np->str_value = malloc(len+1);
		memcpy(np->str_value, val, len);
		return;
	}
	
	np = __new_dic_node(key);
	if (!np) return;
	
	size_t len = strlen(val);
	np->str_value = malloc(len+1);
	memcpy(np->str_value, val, len);
	
	unsigned int hashval = hash_str(key, dic->bucket_count);
	np->next = dic->buckets[hashval];
	dic->buckets[hashval] = np;
}


int dictionary_get_int(Dictionary_t *dic, const char *key, int64_t *value) {
	struct dic_node *np;
	for (np = dic->buckets[hash_str(key, dic->bucket_count)]; np != NULL; np = np->next) {
		if (strcmp(key, np->key) == 0) {
			*value = np->int_value;
			return 1; /* found */
		}
	}
	return 0; /* not found */
}

const char *dictionary_get_str(Dictionary_t *dic, const char *key) {
	struct dic_node *np;
	for (np = dic->buckets[hash_str(key, dic->bucket_count)]; np != NULL; np = np->next) {
		if (strcmp(key, np->key) == 0) {
			return np->str_value; /* found */
		}
	}
	return NULL; /* not found */
}

void dictionary_remove_item(Dictionary_t *dic, const char *key) {
	
}

void dictionary_destroy(Dictionary_t *dic) {
	if (!dic) return;
	for (int i = 0; i < dic->bucket_count; i++) {
		__destroy_bucket(dic->buckets[i]);
	}
	free(dic);
}
