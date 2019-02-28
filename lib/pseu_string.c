#include <assert.h>
#include <string.h>

#include "pseu_string.h"

#define STRING_TABLE_INIT_CAPACITY 16

static unsigned int hash_djb2(char *buffer, size_t length) {
	unsigned int hash = 5381;
	for (unsigned int i = 0; i < length; i++) {
		hash = ((hash << 5) + hash) + buffer[i];
	}
	return hash;
}

static struct string_table_entry *table_entry_new(struct string_object *val) {
	struct string_table_entry *entry = malloc(sizeof(struct string_table_entry));
	entry->item = val;
	entry->next = NULL;
	return entry;
}

static void table_resize(struct string_table *table, size_t capacity) {
	struct string_table_entry **entries = realloc(table->entries, sizeof(struct string_table_entry *) * capacity);
	/* realloc failed */
	if (entries == NULL) {
		return;
	}

	size_t old_capacity = table->capacity;
	table->entries = entries;
	table->capacity = capacity;

	/* rehash old part of the table if the table has grown */
	if (old_capacity < capacity) {
		for (unsigned int i = 0; i < old_capacity; i++) {
			struct string_table_entry *entry = table->entries[i];
			table->entries[i] = NULL;

			while (entry != NULL) {
				struct string_table_entry *next = entry->next;
				unsigned int index = entry->item->hash % table->capacity;

				/* insert at the start of the chain/list */
				entry->next = table->entries[index];
				table->entries[index] = entry;
				entry = next;
			}
		}
	}
}

void string_table_init(pseu_vm_t *vm, struct string_table *table) {
	assert(table && vm);

	table->vm = vm;
	table->capacity = STRING_TABLE_INIT_CAPACITY;
	table->count = 0;
	table->entries = calloc(1, sizeof(struct string_table_entry *) * STRING_TABLE_INIT_CAPACITY);
}

void string_table_deinit(struct string_table *table) {
	/* free the entries */
	for (unsigned int i = 0; i < table->count; i++) {
		struct string_table_entry *entry = table->entries[i];
		if (entry == NULL) {
			continue;
		}

		/* free the seperate chains */
		struct string_table_entry *next;
		for (; entry != NULL; entry = next) {
			next = entry->next;
			free(entry);
		}
	}

	free(table->entries);
}

struct string_object *string_table_intern(struct string_table *table, char *buffer, size_t length) {
	unsigned int hash = hash_djb2(buffer, length);
	unsigned int index = hash % table->capacity;
	struct string_object *string = NULL;
	struct string_table_entry *entry = table->entries[index];

	if (entry == NULL) {
		/* create new entry if not found */
		//string = object_new_string(buffer, length, hash);
		entry = table_entry_new(string);

		table->entries[index] = entry;
		table->count++;
	} else {
		do {
			/*
			 * compare the hash, length and content
			 * of the string to find a matching interned string
			 */
			if (entry->item->hash == hash && entry->item->length == length && memcmp(entry->item->buffer, buffer, length) == 0) {
				string = entry->item;
				return string;
			}
			entry = entry->next;
		} while (entry != NULL);

		/* grow table if needed */
		if (table->count >= table->capacity) {
			table_resize(table, table->capacity * 2);
			entry = table->entries[hash % table->capacity];
		}

		/* string not found in chain, create a new one and extend chain */
		//string = object_new_string(buffer, length, hash);
		entry = table_entry_new(string);

		/* insert at the start of the chain/list */
		entry->next = table->entries[index];
		table->entries[index] = entry;
	}

	return string;
}
