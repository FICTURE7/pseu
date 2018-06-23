#ifndef STRING_H
#define STRING_H

#include <stdlib.h>
#include "value.h"

/*
 * string table entry which
 * contains a reference to a string object
 * and a seperate chain 'next'
 */
struct string_table_entry {
	struct string_object *item;
	/* next item in the link list/chain */
	struct string_table_entry *next;
};

/*
 * string table is a hashtable with a
 * seperate chain strategy for collisions
 */
struct string_table {
	size_t count;
	size_t capacity;
	struct string_table_entry **entries;
};

void string_table_init(struct string_table *table);

void string_table_deinit(struct string_table *table);

struct string_object *string_table_intern(struct string_table *table, char *val, size_t len);

#endif /* STRING_H */
