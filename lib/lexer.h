#ifndef LEXER_H
#define LEXER_H

#include "token.h"
#include "location.h"

struct lexer {
	char *path;
	char *src;
	char *pos;
	char *end;
	struct location loc;
};

void lexer_init(struct lexer *lexer, char *path, char *buffer);

void lexer_scan(struct lexer *lexer, struct token *token);

#endif