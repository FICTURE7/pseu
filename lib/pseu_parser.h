#ifndef PSEU_PARSER_H
#define PSEU_PARSER_H

#include "pseu_node.h"
#include "pseu_lexer.h"
#include "pseu_state.h"
#include "pseu_token.h"
#include "pseu_parser.h"

/* 
 * Represents a pseu parser. 
 */
struct parser {
	/* State which owns this parser instance. */
	struct state *state;
	/* Lexer which will be used to lex the source. */
    struct lexer *lexer;
	/* Current token we're processing. */
	struct token token;

	/* Number of errors during parsing process. */
	unsigned int error_count;
};

/*
 * Initializes the specified parser with the specified lexer and state.
 *
 * @param parser Parser instance.
 * @param lexer Lexer instance.
 * @param state State instance.
 */
void parser_init(struct parser *parser, struct lexer *lexer, struct state *state);

/*
 * Parses the code by going through the parser's lexer.
 *
 * @param parser Parser instance.
 * @return Root AST node if success; otherwise returns NULL.
 */
struct node *parser_parse(struct parser *parser);

#endif /* PSEU_PARSER_H */
