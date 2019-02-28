#ifndef PSEU_LEXER_H
#define PSEU_LEXER_H

#include "pseu_token.h"
#include "pseu_location.h"

/*
 * Represents a pseu lexer.
 */
struct lexer {
	/* Pointer to buffer containing source to lex. */
	const char *src;
	/* Pointer to end of `src`. */
	const char *end; 
	/* Current position of the lexer in the source. */
	struct location loc;
};

/*
 * Initializes the specified lexer with the specified source code to lex.
 *
 * @param lexer Lexer to initialize.
 * @param src Source code to lex.
 */
void lexer_init(struct lexer *lexer, const char *src);

/*
 * Lexes the next token in the source of the specified lexer.
 *
 * @param lexer Lexer to use.
 * @param token Token to set.
 */
void lexer_lex(struct lexer *lexer, struct token *token);

#endif /* PSEU_LEXER_H */
