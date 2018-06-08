#include <stdio.h>
#include <notpseudo.h>
#include "node.h"
#include "lexer.h"
#include "parser.h"
#include "visitor.h"
#include "vm.h"
#include "vm/ssvm.h"

struct notpseudo {
	struct state state;
	struct lexer lexer;
	struct parser parser;
	struct node *root;
	struct vm vm;
};

notpseudo_t *notpseudo_alloc(void) {
	return malloc(sizeof(struct notpseudo));
}

void notpseudo_free(notpseudo_t *notpseudo) {
	notpseudo->vm.deinit(&notpseudo->vm);
	free(notpseudo);
}

void notpseudo_init(notpseudo_t *notpseudo, char *path, char *src) {
	state_init(&notpseudo->state);
	lexer_init(&notpseudo->lexer, path, src);
	parser_init(&notpseudo->parser, &notpseudo->state, &notpseudo->lexer);
	vm_ssvm_init(&notpseudo->vm);

	/* parse the src right away */
	parser_parse(&notpseudo->parser, &notpseudo->root);
}

void notpseudo_eval(notpseudo_t *notpseudo) {
	/* todo: implement */
}
