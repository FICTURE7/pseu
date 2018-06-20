#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "test.h"
#include "vector.h"
#include "token.h"
#include "lexer.h"
#include "object.h"
#include "parser.h"
#include "vm.h"

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
				{ TOK_ERR_INVALID_HEX, "0x" },
				{ TOK_LIT_INTEGER, "000" },
				{ TOK_LIT_REAL, "0e24" },
				{ TOK_LIT_REAL, "0.10" },
				{ TOK_LIT_REAL, "0.e5" },
				{ TOK_LIT_INTEGERHEX, "0xFF" },
				{ TOK_EOF, "" }
			},
			.tokens_count = 9
		},
		{
			.src = " 1  5454  55.676  14e5 14.5e5  44e  43e-4   54e+55  ",
			.tokens = {
				{ TOK_LIT_INTEGER, "1" },
				{ TOK_LIT_INTEGER, "5454" },
				{ TOK_LIT_REAL, "55.676" },
				{ TOK_LIT_REAL, "14e5" },
				{ TOK_LIT_REAL, "14.5e5" },
				{ TOK_ERR_INVALID_EXP, "44e" },
				{ TOK_LIT_REAL, "43e-4" },
				{ TOK_LIT_REAL, "54e+55" },
				{ TOK_EOF, "" }
			},
			.tokens_count = 8
		},
		{
			.src = "  :   ",
			.tokens = {
				{ TOK_COLON, ":" },
				{ TOK_EOF, "" }
			},
			.tokens_count = 2
		},
		{
			.src = " // test comment\nxD",
			.tokens = {
				{ TOK_IDENT, "xD" },
				{ TOK_EOF, "" }
			},
			.tokens_count = 2
		},
		{
			.src = "1 / 2 // half",
			.tokens = {
				{ TOK_LIT_INTEGER, "1" },
				{ TOK_OP_DIV, "/" },
				{ TOK_LIT_INTEGER, "2" },
				{ TOK_EOF, "" }
			},
			.tokens_count = 4
		},
		{
			.src = " /* fancy comment \n new line \n test: 1 * 2 / 3 /* */ xD",
			.tokens = {
				{ TOK_IDENT, "xD" },
				{ TOK_EOF, "" }
			},
			.tokens_count = 2
		}
	};

	for (int i = 0; i < sizeof(data) / sizeof(struct test_data_lexer); i++) {
		struct test_data_lexer cur_data = data[i];
		lexer_init(&lexer, NULL, cur_data.src);

		for (int k = 0; k < cur_data.tokens_count; k++) {
			struct test_data_token tdata = cur_data.tokens[k];
			lexer_scan(&lexer, &token);
			TEST_EQ_TOK(token, tdata.type, tdata.value);

			/*
			if (tdata.type == TOK_EOF) {
				TEST_EQ(lexer.loc.pos, '\0');
			}
			*/
		}
	}
	return 0;
}

int test_vector() {
	struct vector vec;

	/* test growing of vector */
	vector_init(&vec);
	TEST_EQ(vec.count, 0);
	TEST_EQ(vec.capacity, 16);
	for (int i = 0; i < 17; i++) {
		vector_add(&vec, (void *)i);
	}
	TEST_EQ(vec.count, 17);
	TEST_EQ(vec.capacity, 32);

	/* test shrinking of vector */
	vector_remove(&vec, 0);
	TEST_EQ(vec.count, 16);
	TEST_EQ(vec.capacity, 16);
	TEST_EQ(vector_get(&vec, 0), 1);
	return 0;
}

void vector_addrange(struct vector *vec, void **items, int size) {
	for (int i = 0; i < size; i++) {
		vector_add(vec, items[i]);
	}
}

int test_unescape_string() {
	struct state state;
	struct lexer lexer;
	struct parser parser;
	struct node *node;

	state_init(&state);
	lexer_init(&lexer, NULL, "OUTPUT \"xD \\\"a \\ \"");
	parser_init(&parser, &state, &lexer);

	parser_parse(&parser, &node);
	return 0;
}

int test_string_intern() {
	struct string_table table;
	struct string_object *str1;
	struct string_object *str2;

	string_table_init(&table);
	str1 = string_table_intern(&table, "xD", 2);
	str2 = string_table_intern(&table, "xD", 2);
	
	TEST_EQ(str1, str2);
	TEST_EQ(table.count, 1);

	/*
	NOTE: hack string.c to always collide

	struct string_object *str3;
	struct string_object *str4;
	str3 = string_table_intern(&table, "xDD", 3);
	str4 = string_table_intern(&table, "xDD", 3);
	TEST_EQ(str3, str4);
	TEST_EQ(table.count, 1);

	string_table_intern(&table, "xDDD", 4);
	TEST_EQ(table.count, 1);
	*/

	return 0;
}

int test_vm() {
	struct vm vm;
	struct state state;

	vm_init(&vm);
	state_init(&state);

	vm_exec(&vm, &state);

	state_deinit(&state);
	return 0;
}

int main(int argc, char **argv) {
	TEST_INIT();
	//TEST(test_lexer);
	//TEST(test_vector);
	//TEST(test_unescape_string);
	//TEST(test_string_intern);
	TEST(test_vm)
	TEST_DEINIT();

#if WIN32
	getchar();
#endif
	return 0;
}
