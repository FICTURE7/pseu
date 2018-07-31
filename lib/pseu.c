#include <stdio.h>
#include <pseu.h>
#include "node.h"
#include "lexer.h"
#include "parser.h"
#include "visitor.h"
#include "vm.h"

struct pseu {
	struct state state;
	struct vm vm;
};

pseu_t *pseu_new(pseu_config_t *config) {
	struct pseu *pseu = malloc(sizeof(struct pseu));
	state_init(&pseu->state);
	vm_init(&pseu->vm, &pseu->state);

	return pseu;
}

void pseu_free(pseu_t *pseu) {
	free(pseu);
}

enum pseu_result pseu_interpret(pseu_t *pseu, char *src) {
	struct lexer lexer;
	struct parser parser;
	struct node *root;

	lexer_init(&lexer, NULL, src);
	parser_init(&parser, &pseu->state, &lexer);

	parser_parse(&parser, &root);

	/* TODO: free node tree */
	/* TODO: codegen and all the good stuff */
	return PSEU_RESULT_SUCCESS;
}
