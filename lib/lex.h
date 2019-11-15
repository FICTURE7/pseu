#ifndef PSEU_LEX_H
#define PSEU_LEX_H

#include "obj.h"

typedef char token_t;

enum token {
	TK_eof,
	TK_identifier,
	TK_op_not,
	TK_op_assign,
	TK_kw_declare,
	TK_kw_output,
	TK_kw_procedure,
	TK_kw_function,
	TK_lit_true,
	TK_lit_false,
	TK_newline,
	TK_lit_real,
	TK_lit_integer,
	TK_lit_string,
};

struct span {
	char *pos;
	uint32_t len;
};

struct lexer {
	pseu_state_t *state;
	struct span span;
	char *pos;
	char *start;
	char *end;
	char peek;
	uint32_t col;
	uint32_t row;
	uint8_t failed;
	struct value value;
};

token_t pseu_lex_can(struct lexer *l);

#endif /* PSEU_LEX_H */
