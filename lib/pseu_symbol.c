#include <assert.h>
#include <string.h>

#include <pseu.h>

#include "pseu_vm.h"
#include "pseu_value.h"
#include "pseu_debug.h"
#include "pseu_symbol.h"
#include "pseu_vector.h"

static void closure_free(pseu_vm_t *vm, struct closure *closure) {
	pseu_free(vm, closure->consts);
	pseu_free(vm, closure->locals);
	pseu_free(vm, closure->code);
}

static void function_free(pseu_vm_t *vm, struct function *fn) {
	if (fn->ident) {
		/*
		 * TODO: Allocate primitive fn's ident on the heap as well.
		 */

		/* pseu_free(vm, (void *)fn->ident); */
	}

	if (fn->params_types) {
		pseu_assert(fn->params_count > 0);
		pseu_free(vm, fn->params_types);
	}

	if (fn->type == FN_TYPE_USER) {
		closure_free(vm, fn->as_closure);
	}

	pseu_free(vm, fn);
}

void symbol_table_init(pseu_vm_t *vm, struct symbol_table *table) {
	pseu_assert(vm && table);
	
	table->vm = vm;
	vector_init(vm, &table->fns, 16);
	vector_init(vm, &table->vars, 16);
	vector_init(vm, &table->types, 16);
}

void symbol_table_deinit(struct symbol_table *table) {
	pseu_assert(table);

	/* Free functions in list. */
	for (size_t i = 0; i < table->fns.count; i++) {
		function_free(table->vm, table->fns.data[i]);
	}

	/* Free variables in list. */
	for (size_t i = 0; i < table->vars.count; i++) {
		pseu_free(table->vm, table->vars.data[i]);
	}

	/* Free types in list. */
	for (size_t i = 0; i < table->types.count; i++) {
		/* 
		 * Free only non primitive types because primitive types are referenced
		 * from the pseu_vm struct instance.
		 */
		struct type *type = table->types.data[i];
		if (type->fields_count > 0) {
			pseu_free(table->vm, type);
		}
	}

	/* Free actual lists. */
	vector_deinit(table->vm, &table->fns);
	vector_deinit(table->vm, &table->vars);
	vector_deinit(table->vm, &table->types);
}

int symbol_table_add_type(struct symbol_table *table,
				struct type *type) {
	pseu_assert(table && type);

	/* 
	 * Check if another type with this identifier has already been defined.
	 */
	if (symbol_table_get_type(table, type->ident) != -1) {
		return 1;
	}

	vector_add(table->vm, &table->types, type);
	return 0;
}

int symbol_table_add_function(struct symbol_table *table,
				struct function *fn) {
	pseu_assert(table && fn);

	/* 
	 * Check if another function with this identifier has already been
	 * defined.
	 */
	if (symbol_table_get_function(table, fn->ident) != -1) {
		return 1;
	}

	vector_add(table->vm, &table->fns, fn);
	return 0;
}

int symbol_table_add_variable(struct symbol_table *table,
				struct variable *var) {
	pseu_assert(table && var);

	/* 
	 * Check if another variable with this identifier has already been
	 * defined.
	 */
	if (symbol_table_get_variable(table, var->ident) != -1) {
		return 1;
	}

	vector_add(table->vm, &table->vars, var);
	return 0;
}

int symbol_table_get_type(struct symbol_table *table, const char *ident) {
	pseu_assert(table && ident);

	for (size_t i = 0; i < table->types.count; i++) {
		struct type *type = table->types.data[i];
		if (!strcmp(type->ident, ident)) {
			return i;
		}
	}

	return -1;
}

int symbol_table_get_function(struct symbol_table *table, const char *ident) {
	pseu_assert(table && ident);
	
	for (size_t i = 0; i < table->fns.count; i++) {
		struct function *fn = table->fns.data[i];
		if (!strcmp(fn->ident, ident)) {
			return i;
		}
	}

	return -1;
}

int symbol_table_get_variable(struct symbol_table *table, const char *ident) {
	pseu_assert(table && ident);

	for (size_t i = 0; i < table->vars.count; i++) {
		struct variable *var = table->vars.data[i];
		if (!strcmp(var->ident, ident)) {
			return i;
		}
	}

	return -1;
}
