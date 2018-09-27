#include <pseu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "state.h"
#include "string.h"
#include "symbol.h"
#include "compiler.h"

/* 
 * used to initialize a primitive type then add 
 * it to the virtual machine's global symbol
 * table
 */
static inline void add_primitive(pseu_vm_t *vm, struct type *type, const char *ident) {
	type->ident = ident;
	type->nfields = 0;

	symbol_table_add_type(vm->symbols, type);
}

/* default config, if non specified when pseu_new(NULL) */
static inline void pseu_config_init_default(pseu_config_t *config) {
	config->init_stack_size = 512;
	config->max_stack_size = 1024;
}

pseu_vm_t *pseu_vm_new(pseu_config_t *config) {
	/*
	 * allocate the necessary data on the
	 * heap
	 */
	pseu_vm_t *vm = calloc(sizeof(pseu_vm_t), 1);
	vm->strings = malloc(sizeof(struct string_table));
	vm->symbols = malloc(sizeof(struct symbol_table));

	/* 
	 * use default config if not given one;
	 * otherwise make a copy of the specified
	 * config
	 */
	if (config == NULL) {
		pseu_config_init_default(&vm->config);
	} else {
		memcpy(&vm->config, config, sizeof(pseu_config_t));
	}

	/*
	 * initialize the global state,
	 * global string_table and 
	 * global symbol_table
	 */
	state_init(&vm->state, vm);
	string_table_init(vm->strings);
	symbol_table_init(vm->symbols);

	/*
	 * initialize language primitives
	 * and register them to the global
	 * symbol table
	 */
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
	
	state_deinit(&vm->state);
	string_table_deinit(vm->strings);
	symbol_table_deinit(vm->symbols);

	free(vm->strings);
	free(vm->symbols);
	free(vm);
}

enum pseu_result pseu_interpret(pseu_vm_t *vm, const char *src) {
	if (vm == NULL || src == NULL) {
		return PSEU_RESULT_ERROR;
	}

	/*
	 * compile the src into an anonymous
	 * function
	 */
	struct func *fn = vm_compile(&vm->state, src);
	/* check if failed to compile */
	if (fn == NULL) {
		return PSEU_RESULT_ERROR;
	}

	/* call & execute the function */
	int result = vm_call(&vm->state, fn);
	/* check if call failed */
	if (result != 0) { 
		return PSEU_RESULT_ERROR;
	}

	return PSEU_RESULT_SUCCESS;
}

enum pseu_result pseu_call(pseu_vm_t *vm, const char *name) {
	if (vm == NULL || name == NULL) {
		return PSEU_RESULT_ERROR;
	}

	/*
	 * look up function in the global
	 * symbols table
	 */
	struct func *fn = symbol_table_get_func(vm->symbols, name);
	/* check if not found */
	if (fn == NULL) {
		return PSEU_RESULT_ERROR;
	}

	/* call & execute the function */
	int result = vm_call(&vm->state, fn);
	/* check if call failed */
	if (result != 0) {
		return PSEU_RESULT_ERROR;
	}

	return PSEU_RESULT_SUCCESS;
}
