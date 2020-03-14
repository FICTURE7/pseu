#include <stdarg.h>
#include <string.h>

#include "vm.h"
#include "lex.h"

#define char_isalpha(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))
#define char_isdigit(x) ((x >= '0' && x <= '9'))

static void lex_err(struct lexer *l, const char *message, ...)
{
	l->failed = 1;
	/* Proper error reporting. */
	pseu_print(l->state, message);
}

/* Moves l->pos to the next character, setting l->peek. */
static void lex_eat(struct lexer *l)
{
	if (l->pos + 1 >= l->end) {
		l->peek = TK_eof;
	} else {
		l->peek = *(++l->pos);
		l->col++;
	}
}

/* Moves l->col, l->row to a new line. */
static void lex_newline(struct lexer *l)
{
	l->col = 1; l->row++;
}

/* Skips over a block comment. */
static void lex_skip_block_comment(struct lexer *l)
{
	do {
		lex_eat(l);
		if (l->peek == '*') {
			lex_eat(l);
			if (l->peek == '/') {
				lex_eat(l);
				break;
			}
		}
	} while (l->peek != TK_eof);
}

/* Skips over a line comment. */
static void lex_skip_line_comment(struct lexer *l)
{
	do {
		lex_eat(l);
	}  while (l->peek != TK_eof && l->peek != '\n');
}

/* Lexes an identifier or a reserved keyword. */
static token_t lex_ident(struct lexer *l)
{
	token_t result;
	char *start = l->pos;
	size_t len = 0;

	do {
		len++;
		lex_eat(l);
	} while (l->peek != TK_eof && 
			(char_isalpha(l->peek) || 
			 char_isdigit(l->peek) ||
			 l->peek == '_'));

	if (strncmp("PROCEDURE", start, len) == 0) {
		result = TK_kw_procedure;
	} else if (strncmp("FUNCTION", start, len) == 0) {
		result = TK_kw_function;
	} else if (strncmp("OUTPUT", start, len) == 0) {
		result = TK_kw_output;
	} else if (strncmp("DECLARE", start, len) == 0) {
		result = TK_kw_declare;
  /*
	} else if (strncmp("TRUE", start, len) == 0) {
		result = TK_lit_true;
	} else if (strncmp("FALSE", start, len) == 0) {
		result = TK_lit_false;
  */
	} else {
		l->span.pos = start;
		l->span.len = len;
		result = TK_identifier;
	}

	return result;
}

static void lex_string(struct lexer *l)
{
	do {
		lex_eat(l);
		/* TODO: Implement. */
	} while (l->peek != TK_eof);

	l->value.type = VAL_OBJ;
	l->value.as.object = NULL;
}

static token_t lex_number(struct lexer *l)
{
	token_t result = TK_lit_integer;
	char *start = l->pos;
	char *end = start;

	do {
		end++;
		lex_eat(l);
	} while (l->peek != TK_eof && char_isdigit(l->peek));

	if (result == TK_lit_integer) {
		l->value.type = VAL_INT;
		l->value.as.integer = (uint32_t)strtol(start, &end, 10);

		/* TODO: Check if overflow and stuff. */
	} else if (result == TK_lit_real) {
		l->value.type = VAL_FLOAT;
		l->value.as.real = 0;

		/* TODO: Implement. */
	}

	return result;
}

int pseu_lex_init(pseu_state_t *s, struct lexer *l, const char *src)
{
	l->state = s;
	l->start = src;
	l->end = src + strlen(src);
	l->value = (struct value) { 0 };
	l->pos = (char *)l->start;
	l->row = 1;
	l->col = 1;
	l->failed = 0;
	l->peek = l->pos < l->end ? *l->pos : TK_eof;
	return 0;
}

token_t pseu_lex_scan(struct lexer *l)
{
	for (;;) {
		char c = l->peek;

		if (char_isdigit(c))
			return lex_number(l);
		else if (char_isalpha(c))
			return lex_ident(l);

		switch (c) {
		case TK_eof:
			return TK_eof;

		case '\n':
			lex_newline(l);
			lex_eat(l);
			return '\n';

		case ' ':
		case '\t':
		case '\r':
			lex_eat(l);
			continue;

		case '(':
		case ')':
		case '+':
		case '-':
		case '*':
		case '=':
		case '>':
		case ':':
			lex_eat(l);
			return c;

		case '_':
			return lex_ident(l);
		case '"':
			return lex_string(l), TK_lit_string;
		case '/':
			lex_eat(l);
			if (l->peek == '/') { 
				lex_skip_line_comment(l);
				continue; 
			} else if (l->peek == '*') {
				lex_skip_block_comment(l);
				continue;
			}
			return '/';
		case '<':
			lex_eat(l);
			if (l->peek == '>')
				return lex_eat(l), TK_op_not;
			else if (l->peek == '-')
				return lex_eat(l), TK_op_assign;
			return '<';
		case '.':
			lex_eat(l);
			if (char_isdigit(l->peek))
				return lex_number(l);
			return '.';

		default:
			lex_err(l, "Unexpected character '%c'", c);
			lex_eat(l);
			return '\0';
		}
	}
}
