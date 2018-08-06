#include <stdlib.h>
#include "func.h"
#include "lexer.h"
#include "parser.h"
#include "compiler.h"

void compiler_init(struct compiler *compiler, struct state *state) {
	compiler->state = state;

	lexer_init(&compiler->lexer);
	parser_init(&compiler->parser, &compiler->lexer, state);
}

struct func *compiler_compile(struct compiler *compiler) {
	/* TODO: implement */
	return NULL;
}
