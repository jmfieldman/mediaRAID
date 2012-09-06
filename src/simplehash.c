//
//  simplehash.c
//  mediaRAID
//
//  Created by Jason Fieldman on 9/6/12.
//  Copyright (c) 2012 Jason Fieldman. All rights reserved.
//

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "simplehash.h"



struct nlist_int {          /* table entry: */
	struct nlist_int *next; /* next entry in chain */
	char *name;             /* defined name */
	int64_t value;          /* value */
};

#define SIMPLE_HASHSIZE 128
#define SIMPLE_HASHSIZE_MASK 0x7F

/* hash: form hash value for string s */
static unsigned hash(const char *s) {
	unsigned int hashval;
	for (hashval = 0; *s != 0; s++) {
		hashval = *s + 31 * hashval;
	}
	return hashval & SIMPLE_HASHSIZE_MASK;
}

/* lookup: look for s in hashtab */
static struct nlist_int *lookup_int(struct nlist_int **table, const char *s)
{
	struct nlist_int *np;
	for (np = table[hash(s)]; np != NULL; np = np->next)
		if (strcmp(s, np->name) == 0)
			return np; /* found */
	return NULL; /* not found */
}


static char *strdup_h(const char *s) /* make a duplicate of s */
{
    char *p;
    p = (char *) malloc(strlen(s)+1); /* +1 for ’\0’ */
    if (p != NULL)
		strcpy(p, s);
    return p;
}

/* install: put (name, defn) in hashtab */
static struct nlist_int *put_int(struct nlist_int **table, const char *name, int64_t value)
{
    struct nlist_int *np;
    unsigned int hashval;
    if ((np = lookup_int(table, name)) == NULL) { /* not found */
        np = (struct nlist_int *) malloc(sizeof(*np));
        if (np == NULL || (np->name = strdup_h(name)) == NULL)
			return NULL;
        hashval = hash(name);
        np->next = table[hashval];
        table[hashval] = np;
    }
	np->value = value;
    return np;
}

static void killnode_int(struct nlist_int *np) {
	free(np->name);
	free(np);
}

static void remove_int(struct nlist_int **table, const char *name) {
	unsigned int hashval = hash(name);
	struct nlist_int *np = table[hashval];
	
	if (np == NULL) return;
	
	if (!strcmp(np->name, name)) {
		table[hashval] = np->next;
		killnode_int(np);
		return;
	}
	
	while (np->next) {
		if (!strcmp(np->next->name, name)) {
			struct nlist_int *t = np->next;
			np->next = np->next->next;
			killnode_int(t);
			return;
		}
		np = np->next;
	}
}



/* File handle operations */
static struct nlist_int *open_files[SIMPLE_HASHSIZE]; /* pointer table */
pthread_mutex_t open_files_mutex = PTHREAD_MUTEX_INITIALIZER;

void init_open_fh_table() {
	memset(open_files, 0, sizeof(open_files));
}

void set_open_fh_for_path(const char *path, int64_t fh) /* Sending -1 as FH removes entry */ {
	pthread_mutex_lock(&open_files_mutex);
	if (fh < 0) {
		remove_int(open_files, path);
	} else {
		put_int(open_files, path, fh);
	}
	pthread_mutex_unlock(&open_files_mutex);
}

int64_t get_open_fh_for_path(const char *path) {
	pthread_mutex_lock(&open_files_mutex);
	int64_t val = -1;
	struct nlist_int *lookup = lookup_int(open_files, path);
	if (lookup != NULL) val = lookup->value;
	pthread_mutex_unlock(&open_files_mutex);
	return val;
}




