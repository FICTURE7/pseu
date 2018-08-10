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
	/* global variables */
	struct symbol_table *symbols;
	
	/* configuration of the pseu instance */
	pseu_config_t config;
};

void pseu_config_init(pseu_config_t *config) {
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
	state_init(&pseu->state);
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

enum pseu_result pseu_interpret(pseu_t *pseu, char *src) {
	struct compiler compiler;
	struct func *fn;
	enum vm_result result;

	/* exit early if arguments null */
	if (pseu == NULL || src == NULL) {
		return PSEU_RESULT_ERROR;
	}

	/* initialize the compiler */
	compiler_init(&compiler, &pseu->state);

	/* compile the source and check if failed */
	fn = compiler_compile(&compiler, src);
	if (fn == NULL) {
		return PSEU_RESULT_ERROR;
	}

	/* execute the code */
	result = vm_call(&pseu->state, fn);
	/* check if an error occured when running the code */
	if (result != VM_RESULT_SUCCESS) { 
		return PSEU_RESULT_ERROR;
	}

	/* TODO: free node tree */
	return PSEU_RESULT_SUCCESS;
}
