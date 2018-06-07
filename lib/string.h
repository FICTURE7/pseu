#ifndef STRING_H
#define STRING_H

#include <stdlib.h>

struct string_table {
	int count;
	size_t capacity;
	struct string **strings;
};

struct string {
	unsigned int hash;
	size_t len;
	char *val;
};

void string_table_init(struct string_table *string_table);

#endif /* STRING_H */
