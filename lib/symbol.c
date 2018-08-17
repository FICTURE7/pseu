#include "symbol.h"

#define SYMBOL_TABLE_INIT_CAPACITY 16

static inline void table_grow(struct symbol_table *table) {
	size_t new_capacity = table->capacity * 2;
	struct symbol *items = realloc(table->items, new_capacity);

	if (items != NULL) {
		table->capacity = new_capacity;
		table->items = items;
	}
}

void symbol_table_init(struct symbol_table *table) {
	table->count = 0;
	table->capacity = SYMBOL_TABLE_INIT_CAPACITY;
	table->items = malloc(sizeof(struct symbol) * table->capacity);
}

unsigned int symbol_table_add(struct symbol_table *table, struct symbol symbol) {
	if (table->count >= table->capacity) {
		table_grow(table);
	}

	table->items[table->count] = symbol;
	return (unsigned int)table->count++;
}
