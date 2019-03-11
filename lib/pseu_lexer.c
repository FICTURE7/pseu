#include <ctype.h>
#include <stdbool.h>
#include <string.h>

#include "pseu_lexer.h"

/* advances the lexer's position by the specified amount of character */
static char *advance_by(struct lexer *lexer, int count) {
	lexer->loc.pos += count;
	lexer->loc.col += count;
	return lexer->loc.pos;
}

/* advances the lexer's position by one character */
static char *advance(struct lexer *lexer) {
	return advance_by(lexer, 1);
}

/*
 * identifier = (ALPHA / "_") *(ALPHA / DIGIT / "_")
 */
static void scan_identifier(struct lexer *lexer, struct token *token) {
	/*
	 * no need to check if first character is an alphabet or "_",
	 * assume caller already did
	 */
	char c = *lexer->loc.pos;
	token->loc = lexer->loc;
	token->len = 1;

	while (advance(lexer) < lexer->end) {
		c = *lexer->loc.pos;
		if (!isalnum(c) && c != '_') {
			break;
		}
		token->len++;
	}

	/* todo: turn in a hashmap or something */
	/* check if the identifier is a keyword */
	if (token_value_equals(token, "OUTPUT")) {
		token->type = TOK_KW_OUTPUT;
	} else if (token_value_equals(token, "DECLARE")) {
		token->type = TOK_KW_DECLARE;
	} else if (token_value_equals(token, "FUNCTION")) {
		token->type = TOK_KW_FUNCTION;
	} else if (token_value_equals(token, "ENDFUNCTION")) {
		token->type = TOK_KW_ENDFUNCTION;
	} else if (token_value_equals(token, "RETURN")) {
		token->type = TOK_KW_RETURN;
	} else if (token_value_equals(token, "TRUE")) {
		token->type = TOK_LIT_BOOLEAN_TRUE;
	} else if (token_value_equals(token, "FALSE")) {
		token->type = TOK_LIT_BOOLEAN_FALSE;
	} else if (token_value_equals(token, "NOT")) {
		token->type = TOK_OP_LOGICAL_NOT;
	} else if (token_value_equals(token, "AND")) {
		token->type = TOK_OP_LOGICAL_AND;
	} else if (token_value_equals(token, "OR")) {
		token->type = TOK_OP_LOGICAL_OR;
	} else {
		/* must be an identifier */
		token->type = TOK_IDENT;
	}
}

/*
 * integer-hex		= ("0x" / "0X") 1*HEXDIG
 * integer			= 1*DIGIT / integer-hex
 * real-exponent	= ("e" / "E") 1*DIGIT
 * real				= (1*DIGIT "." *DIGIT / "." 1*DIGIT) *1real-exponent / 1*DIGIT real-exponent
 * number			= integer / real
 */
static void scan_number(struct lexer *lexer, struct token *token) {
	char c = *lexer->loc.pos;
	token->loc = lexer->loc;
	token->len = 1;

	/*
	 * check for
	 * hex number starting with '0x' or '0X'
	 * OR
	 * floating point starting with '0e' or '0E' (because why not)
	 * OR
	 * floating point starting with '0.'
	 * OR
	 * number starting with a '0'
	 */
	if (c == '0') {
		/* check if we've reached eof */
		if (advance(lexer) >= lexer->end) {
			token->type = TOK_LIT_INTEGER;
			return;
		}

		c = *lexer->loc.pos;
		switch (c) {
			/* hex number starting with '0x' or '0X' */
			case 'x':
			case 'X':
				token->len++;
				/* try to peek the next character after the 'x' */
				if (advance(lexer) >= lexer->end) {
					token->type = TOK_ERR_INVALID_HEX;
					return;
				}

				/* scan rest of hex value */
				c = *lexer->loc.pos;
				while (isdigit(c) || (c >= 'a' && c <= 'f')
								|| (c >= 'A' && c <= 'F')) {
					token->len++;
					/* reached eof */
					if (advance(lexer) >= lexer->end) {
						break;
					}
					c = *lexer->loc.pos;
				}
				/* todo: add support for the error message */
				token->type = token->len > 2 ?
						TOK_LIT_INTEGERHEX : TOK_ERR_INVALID_HEX;
				return;

			/* floating point starting with '0e' or '0E' */
			case 'e':
			case 'E':
				goto exponent;

			/* floating point starting with '0.' */
			case '.':
				token->len++;
				goto fraction;

			/* either a '0' or a number starting with '0' */
			default:
				while (isdigit(c)) {
					token->len++;
					/* reached eof */
					if (advance(lexer) >= lexer->end) {
						break;
					}
					c = *lexer->loc.pos;
					if (c == 'e' || c == 'E') {
						goto exponent;
					}
				}
				token->type = TOK_LIT_INTEGER;
				return;
		}
	}

	/* scan fraction part of floating points */
	if (c == '.') {
fraction:
		c = *(advance(lexer));
		/*
		 * for floating point with an exponent after the '.'
		 * e.g: 1.e5, 556.e5
		 */
		if (c == 'e' || c == 'E') {
			goto exponent;
		}

		while (isdigit(c)) {
			token->len++;
			/* reached eof */
			if (advance(lexer) >= lexer->end) {
				break;
			}

			c = *lexer->loc.pos;
			if (c == 'e' || c == 'E') {
				goto exponent;
			}
		}
		token->type = TOK_LIT_REAL;
		return;

exponent:
		token->len++;
		c = *(advance(lexer));

		/* exponent sign */
		if (c == '+' || c == '-') {
			token->len++;
			c = *(advance(lexer));
		}

		if (isdigit(c)) {
			do {
				token->len++;
				/* reached eof */
				if (advance(lexer) >= lexer->end) {
					break;
				}
				c = *lexer->loc.pos;
			} while (isdigit(c));
			token->type = TOK_LIT_REAL;
		} else {
			token->type = TOK_ERR_INVALID_EXP;
		}
		return;
	}

	/* scan integers */
	while (advance(lexer) < lexer->end) {
		c = *lexer->loc.pos;

		/* check for exponents and fractions */
		switch (c) {
			case 'e':
			case 'E':
				goto exponent;

			case '.':
				token->len++;
				goto fraction;
		}

		if (!isdigit(c)) {
			break;
		}

		token->len++;
	}
	token->type = TOK_LIT_INTEGER;
	return;
}

static void scan_string(struct lexer *lexer, struct token *token) {
	/* skip the openning '"' character */
	char c = *(advance(lexer));
	token->loc = lexer->loc;
	token->len = 0;

	do {
again:
		token->len++;
		/* reached eof without the closing '"' character */
		if (advance(lexer) >= lexer->end) {
			token->type = TOK_ERR_INVALID_STRING;
			return;
		}
		c = *lexer->loc.pos;
		switch (c) {
			case '\n':
			case '\r':
				/* prevent newlines inside of strings */
				token->type = TOK_ERR_INVALID_STRING;
				return;
			case '\\':
				/* try to peak the next character */
				if (lexer->loc.pos + 1 >= lexer->end) {
					continue;
				}
				c = *(lexer->loc.pos + 1);
				/* skip current '"' if it was preceeded by a '\' */
				if (c == '"') {
					token->len++;
					advance(lexer);
					goto again;
				}
				break;
		}
	} while (c != '"');

	token->type = TOK_LIT_STRING;
	/* skip the closing '"' character */
	advance(lexer);
}

void lexer_init(struct lexer *lexer, const char *src) {
	lexer->src = src;
	lexer->end = src + strlen(src);
	lexer->loc.pos = (char *)src;
	lexer->loc.ln = 1;
	lexer->loc.col = 1;
}

void lexer_next(struct lexer *lexer, struct token *token) {
	/* peek character */
	char p;
	/* current character */
	char c;

again:
	/* set tok to TOK_EOF if we reached eof */
	if (lexer->loc.pos >= lexer->end) {
		goto eof_exit;
	}

	c = *lexer->loc.pos;
	/* skip spaces except '\n' */
	while (isspace(c) && c != '\n') {
		c = *(advance(lexer));
		/* reached eof */
		if (lexer->loc.pos >= lexer->end) {
			goto eof_exit;
		}
	}

	switch (c) {
		case '"':
			/* scan string literals */
			scan_string(lexer, token);
			return;

		case '\n':
			token_init(token, TOK_LF, lexer->loc, 1);

			/*
			 * we moved to the next line so
			 * update loc.ln and loc.col to point
			 * to new begining of line
			 */
			lexer->loc.ln++;
			lexer->loc.col = 1;
			lexer->loc.pos++;
			return;

		case '.':
			/* if no character after then it must be a dot */
			if (lexer->loc.pos + 1 >= lexer->end) {
				token_init(token, TOK_DOT, lexer->loc, 1);
				goto advance_exit;
			}

			/* if next character is a digit it must be a number (floating point) */
			p = *(lexer->loc.pos + 1);
			if (isdigit(p)) {
				scan_number(lexer, token);
				return;
			}

			token_init(token, TOK_DOT, lexer->loc, 1);
			goto advance_exit;
		case ':':
			token_init(token, TOK_COLON, lexer->loc, 1);
			goto advance_exit;
		case ',':
			token_init(token, TOK_COMMA, lexer->loc, 1);
			goto advance_exit;
		case '_':
			/* scan identifiers starting with '_' */
			scan_identifier(lexer, token);
			return;
		case '=':
			token_init(token, TOK_EQUAL, lexer->loc, 1);
			goto advance_exit;
		case '<':
			/* if no character after then it must be a less */
			if (lexer->loc.pos + 1 >= lexer->end) {
				token_init(token, TOK_LESS, lexer->loc, 1);
				goto advance_exit;
			}
			
			p = *(lexer->loc.pos + 1);
			if (p == '-') {
				token_init(token, TOK_ASSIGN, lexer->loc, 2);
				advance(lexer);
				goto advance_exit;
			}

		case '(':
			token_init(token, TOK_LPAREN, lexer->loc, 1);
			goto advance_exit;
		case ')':
			token_init(token, TOK_RPAREN, lexer->loc, 1);
			goto advance_exit;
		case '+':
			token_init(token, TOK_OP_ADD, lexer->loc, 1);
			goto advance_exit;
		case '-':
			token_init(token, TOK_OP_SUB, lexer->loc, 1);
			goto advance_exit;
		case '/':
			/* try to peek the next character */
			if (lexer->loc.pos + 1 >= lexer->end) {
				token_init(token, TOK_OP_DIV, lexer->loc, 1);
				goto advance_exit;
			}
			p = *(lexer->loc.pos + 1);
			/*
			 * if the next character is also a '/' then its a single-line comment
			 * or the next character is a '*' then its a multi-line comment
			 * otherwise its a div token
			 */
			switch (p) {
				case '/':
					advance(lexer);
					/* scan the until the next linefeed */
					do {
						if (advance(lexer) >= lexer->end) {
							goto eof_exit;
						}
						c = *lexer->loc.pos;
					} while (c != '\n');
					/*
					 * skip the current '\n' character
					 * move the loc of the lexer to next line
					 * and start lexing again
					 */
					lexer->loc.ln++;
					lexer->loc.col = 1;
					lexer->loc.pos++;
					goto again;
				case '*':
					advance(lexer);
					/* scan the until the next '*' '/' sequence */
					while (advance(lexer) < lexer->end) {
						c = *lexer->loc.pos;
						if (c == '*') {
							/* try to peek the next character*/
							if (lexer->loc.pos + 1 >= lexer->end) {
								goto eof_exit;
							}
							p = *(advance(lexer));
							/*
							 * current char is a '*' next character is a '/'
							 * which indicates the end of the multi-line comment
							 */
							if (p == '/') {
								advance(lexer);
								goto again;
							}
						}
					};
					goto eof_exit;

				default:
					token_init(token, TOK_OP_DIV, lexer->loc, 1);
					goto advance_exit;
			}

		case '*':
			token_init(token, TOK_OP_MUL, lexer->loc, 1);
			goto advance_exit;

		default:
			/* scan identifiers or keywords */
			if (isalpha(c)) {
				scan_identifier(lexer, token);
				return;
			}

			/* scan number literals */
			if (isdigit(c)) {
				scan_number(lexer, token);
				return;
			}

			/* unknown character */
			token_init(token, TOK_ERR_UNKNOWN_CHAR, lexer->loc, 1);
			goto advance_exit;
	}

advance_exit:
	advance(lexer);
	return;

eof_exit:
	token_init(token, TOK_EOF, lexer->loc, 0);
	return;
}

void lexer_peek(struct lexer *lexer, struct token *token) {
	/* Store the location of the lexer before peeking. */
	struct location loc_original = lexer->loc;
	lexer_next(lexer, token);
	lexer->loc = loc_original;
}
