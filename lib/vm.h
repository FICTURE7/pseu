#ifndef VM_H
#define VM_H

#include <pseu.h>
#include "node.h"
#include "state.h"
#include "value.h"

struct vm {
	/*
	 * NOTE:
	 *
	 * pseu does not provide any synchronizing
	 * mechanisms for the access of string_table
	 * and symbol_table by different threads
	 */

	/* global/main machine state */
	struct state state;	
	
	/* string table for interning strings */
	struct string_table *strings; 
	/* symbols in the state */
	struct symbol_table *symbols; 

	/* primitive built-in types */
	struct type void_type;
	struct type boolean_type;
	struct type integer_type;
	struct type real_type;
	struct type string_type;
	struct type array_type;

	/* configuration of the pseu instance */
	pseu_config_t config;
};

struct func *vm_compile(struct state *state, const char *src);
int vm_call(struct state *state, struct func *fn);

void *vm_alloc(struct vm *vm, size_t size);
void *vm_realloc(struct vm *vm, void *ptr, size_t size);
void vm_free(struct vm *vm, void *ptr);

#endif /* VM_H */
