#ifndef PARSER_H
#define PARSER_H

#include "node.h"
#include "lexer.h"
#include "diagnostic.h"
#include "parser.h"

struct parser {
	struct token token;
    struct lexer *lexer;
	struct vector diagnostics;
};

void parser_init(struct parser *parser, struct lexer *lexer);

void parser_deinit(struct parser *parser);

void parser_parse(struct parser *parser, struct node **root);

#endif
