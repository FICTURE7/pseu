#ifndef SYMBOL_H
#define SYMBOL_H

#include <stdlib.h>
#include "value.h"
#include "vector.h"

/* represents a symbol `table` */
struct symbol_table {
	struct vector types;
	struct vector fns;
	struct vector vars;
};

void symbol_table_init(struct symbol_table *table);
void symbol_table_deinit(struct symbol_table *table);

/* add stuff to the symbol table */
void symbol_table_add_type(struct symbol_table *table, struct type *type);
void symbol_table_add_func(struct symbol_table *table, struct func *fn);
void symbol_table_add_variable(struct symbol_table *table, struct variable *var);

/* find stuff from the table */
struct type *symbol_table_get_type(struct symbol_table *table, const char *ident);
struct func *symbol_table_get_func(struct symbol_table *table, const char *ident);
struct variable *symbol_table_get_variable(struct symbol_table *table, const char *ident);

#endif /* SYMBOL_H */
