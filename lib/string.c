#include "string.h"
#include <string.h>

#define STRING_TABLE_INIT_CAPACITY 16

static unsigned int hash_djb2_string(char *val, size_t len) {
	unsigned int hash = 5381;
	for (unsigned int i = 0; i < len; i++) {
		hash = ((hash << 5) + hash) + val[i];
	}
	return hash;
}

static struct string *string_new(char *val, size_t len, unsigned int hash) {
	struct string *string = malloc(sizeof(struct string));
	string->val = val;
	string->len = len;
	string->hash = hash;
	return string;
}

void string_table_init(struct string_table *table) {
	table->capacity = STRING_TABLE_INIT_CAPACITY;
	table->count = 0;
	table->entries = calloc(1, sizeof(struct string_table_entry *) * STRING_TABLE_INIT_CAPACITY);
}

void string_table_deinit(struct string_table *table) {
	free(table->entries);
}

struct string *string_table_intern(struct string_table *table, char *val, size_t len) {
	/* todo: lot of stuff */
	int hash = hash_djb2_string(val, len);
	int index = hash % table->capacity;
	struct string *string = NULL;
	struct string_table_entry *entry = table->entries[index];

	if (entry == NULL) {
		/* create new entry if not found */
		string = string_new(val, len, hash);
		entry = malloc(sizeof(struct string_table_entry));
		entry->val = string;
		entry->next = NULL;

		table->entries[index] = entry;
	} else {
		struct string_table_entry *previous;
		do {
			if (entry->val->hash == hash && entry->val->len == len && memcmp(entry->val->val, val, len) == 0) {
				/* found interned string */
				string = entry->val;
				break;
			}

			previous = entry;
			entry = entry->next;
		} while (entry != NULL);
	}

	return string;
}
