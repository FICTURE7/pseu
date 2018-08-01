#ifndef PSEU_H
#define PSEU_H

#include <stdlib.h>

/*
 * represents a pseu instance
 */
typedef struct pseu pseu_t;

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
}

/*
 * represents the configuration of a
 * pseu instance
 */
typedef struct pseu_config {
	/* max stack size can grow before returning an error */
	size_t max_stack_size;
	/* intial stack size */
	size_t init_stack_size;

	/* callback whenever pseu wants to print something to the stdout */
	void (*onprint)(pseu_t *pseu, char *text); 	
	/* callback whenever pseu has encoutered an error */
	void (*onerror)(pseu_t *pseu, enum pseu_error_type type, int row, int col, const char *message); 
	/* callback whenever pseu has encoutered a warning  */
	void (*onwarn)(pseu_t *pseu, enum pseu_warn_type type, int row, int col, const char *message); 
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
 * allocates a new pseu instance with the
 * specified configuration
 */
pseu_t *pseu_new(pseu_config_t *config);

/*
 * frees the specified pseu instance along
 * with its resources
 */
void pseu_free(pseu_t *pseu);

/*
 * interprets the specified source using
 * the specified pseu instance
 */
enum pseu_result pseu_interpret(pseu_t *pseu, char *src);

#endif /* PSEU_H */
