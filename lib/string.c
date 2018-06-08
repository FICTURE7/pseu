#include "string.h"

#define STRING_TABLE_INIT_CAPACITY 16

void string_table_init(struct string_table *strings) {
	strings->capacity = STRING_TABLE_INIT_CAPACITY;
	strings->count = 0;
	strings->items = malloc(sizeof(struct string *) * STRING_TABLE_INIT_CAPACITY);
}

void string_table_deinit(struct string_table *strings) {
	free(strings->items);
}

struct string *string_intern(struct string_table *strings, struct string *val) {
	
}
