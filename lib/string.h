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
	struct string_table_entry *next; /* next item in the link list/chain */
};

/*
 * string table is a hashtable with a
 * seperate chain strategy for collisions
 */
struct string_table {
	size_t count; /* number of strings in the string table */
	size_t capacity; /* capacity of the string table before resizing it */
	struct string_table_entry **entries; /* entries in the string table */
};

void string_table_init(struct string_table *table);
void string_table_deinit(struct string_table *table);
struct string_object *string_table_intern(struct string_table *table, char *val, size_t len);

#endif /* STRING_H */
