#include <stdio.h>
#include "test.h"
#include "lexer.h"
#include "token.h"

int test_lexer_invalid_hex() {
	struct lexer lexer;
	struct token token;

	lexer_init(&lexer, NULL, "0x");
	lexer_scan(&lexer, &token);

	TEST_EQ(token.type, TOK_ERR);
	return 0;
}

int test_lexer_single_digit_0() {
	struct lexer lexer;
	struct token token;

	lexer_init(&lexer, NULL, "0");
	lexer_scan(&lexer, &token);

	TEST_EQ(token.type, TOK_LIT_INTEGER);
	return 0;
}

int test_lexer_normal_number() {
	struct lexer lexer;
	struct token token;

	lexer_init(&lexer, NULL, "0050");
	lexer_scan(&lexer, &token);

	TEST_EQ(token.type, TOK_LIT_INTEGER);
	return 0;
}

int main(int argc, char **argv) {	
	TEST_INIT();
	TEST(test_lexer_invalid_hex);
	TEST(test_lexer_single_digit_0);
	TEST(test_lexer_normal_number);
	TEST_DEINIT();

	getchar();
    return 0;
}
