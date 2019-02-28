#ifndef PSEU_TOKEN_H
#define PSEU_TOKEN_H

#include <stdlib.h>
#include "pseu_location.h"

enum token_type {
	/* Errors. */
	TOK_ERR_INVALID_HEX,
	TOK_ERR_INVALID_EXP,
	TOK_ERR_INVALID_STRING,
	TOK_ERR_UNKNOWN_CHAR,

	TOK_EOF,

	/* Literals. */
	TOK_LIT_INTEGER,
	TOK_LIT_INTEGERHEX,
	TOK_LIT_REAL,
	TOK_LIT_STRING,
	TOK_LIT_BOOLEAN_TRUE,
	TOK_LIT_BOOLEAN_FALSE,

	TOK_LF,
	TOK_DOT,
	TOK_COLON,
	TOK_EQUAL,
	TOK_LESS,
	TOK_ASSIGN,
	TOK_IDENT,

	TOK_LPAREN,
	TOK_RPAREN,
	/* Operators. */
	TOK_OP_ADD,
	TOK_OP_SUB,
	TOK_OP_MUL,
	TOK_OP_DIV,
	TOK_OP_LOGICAL_NOT,
	TOK_OP_LOGICAL_AND,
	TOK_OP_LOGICAL_OR,

	/* Keywords. */
	TOK_KW_DECLARE,
	TOK_KW_OUTPUT,
	TOK_KW_INTEGER,
	TOK_KW_REAL,
	TOK_KW_STRING,
	TOK_KW_BOOLEAN,
};

/* Represents a token. */
struct token {
	/* Token type. */
	enum token_type type;
	/* Location of the token in source. */
	struct location loc;
	/* Length of the token. */
	size_t len;
};

void token_init(struct token *token, enum token_type type, struct location loc, size_t len);
void token_value(struct token *token, char *buffer);
int token_value_cmp(struct token *token, char *buffer);

#endif /* PSEU_TOKEN_H */
