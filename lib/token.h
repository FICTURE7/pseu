#ifndef TOKEN_H
#define TOKEN_H

#include <stdlib.h>
#include "location.h"

enum token_type {
	/* errors */
	TOK_ERR_INVALID_HEX,
	TOK_ERR_INVALID_EXP,
	TOK_ERR_INVALID_STRING,
	TOK_ERR_UNKNOWN_CHAR,

	TOK_EOF,

	/* literals */
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
	TOK_IDENT,

	TOK_LPAREN,
	TOK_RPAREN,
	/* operators */
	TOK_OP_ADD,
	TOK_OP_SUB,
	TOK_OP_MUL,
	TOK_OP_DIV,
	TOK_OP_LOGICAL_NOT,
	TOK_OP_LOGICAL_AND,
	TOK_OP_LOGICAL_OR,

	/* keywords */
	TOK_KW_DECLARE,
	TOK_KW_OUTPUT,
	TOK_KW_INTEGER,
	TOK_KW_REAL,
	TOK_KW_STRING,
	TOK_KW_BOOLEAN,
};

struct token {
	enum token_type type;
	struct location loc;
	size_t len;
};

void token_init(struct token *token, enum token_type type, struct location loc, size_t len);
void token_value(struct token *token, char *buffer);
int token_value_cmp(struct token *token, char *buffer);

#endif /* TOKEN_H */
