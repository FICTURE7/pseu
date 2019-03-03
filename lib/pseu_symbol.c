#include <assert.h>
#include <string.h>

#include <pseu.h>

#include "pseu_vm.h"
#include "pseu_value.h"
#include "pseu_debug.h"
#include "pseu_symbol.h"
#include "pseu_vector.h"

void symbol_table_init(pseu_vm_t *vm, struct symbol_table *table) {
	pseu_assert(vm && table);
	
	table->vm = vm;
	vector_init(vm, &table->fns, 16);
	vector_init(vm, &table->types, 16);
}

void symbol_table_deinit(struct symbol_table *table) {
	pseu_assert(table);

	vector_deinit_items(table->vm, &table->fns);
	vector_deinit(table->vm, &table->fns);

	for (size_t i = 0; i < table->types.count; i++) {
		/* 
		 * Free only non primitive types because primitive types are referenced
		 * from the pseu_vm struct. 
		 */
		struct type *type = table->types.data[i];
		if (type->fields_count > 0) {
			pseu_free(table->vm, type);
		}
	}
	vector_deinit(table->vm, &table->types);
}

void symbol_table_add_type(struct symbol_table *table,
				struct type *type) {
	pseu_assert(table && type);
	vector_add(table->vm, &table->types, type);
}

void symbol_table_add_function(struct symbol_table *table,
				struct function *fn) {
	pseu_assert(table && fn);
	vector_add(table->vm, &table->fns, fn);
}

int symbol_table_get_type(struct symbol_table *table,
				const char *ident) {
	pseu_assert(table && ident);

	for (size_t i = 0; i < table->types.count; i++) {
		struct type *type = table->types.data[i];
		if (!strcmp(type->ident, ident)) {
			return i;
		}
	}

	return -1;
}

int symbol_table_get_function(struct symbol_table *table,
				const char *ident) {
	pseu_assert(table && ident);
	
	for (size_t i = 0; i < table->fns.count; i++) {
		struct function *fn = table->fns.data[i];
		if (!strcmp(fn->ident, ident)) {
			return i;
		}
	}

	return -1;
}
