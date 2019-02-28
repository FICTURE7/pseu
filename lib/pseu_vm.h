#ifndef PSEU_VM_H
#define PSEU_VM_H

#include <pseu.h>

#include "pseu_lexer.h"
#include "pseu_state.h"
#include "pseu_value.h"
#include "pseu_string.h"
#include "pseu_symbol.h"

/*
 * Types of arithmetic and logical operations.
 */
enum op_type {
	OP_ADD = TOK_OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_LOGICAL_NOT,
	OP_LOGICAL_AND,
	OP_LOGICAL_OR
};

/* 
 * Represents a pseu virtual machine. This is the opaque type we're exposing 
 * as `pseu_vm_t`.
 *
 * NOTE:
 * Pseu does not provide any synchronizing mechanisms for the access of 
 * string_table and symbol_table by different threads.
 */
struct pseu_vm {
	/* Global state. */
	struct state state;	
	
	/* Global interned string table of the virtual machine. */
	struct string_table strings; 
	/* Global symbol table of the virtual machine. */
	struct symbol_table symbols; 

	/* Primitive built-in types. */
	struct type void_type;
	struct type boolean_type;
	struct type integer_type;
	struct type real_type;
	struct type string_type;
	struct type array_type;

	/* Configuration of the pseu virtual machine instance. */
	pseu_config_t config;
	/* User data attached to the pseu virtual machine instance. */
	void *data;
};


/*
 * Calls the specified function using the specified pseu virtual machine state.
 *
 * @param[in] state Pseu virtual machine state.
 * @param[in] fn Function to execute.
 */
int vm_call(struct state *state, struct function *fn);

/* 
 * TODO: Consider renaming vm_{alloc,realloc,free} to pseu_{alloc,realloc,free}. 
 */

/*
 * Allocates a block of memory of the specified size using the alloc function
 * of the specified virtual machine instance.
 *
 * @param[in] vm Pseu virtual machine instance.
 * @param[in] size Size of block to allocate.
 */
void *pseu_alloc(pseu_vm_t *vm, size_t size);

/*
 * Resizes the specified block of memory to the specified size using the realloc
 * function of the specified virtual machine instance.
 *
 * @param[in] vm Pseu virtual machine instance.
 * @param[in] ptr Block to resize.
 * @param[in] size Size of block to allocate.
 * @return Pointer to possible new block; returns NULL if failed.
 */
void *pseu_realloc(pseu_vm_t *vm, void *ptr, size_t size);

/*
 * Frees the specified block of memory using the free function of the specified
 * virtual machine instance.
 *
 * @param[in] vm Pseu virtual machine instance.
 * @param[in] ptr Block to free.
 */
void pseu_free(pseu_vm_t *vm, void *ptr);

/*
 * Prints the specified text using the print function of the specified virtual 
 * machine instance.
 *
 * @param[in] vm Pseu virtual machine instance.
 * @param[in] text Text to print.
 */
void pseu_print(pseu_vm_t *vm, const char *text);

#endif /* PSEU_VM_H */
