#ifndef PSEU_LEX_H
#define PSEU_LEX_H

#include "obj.h"

typedef enum TokenType {
	TK_eof,
	TK_identifier,
	TK_op_not,
	TK_op_assign,
	TK_kw_declare,
	TK_kw_output,
	TK_kw_procedure,
	TK_kw_function,
	TK_newline = 10,
  TK_kw_if,
  TK_kw_else,
  TK_kw_endif,
  TK_kw_then,
	TK_lit_real,
	TK_lit_integer,
	TK_lit_string,
} TokenType;

/* Represents a token. */
typedef char Token;

/* Represents a region of memory - usually is a region of a string. */
typedef struct Span {
	char *pos;
	u32 len;
} Span;

/* Represents a lexer. */
typedef struct Lexer {
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
