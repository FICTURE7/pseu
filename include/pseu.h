#ifndef PSEU_H
#define PSEU_H

#include <stdlib.h>

/*
 * represents a pseu instance
 */
typedef struct vm pseu_vm_t;

/*
 * represents the type of errors
 */
enum pseu_error_type {
	/* run time error */
	PSEU_ERROR_RUNTIME,
	/* compile time error */
	PSEU_ERROR_COMPILE
};

/*
 * represents the type of warnings
 */
enum pseu_warn_type {
	/* run time warning */
	PSEU_WARN_RUNTIME,
	/* compile time warning */
	PSEU_WARN_COMPILE
};

/*
 * represents the configuration of a
 * virtual machine
 */
typedef struct pseu_config {
	/* max stack size can grow before returning an error */
	size_t max_stack_size;
	/* intial stack size */
	size_t init_stack_size;

	/* callback whenever pseu wants to print something to the stdout */
	void (*onprint)(pseu_vm_t *pseu, char *text); 	
	/* callback whenever pseu has encoutered an error */
	void (*onerror)(pseu_vm_t *pseu, enum pseu_error_type type, int row, int col, const char *message); 
	/* callback whenever pseu has encoutered a warning  */
	void (*onwarn)(pseu_vm_t *pseu, enum pseu_warn_type type, int row, int col, const char *message); 

	/* allocates a block of memory */
	void *(*alloc)(size_t size);
	/* reallocates a block of memory */
	void *(*realloc)(void *ptr, size_t size);
	/* frees a block of memory */
	void (*free)(void *ptr);
} pseu_config_t;

/*
 * represents the type of results
 * when interpreting some pseu code
 */
enum pseu_result {
	/* successfully interpreted code */
	PSEU_RESULT_SUCCESS,
	/* error when interpreting code */
	PSEU_RESULT_ERROR
};

/*
 * creates a new virtual machine with the specified
 * `config`.
 *
 * returns a pointer to a heap allocated pseu_vm_t
 */
pseu_vm_t *pseu_vm_new(pseu_config_t *config);

/*
 * frees the specified pseu instance along
 * with its resources
 */
void pseu_free(pseu_vm_t *pseu);

/*
 * interprets the specified source using
 * the specified pseu instance
 */
enum pseu_result pseu_interpret(pseu_vm_t *pseu, const char *src);

#endif /* PSEU_H */
