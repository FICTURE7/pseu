#include <pseu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "node.h"
#include "value.h"
#include "lexer.h"
#include "parser.h"
#include "visitor.h"
#include "compiler.h"

struct pseu {
	/* global state instance */
	struct state state;	

	/* configuration of the pseu instance */
	pseu_config_t config;
};

/* default config, if non specified when pseu_new(NULL) */
static inline void pseu_config_init(pseu_config_t *config) {
	config->init_stack_size = 512;
	config->max_stack_size = 1024;
}

pseu_t *pseu_new(pseu_config_t *config) {
	pseu_t *pseu = calloc(sizeof(pseu_t), 1);	
	/* check if stuff was allocated */
	if (pseu == NULL) {
		return NULL;
	}

	/* 
	 * use default config if not give one; otherwise
	 * make a copy of the specified config
	 */
	if (config == NULL) {
		pseu_config_init(&pseu->config);
	} else {
		memcpy(&pseu->config, config, sizeof(pseu_config_t));
	}

	/* initialize the global state */
	state_init(&pseu->state, &pseu->config);

	/* set the primitive types */
	pseu->state.void_type = (struct type *)&void_type;
	pseu->state.string_type = (struct type *)&string_type;
	pseu->state.array_type = (struct type *)&array_type;

	return pseu;
}

void pseu_free(pseu_t *pseu) {
	/* make sure we're not freeing nothing */
	if (pseu == NULL) {
		return;
	}
	
	/* TODO: free other resources */
	state_deinit(&pseu->state);
	free(pseu);
}

enum pseu_result pseu_interpret(pseu_t *pseu, const char *src) {
	/* exit early if arguments null */
	if (pseu == NULL || src == NULL) {
		return PSEU_RESULT_ERROR;
	}

	/* compiler which will compile the source */
	struct compiler compiler;
	/* result of the execution of the compiled source */
	enum vm_result result;
	/* function we've compiled */
	struct func *fn;

	/* initialize the compiler */
	compiler_init(&compiler, &pseu->state);

	/* compile the source to vm bytecode (instr_t) */
	fn = compiler_compile(&compiler, src);
	/* check if compilation process failed */
	if (fn == NULL) {
		return PSEU_RESULT_ERROR;
	}

	/* execute the code */
	result = vm_execute(&pseu->state, fn);
	/* check if an error occured when running the code */
	if (result != VM_RESULT_SUCCESS) { 
		return PSEU_RESULT_ERROR;
	}

	/* TODO: free node tree */
	return PSEU_RESULT_SUCCESS;
}
