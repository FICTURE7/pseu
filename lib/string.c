#include "string.h"

#define STRING_TABLE_INIT_CAPACITY 16

void string_table_init(struct string_table *string_table) {
	string_table->capacity = STRING_TABLE_INIT_CAPACITY;
	string_table->count = 0;
	string_table->strings = malloc(sizeof(struct string *) * STRING_TABLE_INIT_CAPACITY);
}

struct string *string_intern(struct string_table *string_table, struct string *string) {

}
