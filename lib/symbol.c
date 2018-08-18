#include <string.h>
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

struct symbol *symbol_table_find(struct symbol_table *table, char *name) {
	for (size_t i = 0; i < table->count; i++) {
		struct symbol *symbol = &table->items[i];
		switch (symbol->type) {
			case SYMBOL_TYPE_FN:
				if (!strcmp(symbol->as_fn.proto->ident, name)) {
					return symbol;
				}
				break;
			case SYMBOL_TYPE_VAR:
				if (!strcmp(symbol->as_var.ident, name)) {
					return symbol;
				}
				break;
			case SYMBOL_TYPE_TYPE:
				if (!strcmp(symbol->as_type.ident, name)) {
					return symbol;
				}
				break;
		}
	}

	return NULL;
}
