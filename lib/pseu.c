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

pseu_t *pseu_alloc(void) {
	return malloc(sizeof(struct pseu));
}

void pseu_free(pseu_t *pseu) {
	free(pseu);
}

void pseu_init(pseu_t *pseu, char *path, char *src) {
	state_init(&pseu->state);
	lexer_init(&pseu->lexer, path, src);
	parser_init(&pseu->parser, &pseu->state, &pseu->lexer);
	vm_init(&pseu->vm, &pseu->state);

	/* parse the src right away */
	parser_parse(&pseu->parser, &pseu->root);
}

void notpseudo_eval(pseu_t *pseu) {
	/* todo: implement */
}
