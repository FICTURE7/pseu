#include <pseu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "pseu_vm.h"
#include "pseu_core.h"
#include "pseu_state.h"
#include "pseu_string.h"
#include "pseu_symbol.h"
#include "pseu_compiler.h"

/* Default print function of the pseu virtual machine. */
static void default_print(pseu_vm_t *vm, const char *text) {
	printf("%s", text);
}

/* Default alloc function of the pseu virtual machine. */
static void *default_alloc(pseu_vm_t *vm, size_t size) {
	return malloc(size);
}

/* Default realloc function of the pseu virtual machine. */
static void *default_realloc(pseu_vm_t *vm, void *ptr, size_t size) {
	return realloc(ptr, size);
}

/* Default free function of the pseu virtual machine. */
static void default_free(pseu_vm_t *vm, void *ptr) {
	free(ptr);
}

/* Default onerror handler. */
static void default_onerror(pseu_vm_t *vm, enum pseu_error_type type,
				unsigned int row, unsigned int col, const char *message) {
	fprintf(stderr, "at %u:%u: error: %s.", row, col, message);
}

/* 
 * Initializes the default configuration, used when pseu_vm_new(NULL) is called.
 */
static void config_init_default(pseu_config_t *config) {
	config->print = default_print;
	config->alloc = default_alloc;
	config->realloc = default_realloc;
	config->free = default_free;
	config->onerror = default_onerror;

	/* Swallow warnings. Might change in the future. */
	config->onwarn = NULL;
}

pseu_vm_t *pseu_vm_new(pseu_config_t *config) {
	pseu_vm_t *vm;

	/* 
	 * If configuration was specified use its alloc function to allocate memory;
	 * otherwise use stdlib's malloc.
	 */
	vm = config ? config->alloc(NULL, sizeof(pseu_vm_t)) :
			malloc(sizeof(pseu_vm_t));

	/* If allocation failed, exit early; return NULL. */
	if (!vm) {
		return NULL;
	}
	vm->data = NULL;

	/* 
	 * If configuration specified, use that; otherwise use default 
	 * configuration. 
	 */
	config ? memcpy(&vm->config, config, sizeof(pseu_config_t)) :
			config_init_default(&vm->config);

	/* Initialize global state, global string table and global symbols table. */
	state_init(vm, &vm->state);
	string_table_init(vm, &vm->strings);
	symbol_table_init(vm, &vm->symbols);

	/* Initialize primitives types and functions. */
	core_primitives_init(vm);
	return vm;
}

void pseu_vm_free(pseu_vm_t *vm) {
	if (!vm) {
		return;
	}
	
	/* Clean up global state, global string table and global symbol table. */
	state_deinit(&vm->state);
	string_table_deinit(&vm->strings);
	symbol_table_deinit(&vm->symbols);

	/* 
	 * Finally free the memory block containing the virtual machine structure
	 * itself. 
	 */
	pseu_free(vm, vm);
}

void pseu_vm_set_data(pseu_vm_t *vm, void *data) {
	vm->data = data;
}

void *pseu_vm_get_data(pseu_vm_t *vm) {
	return vm->data;
}

int pseu_interpret(pseu_vm_t *vm, const char *src) {
	assert(vm && src);

	/* TODO: Report about runtime and compile error. */
	/* Compile source to a function executable by the virtual machine. */
	struct function fn;
	struct compiler compiler;
	if (compiler_compile(&vm->state, &compiler, &fn, src)) {
		return PSEU_RESULT_ERROR;
	}

	/* Call and execute the function. */
	if (vm_call(&vm->state, &fn)) {
		return PSEU_RESULT_ERROR;
	}

	return PSEU_RESULT_SUCCESS;
}
