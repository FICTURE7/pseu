#ifndef STRING_H
#define STRING_H

#include <stdlib.h>

struct string_table_entry {
	struct string *val;
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

struct string {
	unsigned int hash;
	size_t len;
	char *buf;
};

void string_table_init(struct string_table *table);

void string_table_deinit(struct string_table *table);

struct string *string_table_intern(struct string_table *table, char *val, size_t len);

#endif /* STRING_H */
