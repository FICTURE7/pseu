#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include "lexer.h"
#include "state.h"
#include "parser.h"

struct parser {
	struct state *state;
    struct lexer *lexer;
	struct token token;
};

void parser_init(struct parser *parser, struct state *state, struct lexer *lexer);
void parser_parse(struct parser *parser, struct node **root);

#endif /* PARSER_H */
