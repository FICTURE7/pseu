#ifndef STRING_H
#define STRING_H

#include <pseu.h>
#include <stdlib.h>

#include "pseu_value.h"

/*
 * Represents a string table entry.
 */
struct string_table_entry {
	struct string_object *item;
	/* Next item in the link list/chain. */
	struct string_table_entry *next;
};

/*
 * Represents a string table.
 */
struct string_table {
	/* Pseu virtual machine instance which owns this string table. */
	pseu_vm_t *vm;

	/* Number of strings in the string table. */
	size_t count;
	/* Capacity of the string table. */
	size_t capacity;
	/* Entries in the string table. */
	struct string_table_entry **entries; 
};

/*
 *
 */
void string_table_init(pseu_vm_t *vm, struct string_table *table);

/*
 *
 */
void string_table_deinit(struct string_table *table);

/*
 *
 */
struct string_object *string_table_intern(struct string_table *table,
					char *value, size_t length);

#endif /* STRING_H */
