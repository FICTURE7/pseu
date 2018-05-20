#include <string.h>
#include "token.h"

void token_init(struct token *token, enum token_type type, char *pos, size_t len, struct location loc) {
	token->type = type;
	token->pos = pos;
	token->len = len;
	token->loc = loc;
}

void token_value(struct token *token, char *buffer) {
	buffer[token->len] = '\0';
	memcpy(buffer, token->pos, token->len);
}

int token_value_cmp(struct token *token, char *buffer) {
	return memcmp(token->pos, buffer, token->len) == 0;
}