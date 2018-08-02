#include <pseu.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vm.h"
#include "node.h"
#include "lexer.h"
#include "parser.h"
#include "visitor.h"

struct pseu {
	/* global state instance */
	struct state state;

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
	struct lexer lexer;
	struct parser parser;
	struct node *root;
	enum pseu_result result;
	instr_t *code;

	/* initial lexer & parser */
	lexer_init(&lexer, src);
	parser_init(&parser, &lexer, &pseu->state);

	/* parse the src into a syntax tree */
	parser_parse(&parser, &root);

	/* failed to parse the code */
	if (parser.errors != NULL) {
		return PSEU_ERROR_COMPILE;
	}

	/* generate the code */
	code = vm_gen(root);
	/* execute the code */
	result = vm_exec(&pseu->state, code);

	/* check if an error occured when running the code */
	if (result != 0) { 
		/* pass stuff to the callback */
		if (pseu->state.errors != NULL) {
			
		}

		return PSEU_ERROR_RUNTIME;
	}

	/* TODO: free node tree */
	return PSEU_RESULT_SUCCESS;
}
