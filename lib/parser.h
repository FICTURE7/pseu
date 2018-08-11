#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include "lexer.h"
#include "state.h"
#include "parser.h"
#include "diagnostic.h"

/* represents a parser instance */
struct parser {
	/* state which owns this parser instance */
	struct state *state;
	/* lexer which will be used to lex the source */
    struct lexer *lexer;
	/* current token we're processing */
	struct token token;
};

void parser_init(struct parser *parser, struct lexer *lexer, struct state *state);
struct node *parser_parse(struct parser *parser);

#endif /* PARSER_H */
