#define LEXER_DEBUG

#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include "lexer.h"

#ifdef LEXER_DEBUG
#include "pretty.h"
#endif

static char *advance(struct lexer *lexer) {
    lexer->pos++;
    lexer->loc.col++;
    return lexer->pos;
}

static void scan_identifier(struct lexer *lexer, struct token *token) {
    char c = *lexer->pos;
    token->loc = lexer->loc;
    token->pos = lexer->pos;
    token->len = 1;

    while (advance(lexer) < lexer->end) {
        c = *lexer->pos;
        if (!isalnum(c) && c != '_') {
            break;
        }
        token->len++;
    }

    /* todo: turn in a hashmap or something */
    /* check if the identifier is a keyword */
    if (token_value_cmp(token, "OUTPUT")) {
        token->type = TOK_KW_OUTPUT;
    } else if (token_value_cmp(token, "SINGLE")) {
        token->type = TOK_KW_REAL;
    } else if (token_value_cmp(token, "REAL")) {
        token->type = TOK_KW_STRING;
    } else if (token_value_cmp(token, "DECLARE")) {
        token->type = TOK_KW_DECLARE;
    } else if (token_value_cmp(token, "BOOLEAN")) {
        token->type = TOK_KW_BOOLEAN;
    } else if (token_value_cmp(token, "TRUE")) {
        token->type = TOK_LIT_BOOLEAN_TRUE;
    } else if (token_value_cmp(token, "FALSE")) {
        token->type = TOK_LIT_BOOLEAN_FALSE;
    } else if (token_value_cmp(token, "NOT")) {
        token->type = TOK_OP_LOGICAL_NOT;
    } else if (token_value_cmp(token, "AND")) {
        token->type = TOK_OP_LOGICAL_AND;
    } else if (token_value_cmp(token, "OR")) {
        token->type = TOK_OP_LOGICAL_OR;
    }
}

static void scna_number(struct lexer *lexer, struct token *token) {
    char c = *lexer->pos;
    token->loc = lexer->loc;
    token->pos = lexer->pos;
    token->len = 1;

    /*
     *	check for
     *	hex number starting with '0x' or '0X'
     *	OR
     *	floating point starting with '0e' or '0E' (because why not)
     *	OR
     *  floating point starting with '0.'
     *	OR
     *	number starting with a '0'
     */
    if (c == '0') {
        /* check if we've reached eof */
        if (advance(lexer) >= lexer->end) {
            token->type = TOK_LIT_INTEGER;
            return;
        }

        c = *lexer->pos;
        switch (c) {
            /* hex number starting with '0x' or '0X' */
            case 'x':
            case 'X':
                token->len++;
                /* try to peek the next character after the 'x' */
                if (advance(lexer) >= lexer->end) {
                    token->type = TOK_ERR;
                    return;
                }

                /* scan rest of hex value */
                c = *lexer->pos;
                while (isdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F')) {
                    token->len++;
                    /* reached eof */
                    if (advance(lexer) >= lexer->end) {
                        break;
                    }
                    c = *lexer->pos;
                }
                /* todo: add support for the error message */
                token->type = token->len > 2 ? TOK_LIT_INTEGERHEX : TOK_ERR;
                return;

            /* floating point starting with '0e' or '0E' */
            case 'e':
            case 'E':
                token->len++;
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
                    c = *lexer->pos;
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

            c = *lexer->pos;
            if (c == 'e' || c == 'E') {
                goto exponent;
            }
        }
        token->type = TOK_LIT_REAL;
        return;

exponent:
        token->len++;
        c = *(advance(lexer));
        /* todo: support +/- prefix */
        while (isdigit(c)) {
            token->len++;
            /* reached eof */
            if (++lexer->pos >= lexer->end) {
                break;
            }
            c = *lexer->pos;
        }
        token->type = TOK_LIT_REAL;
        return;
    }

    /* scan integers */
    while (advance(lexer) < lexer->end) {
        c = *lexer->pos;

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
    token->pos = lexer->pos;
    token->len = 0;

    do {
start:
        token->len++;
        /* reached eof with the closing '"' character */
        if (advance(lexer) >= lexer->end) {
            token->type = TOK_ERR;
            return;
        }
        c = *lexer->pos;
        if (c == '\\') {
            /* try to peak the next character */
            if (lexer->pos + 1 >= lexer->end) {
                continue;
            }

            c = *(lexer->pos + 1);
            /* skip current '"' if it was preceeded by a '\' */
            if (c == '"') {
                advance(lexer);
                goto start;
            }
        }
    } while (c != '"');

    token->type = TOK_LIT_STRING;
    /* skip the closing '"' character */
    advance(lexer);
}

void lexer_init(struct lexer *lexer, char *path, char *src) {
    lexer->path = path;
    lexer->src = src;
    lexer->pos = src;
    lexer->end = src + strlen(src);
    lexer->loc.ln = 1;
    lexer->loc.col = 1;

#ifdef LEXER_DEBUG
    struct token token;
    lexer_scan(lexer, &token);
    while (token.type != TOK_EOF) {
        prettyprint_token(&token);
        lexer_scan(lexer, &token);
    }
    /* reset */
    lexer->loc.ln = 1;
    lexer->loc.col = 1;
    lexer->pos = src;
#endif
}

void lexer_scan(struct lexer *lexer, struct token *token) {
    /* set tok to TOK_EOF if we reached eof */
    if (lexer->pos >= lexer->end) {
        token_init(token, TOK_EOF, lexer->end, 0, lexer->loc);
        return;
    }

    /* todo: scan comments */
    char c = *lexer->pos;
    /* skip spaces except '\n' */
    while (isspace(c) && c != '\n') {
        c = *(advance(lexer));
        /* reached eof */
        if (lexer->pos >= lexer->end) {
            token_init(token, TOK_EOF, lexer->end, 0, lexer->loc);
            return;
        }
    }

    switch (c) {
        case '"':
            /* scan string literals */
            scan_string(lexer, token);
            return;

        case '\n':
            token_init(token, TOK_LF, lexer->pos, 1, lexer->loc);
            lexer->loc.ln++;
            lexer->loc.col = 1;
            lexer->pos++;
            return;
        case '.':
            /* if no character after then it must be a dot */
            if (lexer->pos + 1 >= lexer->end) {
                token_init(token, TOK_DOT, lexer->pos, 1, lexer->loc);
                advance(lexer);
                return;
            }

            /* if next character is a digit it must be a number (floating point) */
            char p = *(lexer->pos + 1);
            if (isdigit(p)) {
                scna_number(lexer, token);
                return;
            }

            token_init(token, TOK_DOT, lexer->pos, 1, lexer->loc);
            advance(lexer);
            return;
        case ':':
            token_init(token, TOK_COLON, lexer->pos, 1, lexer->loc);
            advance(lexer);
            return;
        case '_':
            /* scan identifiers starting with '_' */
            scan_identifier(lexer, token);
            return;

        case '(':
            token_init(token, TOK_LPAREN, lexer->pos, 1, lexer->loc);
            advance(lexer);
            return;
        case ')':
            token_init(token, TOK_RPAREN, lexer->pos, 1, lexer->loc);
            advance(lexer);
            return;
        case '+':
            token_init(token, TOK_OP_ADD, lexer->pos, 1, lexer->loc);
            advance(lexer);
            return;
        case '-':
            token_init(token, TOK_OP_SUB, lexer->pos, 1, lexer->loc);
            advance(lexer);
            return;
        case '/':
            token_init(token, TOK_OP_DIV, lexer->pos, 1, lexer->loc);
            advance(lexer);
            return;
        case '*':
            token_init(token, TOK_OP_MUL, lexer->pos, 1, lexer->loc);
            advance(lexer);
            return;

        default:
            /* scan identifiers or keywords */
            if (isalpha(c)) {
                scan_identifier(lexer, token);
                return;
            }

            /* scan number literals */
            if (isdigit(c)) {
                scna_number(lexer, token);
                return;
            }

            /* unknown character */
            token_init(token, TOK_ERR, lexer->pos, 1, lexer->loc);
            advance(lexer);
            return;
    }
}