#ifndef PSEU_SYMBOL_H
#define PSEU_SYMBOL_H

#include <stdlib.h>

#include "pseu_value.h"
#include "pseu_vector.h"

/*
 * Represents the symbol table of a pseu virtual machine instance.
 */
struct symbol_table {
	/* Pseu virtual machine instance which owns this symbol table. */
	pseu_vm_t *vm;
	/* List of functions in the symbols table. */
	struct vector fns;
	/* List of variables in the symbols table. */
	struct vector vars;
	/* List of types in the symbols table. */
	struct vector types;
};

void symbol_table_init(pseu_vm_t *vm, struct symbol_table *table);
void symbol_table_deinit(struct symbol_table *table);

int symbol_table_add_type(struct symbol_table *table,
				struct type *type);
int symbol_table_add_function(struct symbol_table *table,
				struct function *fn);
int symbol_table_add_variable(struct symbol_table *table,
				struct variable *var);

int symbol_table_get_type(struct symbol_table *table, const char *ident);
int symbol_table_get_function(struct symbol_table *table, const char *ident);
int symbol_table_get_variable(struct symbol_table *table, const char *ident);

#endif /* PSEU_SYMBOL_H */
