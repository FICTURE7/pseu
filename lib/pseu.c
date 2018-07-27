#include <stdio.h>
#include <pseu.h>
#include "node.h"
#include "lexer.h"
#include "parser.h"
#include "visitor.h"
#include "vm.h"

struct pseu {
	struct state state;
	struct lexer lexer;
	struct parser parser;
	struct node *root;
	struct vm vm;
};

struct pseu *pseu_new(struct pseu_config *config) {
	struct pseu *pseu = malloc(sizeof(struct pseu));
	state_init(&pseu->state);
	vm_init(&pseu->vm, &pseu->state);

	return pseu;
}

void pseu_free(struct pseu *pseu) {
	free(pseu);
}

void pseu_interpret(struct pseu *pseu, char *src) {
	lexer_init(&pseu->lexer, NULL, src);
	parser_init(&pseu->parser, &pseu->state, &pseu->lexer);

	parser_parse(&pseu->parser, &pseu->root);
	/* TODO: codegen and all the good stuff */
}
