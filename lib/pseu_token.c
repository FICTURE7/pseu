#include <string.h>
#include "pseu_token.h"

void token_init(struct token *token, enum token_type type,
				struct location loc, size_t len) {
	token->type = type;
	token->loc = loc;
	token->len = len;
}

void token_value(struct token *token, char *buffer) {
	buffer[token->len] = '\0';
	memcpy(buffer, token->loc.pos, token->len);
}

int token_value_cmp(struct token *token, char *buffer) {
	return memcmp(token->loc.pos, buffer, token->len) == 0;
}
