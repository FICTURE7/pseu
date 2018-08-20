#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "location.h"

/* represents a lexer */
struct lexer {
	const char *src; /* pointer to the src/buffer */
	const char *end; /* pointer at which src ends */
	struct location loc; /* current location/position of the lexer */
};

void lexer_init(struct lexer *lexer, const char *src);
void lexer_lex(struct lexer *lexer, struct token *token);

#endif /* LEXER_H */
