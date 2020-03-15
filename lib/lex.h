#ifndef PSEU_LEX_H
#define PSEU_LEX_H

#include "obj.h"

typedef char token_t;
typedef token_t Token;

enum token {
	TK_eof,
	TK_identifier,
	TK_op_not,
	TK_op_assign,
	TK_kw_declare,
	TK_kw_output,
	TK_kw_procedure,
	TK_kw_function,
	TK_newline = 10,
	TK_lit_real,
	TK_lit_integer,
	TK_lit_string,
};

typedef struct span {
	char *pos;
	u32 len;
} Span;

typedef struct lexer {
	State *state;
	char *pos;
	char *start;
	char *end;
	char peek;
	u32 col;
	u32 row;
	u8 failed;
	Span span;
	Value value;
} Lexer;

Token pseu_lex_scan(Lexer *l);
int pseu_lex_init(State *s, Lexer *l, const char *src);

#endif /* PSEU_LEX_H */
