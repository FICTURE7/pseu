#include <pseu.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"
#include "state.h"
#include "string.h"
#include "symbol.h"
#include "compiler.h"

/* 
 * Initializes the specified struct type * with the specified type identifier
 * as a primitive and register it to the global virtual machine symbol table.
 */
static inline void add_primitive(pseu_vm_t *vm, struct type *type,
		const char *ident) {
	type->ident = ident;
	type->nfields = 0;

	symbol_table_add_type(vm->symbols, type);
}

/* Initializes the default config, used when pseu_vm_new(NULL) is called.*/
static inline void pseu_config_init_default(pseu_config_t *config) {
	config->init_stack_size = 512;
	config->max_stack_size = 1024;

	/* Use stdlib's alloc function as defaults. */
	config->alloc = malloc;
	config->realloc = realloc;
	config->free = free;
}

pseu_vm_t *pseu_vm_new(pseu_config_t *config) {
	pseu_vm_t *vm;

	/* 
	 * If configuration was specified use its alloc function to allocate memory;
	 * otherwise use stdlib's malloc.
	 */
	if (config) {
		vm = config->alloc(sizeof(pseu_vm_t));	
	} else {
		vm = malloc(sizeof(pseu_vm_t));
	}

	/* Check if allocation failed. */
	if (!vm) {
		return NULL;
	}

	/* If configuration specified, use that; otherwise use default configuration. */
	if (config) {
		memcpy(&vm->config, config, sizeof(pseu_config_t));
	} else {
		pseu_config_init_default(&vm->config);
	}

	/* 
	 * TODO: Make vm->strings & vm->symbols value types instead of pointer types 
	 * in struct vm.
	 */

	/* Allocate global string table and global symbols table. */
	vm->strings = malloc(sizeof(struct string_table));
	vm->symbols = malloc(sizeof(struct symbol_table));

	/* Initialize global state, global string table and global symbols table. */
	state_init(&vm->state, vm);
	string_table_init(vm->strings);
	symbol_table_init(vm->symbols);

	/* Initialize primitives and register them to the global symbol table. */
	add_primitive(vm, &vm->void_type, "VOID");
	add_primitive(vm, &vm->boolean_type, "BOOLEAN");
	add_primitive(vm, &vm->integer_type, "INTEGER");
	add_primitive(vm, &vm->real_type, "REAL");
	add_primitive(vm, &vm->string_type, "STRING");
	add_primitive(vm, &vm->array_type, "ARRAY");

	return vm;
}

void pseu_free(pseu_vm_t *vm) {
	if (vm == NULL) {
		return;
	}
	
	/* Deinitialize global state, global string table and global symbol table. */
	state_deinit(&vm->state);
	string_table_deinit(vm->strings);
	symbol_table_deinit(vm->symbols);

	/* Free memory blocks. */
	vm_free(vm, vm->strings);
	vm_free(vm, vm->symbols);
	vm_free(vm, vm);
}

int pseu_interpret(pseu_vm_t *vm, const char *src) {
	assert(vm && src);

	/* Compile the source into an anonymous procedure. */
	struct func *fn = vm_compile(&vm->state, src);
	if (fn == NULL) {
		return PSEU_RESULT_ERROR;
	}

	/* Call & execute the function. */
	int result = vm_call(&vm->state, fn);
	if (result != 0) { 
		return PSEU_RESULT_ERROR;
	}

	return PSEU_RESULT_SUCCESS;
}

int pseu_call(pseu_vm_t *vm, const char *name) {
	assert(vm, name);

	/* TODO: Figure out arguments. */

	/* Look up function in global symbols table. */
	struct func *fn = symbol_table_get_func(vm->symbols, name);
	if (fn == NULL) {
		return PSEU_RESULT_ERROR;
	}

	/* Call & execute the function. */
	int result = vm_call(&vm->state, fn);
	if (result != 0) {
		return PSEU_RESULT_ERROR;
	}

	return PSEU_RESULT_SUCCESS;
}
