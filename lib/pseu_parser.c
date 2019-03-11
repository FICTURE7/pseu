#include <string.h>
#include <stdarg.h>

#include "pseu_vm.h"
#include "pseu_node.h"
#include "pseu_value.h"
#include "pseu_lexer.h"
#include "pseu_debug.h"
#include "pseu_string.h"
#include "pseu_parser.h"
#include "pseu_symbol.h"

/* TODO: Check for lexer error tokens. */
/* TODO: Consider using UTF-8 encoding for strings or something. */

static int get_precedence(enum token_type type) {
	switch (type) {
		case TOK_OP_ADD:
			return 20;
		case TOK_OP_SUB:
			return 30;
		case TOK_OP_MUL:
			return 40;
		case TOK_OP_DIV:
			return 50;
		case TOK_OP_LOGICAL_AND:
			return 60;
		case TOK_OP_LOGICAL_OR:
			return 70;

		default:
			return -1;
	}
}

static void error(struct parser *parser, struct location loc,
				const char *format, ...) {
	pseu_vm_t *vm = parser->state->vm;
	char buffer[256];

	va_list args;
	va_start(args, format);	
	vsprintf(buffer, format, args);
	va_end(args);

	parser->error_count++;
	if (vm->config.onerror) {
		vm->config.onerror(vm, PSEU_ERROR_COMPILE, loc.ln, loc.col, buffer);
	}
}

static void warn(struct parser *parser, struct location loc, 
				const char *format, ...) {
	pseu_vm_t *vm = parser->state->vm;
	char buffer[256];

	va_list args;
	va_start(args, format);	
	vsprintf(buffer, format, args);
	va_end(args);

	if (vm->config.onwarn) {
		vm->config.onwarn(vm, PSEU_WARN_COMPILE, loc.ln, loc.col, buffer);
	}
}

/* Consumes the current token and moves to the next token. */
static void eat(struct parser *parser) {
	lexer_next(parser->lexer, &parser->token);

	/* TODO: This can be set directly by the lexer, to save some performance. */
	switch (parser->token.type) {
		case TOK_ERR_INVALID_HEX:
			error(parser, parser->lexer->loc, "Invalid hexadecimal number");
			break;
		case TOK_ERR_INVALID_EXP:
			error(parser, parser->lexer->loc, "Invalid exponential value");
			break;
		case TOK_ERR_INVALID_STRING:
			error(parser, parser->lexer->loc, "Invalid string");
			break;
		case TOK_ERR_UNKNOWN_CHAR:
			error(parser, parser->lexer->loc, "Unknown character");
			break;

		default:
			break;
	}
}

static int expect(struct parser *parser, enum token_type type,
				const char *message) {
	if (parser->token.type != type) {
		error(parser, parser->token.loc, message);
		return 1;
	}

	eat(parser);
	return 0;
}

/* Tries to recover to the next comma. */
static void panic_comma(struct parser *parser) {
	do {
		eat(parser);
	} while (parser->token.type != TOK_LF &&
				parser->token.type != TOK_EOF &&
				parser->token.type != TOK_COMMA);
}

/* Tries to recover to the next statement (new line or end of file). */
static void panic_statement(struct parser *parser) {
	while (parser->token.type != TOK_LF && parser->token.type != TOK_EOF) {
		eat(parser);
	}
}

static struct node *function(struct parser *parser);
static struct node *expression(struct parser *parser);

static struct node *identifier(struct parser *parser, const char *type) {
	struct token token = parser->token;

	if (parser->token.type != TOK_IDENT) {
		error(parser, parser->token.loc, "Expected a %s identifier", type);
		return NULL;
	}

	/* Eat identifier token. */
	eat(parser);

	/* TODO: Consider using node_ident with a flexible array instead. */
	char *buffer = pseu_alloc(parser->state->vm, token.len + 1);
	buffer[token.len] = '\0';
	memcpy(buffer, token.loc.pos, token.len);

	struct node_ident *node = 
		pseu_alloc(parser->state->vm, sizeof(struct node_ident));
	node->base.type = NODE_IDENT;
	node->val = buffer;
	return (struct node *)node;
}

/*
 * parameter = identifier ":" identifier
 */
static struct node *parameter(struct parser *parser) {
	struct node_param *node = 
			pseu_alloc(parser->state->vm, sizeof(struct node_param));
	node->base.type = NODE_PARAM;
	node->ident = (struct node_ident *)identifier(parser, "parameter");

	/* Expect ":" token after identifier. */
	expect(parser, TOK_COLON, "Expected ':' after parameter identifier");

	node->type_ident = (struct node_ident *)identifier(parser, "parameter type");
	return (struct node *)node;
}

/*
 * parameter-list = "(" parameter { "," parameter } ")"
 */
static void parameter_list(struct parser *parser, struct vector *params) {	
	expect(parser, TOK_LPAREN, "Expected '(', for begining of parameter list");

	while (parser->token.type == TOK_IDENT) {
		struct node_param *param = (struct node_param *)parameter(parser);
		vector_add(parser->state->vm, params, param);

		/* Eat comma if present. */
		if (parser->token.type == TOK_COMMA) {
			eat(parser);
		}
	}

	expect(parser, TOK_RPAREN, "Expected ')', after parameter list");
}

/*
 * argument-list = "(" expression { "," expression } ")"
 */
static void argument_list(struct parser *parser, struct vector *args) {
	/* Eat "(" token. */
	eat(parser);

	/* If next token is ")", exit early. */
	if (parser->token.type == TOK_RPAREN) {
		goto exit;
	}

	do {
		struct node *arg = expression(parser);
		if (!arg) {
			error(parser, parser->token.loc, "Expected argument expression");
			panic_comma(parser);
		} else {
			vector_add(parser->state->vm, args, arg);
		}

		if (parser->token.type == TOK_COMMA) {
			eat(parser);
		}
	} while (parser->token.type != TOK_RPAREN);

exit:
	/* Eat ")" token. */
	eat(parser);
}

static struct node *string(struct parser *parser) {
	char *buf = pseu_alloc(parser->state->vm, parser->token.len);

	/* TODO: Might want to rework this. */

	/* Unescape string. */
	size_t len = parser->token.len;
	char *bufpos = buf;
	char *tokpos = parser->token.loc.pos;
	char *tokend = parser->token.loc.pos + parser->token.len;
	for (int i = 0; i < len; i++) {
		char c = *tokpos;
		if (c == '\\') {
			/* Reached end of string. */
			if (tokpos + 1 > tokend) {
				goto add;
			}

			c = *(tokpos + 1);
			switch (c) {
				case 'a':
					c = '\a';
					goto add_unescape;
				case 'b':
					c = '\b';
					goto add_unescape;
				case 'f':
					c = '\f';
					goto add_unescape;
				case 'n':
					c = '\n';
					goto add_unescape;
				case 'r':
					c = '\r';
					goto add_unescape;
				case 't':
					c = '\t';
					goto add_unescape;
				case 'v':
					c = '\v';
					goto add_unescape;
				case '\"':
					c = '\"';
					goto add_unescape;
				case '\'':
					c = '\'';
					goto add_unescape;
				case '\\':
					c = '\\';
					goto add_unescape;
				default: {
					/* Skip unknown escape characters. */
					struct location loc = {
						.ln = parser->token.loc.ln,
						.col = parser->token.loc.col + 
								(tokpos + 1 - parser->token.loc.pos),
						.pos = tokpos + 1
					};
					warn(parser, loc, "Unknown escape character in string");
					len--;
					goto add;
				}
			}
		}
		goto add;

add_unescape:
		/* Adds an unescaped character. */
		len--;
		tokpos++;
add:
		tokpos++;
		/* Adds a single character. */
		*bufpos++ = c;
	}

	/* Eat string token. */
	eat(parser);

	struct node_string *node = 
			pseu_alloc(parser->state->vm, sizeof(struct node_string));
	node->base.type = NODE_LIT_STRING;
	node->val = string_table_intern(&parser->state->vm->strings, buf, len);

	/* 
	 * free buffer, since string_table_intern() has duplicated 
	 * the string, or returned an interned string
	 */
	pseu_free(parser->state->vm, buf);
	return (struct node *)node;
}

static struct node *real(struct parser *parser) {
	char *buffer = pseu_alloc(parser->state->vm, parser->token.len);
	struct node_real *node = 
			pseu_alloc(parser->state->vm, sizeof(struct node_real));

	memcpy(buffer, parser->token.loc.pos, parser->token.len);
	node->base.type = NODE_LIT_REAL;
	node->val = strtof(buffer, NULL);
	pseu_free(parser->state->vm, buffer);

	/* Eat real token. */
	eat(parser);
	return (struct node *)node;
}

static struct node *integer(struct parser *parser) {
	char *buffer = NULL;
	struct node_integer *node = 
			pseu_alloc(parser->state->vm, sizeof(struct node_integer));

	node->base.type = NODE_LIT_INTEGER;
	switch (parser->token.type) {
		case TOK_LIT_INTEGER:
			buffer = pseu_alloc(parser->state->vm, parser->token.len);
			memcpy(buffer, parser->token.loc.pos, parser->token.len);
			node->val = strtol(buffer, NULL, 10);
			break;
		case TOK_LIT_INTEGERHEX:
			/* Skip the first '0x' characters. */
			buffer = pseu_alloc(parser->state->vm, parser->token.len - 2);
			memcpy(buffer, parser->token.loc.pos + 2, parser->token.len - 2);
			node->val = strtol(buffer, NULL, 16);
			break;

		default:
			/* Unexpected token type (should not happen). */
			pseu_free(parser->state->vm, node);
			eat(parser);
			return NULL;
	}

	pseu_free(parser->state->vm, buffer);
	/* Eat integer token. */
	eat(parser);
	return (struct node *)node;
}

/*
 * number = integer / real
 */
static struct node *number(struct parser *parser) {
	switch (parser->token.type) {
		case TOK_LIT_INTEGERHEX:
		case TOK_LIT_INTEGER:
			return integer(parser);
		case TOK_LIT_REAL:
			return real(parser);
		default:
			/* Unexpected token type. */
			return NULL;
	}
}

/*
 * boolean = "TRUE" / "FALSE"
 */
static struct node *boolean(struct parser *parser) {
	bool val = -1;
	switch (parser->token.type) {
		case TOK_LIT_BOOLEAN_TRUE:
			val = true;
			break;
		case TOK_LIT_BOOLEAN_FALSE:
			val = false;
			break;
		default:
			/* Unexpected token type. */
			return NULL;
	}

	struct node_boolean *node = 
			pseu_alloc(parser->state->vm, sizeof(struct node_boolean));
	node->base.type = NODE_LIT_BOOLEAN;
	node->val = val;

	/* Eat "TRUE"/"FALSE" token. */
	eat(parser);
	return (struct node *)node;
}

/*
 * call = identifier argument-list
 */
static struct node *call(struct parser *parser, struct node_ident *ident) {
	struct node_call *call = 
		pseu_alloc(parser->state->vm, sizeof(struct node_call));
	call->base.type = NODE_CALL;
	call->fn_ident = ident;

	vector_init(parser->state->vm, &call->args, 4);
	argument_list(parser, &call->args);
	return (struct node *)call;
}

/*
 * primary = string / number / boolean / identifier / call
 *		/ ("+"/"-"/"NOT") primary / "(" expression ")"
 */
static struct node *primary(struct parser *parser) {
	switch (parser->token.type) {
		/* Unary operations. */
		case TOK_OP_ADD:
		case TOK_OP_SUB:
		case TOK_OP_LOGICAL_NOT: {
			struct node_op_unary *unop = 
				pseu_alloc(parser->state->vm, sizeof(struct node_op_unary));
			unop->base.type = NODE_OP_UNARY;
			unop->op = parser->token.type;

			/* Eat operator. */
			eat(parser);

			unop->expr = primary(parser);
			if (!unop->expr) {
				error(parser, parser->token.loc,
						"Expected expression after unary operator");
			}
			return (struct node *)unop;
		}

		/* Expressions in parenthesis. */
		case TOK_LPAREN: {
			/* Eat "(" token. */
			eat(parser);

			/* Parse expression. */
			struct node *node = expression(parser);

			/* TODO: Check if NULL and stuff. */

			/* Expect ")" after end of expression. */
			if (parser->token.type != TOK_RPAREN) {
				error(parser, parser->token.loc, 
						"Expected a ')' after expression");
			} else {
				/* Eat ")" token. */
				eat(parser);
			}
			return node;
		}

		/* String literals. */
		case TOK_LIT_STRING:
			return string(parser);

		/* Boolean literals. */
		case TOK_LIT_BOOLEAN_TRUE:
		case TOK_LIT_BOOLEAN_FALSE:
			return boolean(parser);

		/* Number literals. */
		case TOK_LIT_INTEGERHEX:
		case TOK_LIT_INTEGER:
		case TOK_LIT_REAL:
			return number(parser);

		/* Variable identifiers & calls. */
		case TOK_IDENT: {
			struct node *ident = identifier(parser, NULL);
			/* If next token is a '(' then its a function/procedure call. */
			if (parser->token.type == TOK_LPAREN) {
				return call(parser, (struct node_ident *)ident);
			}
			return ident;
		}

		default:
			/* unexpected token type */
			return NULL;
	}
}

static struct node *expression_rhs(struct parser *parser, 
				struct node *lhs, int min_precedence) {
	while (true) {
		/* Check current operator's precedence. */
		enum token_type op = parser->token.type;
		int cur_precedence = get_precedence(op);
		if (cur_precedence < min_precedence) {
			return lhs;
		}

		/* 
		 * Eat operator and move to next token which should parse to a primary 
		 * node
		 */
		eat(parser);

		struct node *rhs = primary(parser);
		if (!rhs) {
			error(parser, parser->token.loc, "Expected an expression");
		}

		int next_precedence = get_precedence(parser->token.type);
		if (cur_precedence < next_precedence) {
			struct node *new_rhs = expression_rhs(parser, rhs,
						cur_precedence + 1);
			if (!new_rhs) {
				return NULL;
			}
			rhs = new_rhs;
		}

		struct node_op_binary *new_lhs = 
				pseu_alloc(parser->state->vm, sizeof(struct node_op_binary));
		new_lhs->base.type = NODE_OP_BINARY;
		new_lhs->op = op;
		new_lhs->left = lhs;
		new_lhs->right = rhs;
		lhs = (struct node *)new_lhs;
	}
}

static struct node *expression(struct parser *parser) {
	/* Parse left hand side of expression. */
	struct node *lhs = primary(parser);
	if (!lhs) {
		return NULL;
	}

	/* Parse right hand side of expression. */
	return expression_rhs(parser, lhs, 0);
}

/*
 * declare-statement = "DECLARE" identifier ":" identifier
 *
 * TODO: Add single line multi-variable declare syntax:
 * DECLARE x,y,z: INTEGER
 * TODO: Add assign expression to statement:
 * DECLARE x: INTEGER <- 1
 * TODO: Make sure we can do this as well:
 * DECLARE x,y,z: INTEGER <- 1
 */
static struct node *declare_statement(struct parser *parser) {
	/* Eat "DECLARE" keyword. */
	eat(parser);

	struct node_stmt_decl *node =
			pseu_alloc(parser->state->vm, sizeof(struct node_stmt_decl));
	node->base.type = NODE_STMT_DECLARE;

	/* Parse variable identifier. */
	node->ident = (struct node_ident *)identifier(parser, "variable");
	if (!node->ident) {
		panic_statement(parser);
		return (struct node *)node;
	}

	/* Expect ":" after variable identifier. */
	if (expect(parser, TOK_COLON, "Expected ':' after variable identifier")) {
		panic_statement(parser);
		return (struct node *)node;
	}

	/* Parse type identifier. */
	node->type_ident = (struct node_ident *)identifier(parser, "variable type");
	if (!node->type_ident) {
		panic_statement(parser);
		return (struct node *)node;
	}

	return (struct node *)node;
}

/*
 * assign-statement = identifier "<-" expression
 */
static struct node *assign_statement(struct parser *parser) {
	struct node_stmt_assign *node =
			pseu_alloc(parser->state->vm, sizeof(struct node_stmt_assign));
	node->base.type = NODE_STMT_ASSIGN;

	/* Parse the variable being assigned's identifier. */
	node->ident = (struct node_ident *)identifier(parser, "variable");

	/* Expect "<-" after variable identifier. */
	if (expect(parser, TOK_ASSIGN,
			"Expected assign operator, '<-', after variable identifier")) {
		panic_statement(parser);
		return (struct node *)node;
	};

	/* Parse expression to which the identifier is being assigned to. */
 	node->right = expression(parser);
	if (!node->right) {
		error(parser, parser->token.loc, "Expected assign expression");
		panic_statement(parser);
	}

	return (struct node *)node;
}

/*
 * output-statement = "OUTPUT" expression
 */
static struct node *output_statement(struct parser *parser) {
	/* Eat "OUTPUT" keyword. */
	eat(parser);

	struct node_stmt_output *node =
			pseu_alloc(parser->state->vm, sizeof(struct node_stmt_output));
	node->base.type = NODE_STMT_OUTPUT;
	/* Parse expression to output. */
	node->expr = expression(parser);
	if (!node->expr) {
		error(parser, parser->token.loc, "Expected expression to output");
		panic_statement(parser);
	}

	return (struct node *)node;
}

/*
 * return-statement = "RETURN" expression
 */
static struct node *return_statement(struct parser *parser) {
	/* Eat "RETURN" keyword. */
	eat(parser);

	struct node_stmt_return *node = 
			pseu_alloc(parser->state->vm, sizeof(struct node_stmt_return));
	node->base.type = NODE_STMT_RETURN;
	/* Parse expression to return. */
	node->expr = expression(parser);
	if (!node->expr) {
		error(parser, parser->token.loc, "Expected expression to return");
		panic_statement(parser);
	}

	return (struct node *)node;
}

/*
 * function-block-statement = declare-statement / assign-statement /
 * 					output-statement / return-statement / empty-statement
 */
static struct node *function_block_statement(struct parser *parser) {
	switch (parser->token.type) {
		case TOK_KW_DECLARE:
			return declare_statement(parser);
		case TOK_KW_OUTPUT:
			return output_statement(parser);
		case TOK_KW_RETURN:
			return return_statement(parser);
		case TOK_IDENT: {
			struct token peek;
			lexer_peek(parser->lexer, &peek);

			/* If next token is "<-" then parse assgin statement. */
			if (peek.type == TOK_ASSIGN) {
				return assign_statement(parser);
			}
			break;
		}

		default:
			break;
	}

	error(parser, parser->token.loc,
			"Only declaration, assignment, output and return statements can be "
			"used");
	return NULL;
}

/*
 * function-block = function-statement { LF function-statement }
 */
static struct node *function_block(struct parser *parser) {
	struct node_block *node =
			pseu_alloc(parser->state->vm, sizeof(struct node_block));
	node->base.type = NODE_BLOCK;
	vector_init(parser->state->vm, &node->stmts, 16);

	/* Parse statements until ENDFUNCTION keyword or end of file. */
	while (parser->token.type != TOK_KW_ENDFUNCTION &&
			parser->token.type != TOK_EOF) {
		/* empty-statement definition, just consume the token. */
		if (parser->token.type == TOK_LF) {
			eat(parser);
			continue;
		}

		/* Parse statement which are allowed in functions. */
		struct node *stmt = function_block_statement(parser);
		if (stmt) {
			vector_add(parser->state->vm, &node->stmts, stmt);
		}

		/* Expect new line or end of file at end of statement. */
		if (expect(parser, TOK_LF, "Expected new line")) {
			panic_statement(parser);
		}
	}

	return (struct node *)node;
}

/*
 * function = "FUNCTION" identifier parameter-list [":" identifier]
 * 				function-block "ENDFUNCTION"
 */
static struct node *function(struct parser *parser) {
	/* Eat "FUNCTION" keyword. */
	eat(parser);

	struct node_function *node = 
			pseu_alloc(parser->state->vm, sizeof(struct node_function));
	node->base.type = NODE_FUNCTION;
	vector_init(parser->state->vm, &node->params, 4);

	/* Parse function identifier. */
	node->ident = (struct node_ident *)identifier(parser, "function");

	/* Parse parameter list. */
	parameter_list(parser, &node->params);

	/* 
	 * If there is a colon after parameter list, parse return type identifier.
	 */
	struct node_ident *return_type_ident = NULL;
	if (parser->token.type == TOK_COLON) {
		eat(parser);
		return_type_ident = 
			(struct node_ident *)identifier(parser, "function return type");
	}

	expect(parser, TOK_LF, "Expected new line after function declaration");

	node->return_type_ident = return_type_ident;
	node->body = (struct node_block *)function_block(parser);

	expect(parser, TOK_KW_ENDFUNCTION,
			"Expected 'ENDFUNCTION' keyword after function body");
	return (struct node *)node;
}

/*
 * root-block-statement = declare-statement / assign-statement /
 * 					output-statement / empty-statement / function
 */
static struct node *root_block_statement(struct parser *parser) {
	switch (parser->token.type) {
		case TOK_KW_DECLARE:
			return declare_statement(parser);
		case TOK_KW_OUTPUT:
			return output_statement(parser);
		case TOK_IDENT: {
			struct token peek;
			lexer_peek(parser->lexer, &peek);

			/* If next token is "<-" then parse assgin statement. */
			if (peek.type == TOK_ASSIGN) {
				return assign_statement(parser);
			}
			break;
		}

		case TOK_KW_FUNCTION:
			return function(parser);

		default:
			break;
	}

	error(parser, parser->token.loc, 
			"Only declaration, assignment and output statements can be used");
	return NULL;
}

/*
 * root-block = root-statement { LF root-statement }
 */
static struct node *root_block(struct parser *parser) {
	struct node_block *node =
			pseu_alloc(parser->state->vm, sizeof(struct node_block));
	node->base.type = NODE_BLOCK;
	vector_init(parser->state->vm, &node->stmts, 16);

	/* Parse statements until end of file. */
	while (parser->token.type != TOK_EOF) {
		/* empty-statement definition, just consume the token. */
		if (parser->token.type == TOK_LF) {
			eat(parser);
			continue;
		}

		/* Parse top-level statement. */
		struct node *stmt = root_block_statement(parser);
		if (stmt) {
			vector_add(parser->state->vm, &node->stmts, stmt);
		}

		/* Expect new line or end of file at end of statement. */
		if (parser->token.type != TOK_LF && parser->token.type != TOK_EOF) {
			error(parser, parser->token.loc, "Expected new line or end of file");
			panic_statement(parser);
		} else {
			eat(parser);
		}
	}

	return (struct node *)node;
}

void parser_init(struct parser *parser, struct lexer *lexer,
					struct state *state) {
	pseu_assert(parser && lexer && state);

	parser->error_count = 0;
	parser->state = state;
	parser->lexer = lexer;

	/* Scan first token. */
	eat(parser);
}

struct node *parser_parse(struct parser *parser) {
	pseu_assert(parser);

	/* Parse list of top-level statements. */
	struct node *node = root_block(parser);
	/* Next token should be end of file. */
	expect(parser, TOK_EOF, "Expected end of file");
	return node;
}
