#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>
#include "func.h"
#include "value.h"

/* represents the type of symbol */
enum symbol_type {
	SYMBOL_TYPE_FN, /* function */
	SYMBOL_TYPE_VAR, /* variable */
	SYMBOL_TYPE_TYPE /* type */
};

/* represents a symbol */
struct symbol {
	enum symbol_type type; /* represents the type of symbol */
	union {
		struct variable as_var; /* symbol as a variable */
		struct func as_fn; /* symbol as a function */
		struct type as_type; /* symbol as a type */
	};
};

/* represents a symbol `table` */
struct symbol_table {
	size_t count; /* amount of items in the table */
	size_t capacity; /* capacity of the table */
	struct symbol *items; /* items in the table */
};

void symbol_table_init(struct symbol_table *table);
unsigned int symbol_table_add(struct symbol_table *table, struct symbol symbol);
struct symbol *symbol_table_find(struct symbol_table *table, char *name);

#endif /* SYMBOL_H */
