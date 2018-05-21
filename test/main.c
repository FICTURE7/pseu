#include <stdio.h>
#include <stdlib.h>
#include "test.h"
#include "lexer.h"
#include "token.h"

struct test_data_token {
	enum token_type type;
	char *value;
};

struct test_data_lexer {
	char *src;
	size_t tokens_count;
	struct test_data_token tokens[16];
};

int test_lexer() {
	struct lexer lexer;
	struct token token;

	struct test_data_lexer data[] = {
		{
			.src = "xD",
			.tokens = {
				{ TOK_IDENT, "xD" }
			},
			.tokens_count = 1
		},
		{
			.src = " 0  0.  0x  000  0e24  0.10    0.e5  0xFF   ",
			.tokens = {
				{ TOK_LIT_INTEGER, "0" },
				{ TOK_LIT_REAL, "0." },
				{ TOK_ERR, "0x" },
				{ TOK_LIT_INTEGER, "000" },
				{ TOK_LIT_REAL, "0e24" },
				{ TOK_LIT_REAL, "0.10" },
				{ TOK_LIT_REAL, "0.e5" },
				{ TOK_LIT_INTEGERHEX, "0xFF" },
			},
			.tokens_count = 6
		},
		{
			.src = " 1  5454  55.676  ",
			.tokens = {
				{ TOK_LIT_INTEGER, "1" },
				{ TOK_LIT_INTEGER, "5454" },
				{ TOK_LIT_REAL, "55.676" }
			},
			.tokens_count = 2
		},
		{
			.src = "  :   ",
			.tokens = {
				{ TOK_COLON, ":" }
			},
			.tokens_count = 1
		}
	};

	for (int i = 0; i < sizeof(data) / sizeof(struct test_data_lexer); i++) {
		struct test_data_lexer cur_data = data[i];
		lexer_init(&lexer, NULL, cur_data.src);

		for (int k = 0; k < cur_data.tokens_count; k++) {
			struct test_data_token tdata = cur_data.tokens[k];
			lexer_scan(&lexer, &token);
			TEST_EQ_TOK(token, tdata.type, tdata.value);
		}
	}
	return 0;
}

int main(int argc, char **argv) {
	TEST_INIT();
	TEST(test_lexer);
	TEST_DEINIT();

	getchar();
	return 0;
}
