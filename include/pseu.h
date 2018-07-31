#ifndef PSEU_H
#define PSEU_H

#include <stdlib.h>

/*
 * represents a pseu instance
 */
typedef struct pseu pseu_t;

/*
 * represents the configuration of a
 * pseu instance
 */
typedef struct pseu_config {
	/* intial stack size */
	size_t init_stack_size;
	/* max stack size before returning an error */
	size_t max_stack_size;

	/* callback whenever pseu wants to print something to the stdout */
	void (*onprint)(pseu_t *pseu, char *str); 	
	/* callback whenever pseu has encoutered an error */
	void (*onerror)(pseu_t *pseu, int err); 
} pseu_config_t;

/*
 * represents the type error encountered
 * when interpreting some pseu code
 */
enum pseu_result {
	PSEU_RESULT_SUCCESS,
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
