#include <string.h>
#include "value.h"
#include "symbol.h"

#define SYMBOL_TABLE_INIT_CAPACITY 16

/* TODO: use hashtables instead */

void symbol_table_init(struct symbol_table *table) {
	vector_init(&table->types);
	vector_init(&table->fns);
	vector_init(&table->vars);
}

void symbol_table_deinit(struct symbol_table *table) {
	vector_deinit(&table->types);
	vector_deinit(&table->fns);
	vector_deinit(&table->vars);
}

void symbol_table_add_type(struct symbol_table *table, struct type *type) {
	vector_add(&table->types, type);
}

void symbol_table_add_func(struct symbol_table *table, struct func *fn) {
	vector_add(&table->fns, fn);
}

void symbol_table_add_variable(struct symbol_table *table, struct variable *var) {	
	vector_add(&table->vars, var);
}

struct type *symbol_table_get_type(struct symbol_table *table, const char *ident) {
	for (size_t i = 0; i < table->types.count; i++) {
		struct type *type = table->types.items[i];
		if (!strcmp(type->ident, ident)) {
			return type;
		}
	}

	return NULL;
}

struct func *symbol_table_get_func(struct symbol_table *table, const char *ident) {
	for (size_t i = 0; i < table->fns.count; i++) {
		struct func *fn = table->fns.items[i];
		if (!strcmp(fn->proto->ident, ident)) {
			return fn;
		}
	}

	return NULL;
}

struct variable *symbol_table_get_variable(struct symbol_table *table, const char *ident) {
	for (size_t i = 0; i < table->vars.count; i++) {
		struct variable *var = table->vars.items[i];
		if (!strcmp(var->ident, ident)) {
			return var;
		}
	}

	return NULL;
}
