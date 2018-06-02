#define PARSER_DEBUG

#include <string.h>
#include "node.h"
#include "lexer.h"
#include "diagnostic.h"
#include "parser.h"

#ifdef PARSER_DEBUG
#include <stdio.h>
#include "pretty.h"
#endif

static void eat(struct parser *parser) {
	lexer_scan(parser->lexer, &parser->token);
}

static void error(struct parser *parser, struct token *token, char *message) {
	struct diagnostic *diagnostic = malloc(sizeof(struct diagnostic));
	diagnostic->type = DIAGNOSTIC_TYPE_ERROR;
	diagnostic->loc = token->loc;
	diagnostic->message = message;

	vector_add(&parser->diagnostics, diagnostic);
#ifdef PARSER_DEBUG
	printf("error(ln: %d, col: %d): %s\n", token->loc.ln, token->loc.col, message);
#endif
}

static void warning(struct parser *parser, struct location loc, char *message) {
	struct diagnostic *diagnostic = malloc(sizeof(struct diagnostic));
	diagnostic->type = DIAGNOSTIC_TYPE_WARNING;
	diagnostic->loc = loc;
	diagnostic->message = message;

	vector_add(&parser->diagnostics, diagnostic);
#ifdef PARSER_DEBUG
	printf("warning(ln: %d, col: %d): %s\n", loc.ln, loc.col, message);
#endif
}

static void panic(struct parser *parser) {
	/* skip tokens until a line feed or eof */
	error(parser, &parser->token, "expected '\\n' (linefeed)");
	do {
		eat(parser);
	} while (parser->token.type != TOK_LF && parser->token.type != TOK_EOF);
}

static int precedence(enum token_type type) {
	switch (type) {
		case TOK_OP_ADD: /* + */
			return 20;
		case TOK_OP_SUB: /* - */
			return 30;
		case TOK_OP_MUL: /* * */
			return 40;
		case TOK_OP_DIV: /* / */
			return 50;
		case TOK_OP_LOGICAL_AND: /* AND */
			return 60;
		case TOK_OP_LOGICAL_OR: /* OR */
			return 70;
		default:
			return -1;
	}
}

static struct node *expression(struct parser *parser);

static char *identifier(struct parser *parser) {
	struct token token = parser->token;

	if (parser->token.type != TOK_IDENT) {
		error(parser, &parser->token, "expected an identifier");
		eat(parser);
		return NULL;
	}

	/* eat identifier */
	eat(parser);

	char *ident = malloc(token.len + 1);
	ident[token.len] = '\0';
	memcpy(ident, token.loc.pos, token.len);
	return ident;
}

static struct node *string(struct parser *parser) {
	/* todo: unescape strings */
	struct node_string *node = malloc(sizeof(struct node_string));
	node->base.type = NODE_LIT_STRING;
	node->val = malloc(parser->token.len + 1);

	/* unescape string */
	int count = parser->token.len;
	char *valpos = node->val;
	char *tokpos = parser->token.loc.pos;
	char *tokend = parser->token.loc.pos + parser->token.len;
	for (int i = 0; i < count; i++) {
		char c = *tokpos;
		if (c == '\\') {
			/* reached end of string */
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
					/* skip unknown escape characters */
					struct location loc = {
						.ln = parser->token.loc.ln,
						.col = parser->token.loc.col + (tokpos + 1 - parser->token.loc.pos),
						.pos = tokpos + 1
					};
					warning(parser, loc, "unknown escape character");
					count--;
					goto add;
				}
			}
		}
		goto add;

add_unescape:
		/* adds an unescaped character */
		count--;
		tokpos++;
add:
		tokpos++;
		/* adds a single character */
		*valpos++ = c;
	}

	node->val[count] = '\0';
	/* trim excess which may be caused from unescaping characters */
	if (count < parser->token.len) {
		realloc(node->val, count + 1);
	}

	/* eat string */
	eat(parser);
	/* todo: register in a string table for interning */
	return (struct node *)node;
}

static struct node *real(struct parser *parser) {
	char *buf = malloc(parser->token.len);
	struct node_real *val = malloc(sizeof(struct node_real));

	memcpy(buf, parser->token.loc.pos, parser->token.len);
	val->base.type = NODE_LIT_REAL;
	val->val = strtof(buf, NULL);
	free(buf);
	/* eat real */
	eat(parser);
	return (struct node *)val;
}

static struct node *integer(struct parser *parser) {
	char *buf = NULL;
	struct node_integer *val = malloc(sizeof(struct node_integer));

	val->base.type = NODE_LIT_INTEGER;
	switch (parser->token.type) {
		case TOK_LIT_INTEGER:
			buf = malloc(parser->token.len);
			memcpy(buf, parser->token.loc.pos, parser->token.len);
			val->val = strtol(buf, NULL, 10);
			break;
		case TOK_LIT_INTEGERHEX:
			/* skip the first '0x' characters */
			buf = malloc(parser->token.len - 2);
			memcpy(buf, parser->token.loc.pos + 2, parser->token.len - 2);
			val->val = strtol(buf, NULL, 16);
			break;
	}

	free(buf);
	/* eat integer */
	eat(parser);
	return (struct node *)val;
}

/*
 *	number = integer / real
 */
static struct node *number(struct parser *parser) {
	switch (parser->token.type) {
		case TOK_LIT_INTEGERHEX:
		case TOK_LIT_INTEGER:
			return integer(parser);
		case TOK_LIT_REAL:
			return real(parser);
	}
}

/*
 *	boolean = "TRUE" / "FALSE"
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
	}

	struct node_boolean *node = malloc(sizeof(struct node_boolean));
	node->base.type = NODE_LIT_BOOLEAN;
	node->val = val;
	eat(parser);
	return (struct node *)node;
}

/*
 *	primary = string / number / boolean / ("+"/"-"/"NOT") primary / "(" expression ")"
 */
static struct node *primary(struct parser *parser) {
	switch (parser->token.type) {
		/* unary operations */
		case TOK_OP_ADD:
		case TOK_OP_SUB:
		case TOK_OP_LOGICAL_NOT: {
			struct node_op_unary *unop = malloc(sizeof(struct node_op_unary));
			unop->base.type = NODE_OP_UNARY;
			unop->op = parser->token.type;
			/* eat operator */
			eat(parser);

			unop->expr = primary(parser);
			return (struct node *)unop;
		}

		/* expressions in parenthesis */
		case TOK_LPAREN: {
			/* eat '(' */
			eat(parser);
			struct node *node = expression(parser);
			if (parser->token.type != TOK_RPAREN) {
				error(parser, &parser->token, "expected a ')'");
			} else {
				/* eat ')' */
				eat(parser);
			}
			return node;
		}

		/* string literals */
		case TOK_LIT_STRING:
			return string(parser);

		/* boolean literals */
		case TOK_LIT_BOOLEAN_TRUE:
		case TOK_LIT_BOOLEAN_FALSE:
			return boolean(parser);

		/* integer literals */
		case TOK_LIT_INTEGERHEX:
		case TOK_LIT_INTEGER:
		case TOK_LIT_REAL:
			return number(parser);

		default:
			/* err */
			return NULL;
	}
}

static struct node *expression_rhs(struct parser *parser, struct node *lhs, int min_precedence) {
	while (true) {
		/* check current operators precedence */
		enum token_type op = parser->token.type;
		int cur_precedence = precedence(op);
		if (cur_precedence < min_precedence) {
			return lhs;
		}

		/* eat operator and move to next token which should parse to a primary node */
		eat(parser);

		struct node *rhs = primary(parser);
		if (!rhs) {
			error(parser, &parser->token, "expected an expression");
		}

		int next_precedence = precedence(parser->token.type);
		if (cur_precedence < next_precedence) {
			struct node *new_rhs = expression_rhs(parser, rhs, cur_precedence + 1);
			if (!new_rhs) {
				return NULL;
			}
			rhs = new_rhs;
		}

		struct node_op_binary *new_lhs = malloc(sizeof(struct node_op_binary));
		new_lhs->base.type = NODE_OP_BINARY;
		new_lhs->op = op;
		new_lhs->left = lhs;
		new_lhs->right = rhs;
		lhs = (struct node *)new_lhs;
	}
}

static struct node *expression(struct parser *parser) {
	struct node *lhs = primary(parser);
	if (!lhs) {
		error(parser, &parser->token, "expected an expression");
		return NULL;
	}
	return expression_rhs(parser, lhs, 0);
}

/*
 *	declare-statement = "DECLARE" identifier ":" identifier
 */
static struct node *declare_statement(struct parser *parser) {
	/* eat declare keyword */
	eat(parser);

	struct node_stmt_decl *decl = malloc(sizeof(struct node_stmt_decl));
	decl->base.type = NODE_STMT_DECLARE;
	decl->ident = identifier(parser);

	if (parser->token.type != TOK_COLON) {
		error(parser, &parser->token, "expected a ':'");
	}

	/* eat colon */
	eat(parser);

	decl->type = identifier(parser);
	/* todo: register declaration in a symbol table. */
	return (struct node *)decl;
}

/*
 *	output-statement = "OUTPUT" expression
 */
static struct node *output_statement(struct parser *parser) {
	/* eat output keyword */
	eat(parser);

	struct node *expr = expression(parser);
	struct node_stmt_output *output = malloc(sizeof(struct node_stmt_output));
	output->base.type = NODE_STMT_OUTPUT;
	output->expr = expr;

	return (struct node *)output;
}

/*
 *	statement = declare-statement | output-statement
 */
static struct node *statement(struct parser *parser) {
	switch (parser->token.type) {
		case TOK_KW_DECLARE:
			return declare_statement(parser);
		case TOK_KW_OUTPUT:
			return output_statement(parser);
		default:
			error(parser, &parser->token, "unexpected symbol");
			return NULL;
	}
}

/*
 *	block = statement | 1*(statement LF)
 */
static struct node *block(struct parser *parser) {
	struct node_block *block = malloc(sizeof(struct node_block));
	block->base.type = NODE_BLOCK;
	vector_init(&block->stmts);

	struct node *stmt = statement(parser);
	if (stmt) {
		vector_add(&block->stmts, stmt);
	}

	/* point parser to next statement if not line feed or eof */
	if (parser->token.type != TOK_LF && parser->token.type != TOK_EOF) {
		panic(parser);
	}

	while (parser->token.type == TOK_LF) {
		/* eat linefeed */
		eat(parser);
		stmt = statement(parser);

		/* point parser to next statement if not line feed or eof */
		if (parser->token.type != TOK_LF && parser->token.type != TOK_EOF) {
			panic(parser);
		}

		if (stmt) {
			vector_add(&block->stmts, stmt);
		}
	}

	return (struct node *)block;
}

void parser_init(struct parser *parser, struct lexer *lexer) {
	parser->lexer = lexer;
	vector_init(&parser->diagnostics);
	/* scan first token */
	lexer_scan(lexer, &parser->token);

#ifdef PARSER_DEBUG
	struct node *root;
	parser_parse(parser, &root);
	prettyprint_node(root);
	/* reset */
	lexer->loc.pos = lexer->src;
	lexer->loc.ln = 1;
	lexer->loc.col = 1;
	lexer_scan(lexer, &parser->token);
#endif
}

void parser_deinit(struct parser *parser) {
	/* free the diagnostic structs on the heap in the vector/list */
	for (int i = 0; i < parser->diagnostics.count; i++) {
		free(vector_get(&parser->diagnostics, i));
	}
	vector_deinit(&parser->diagnostics);
}

void parser_parse(struct parser *parser, struct node **root) {
	(*root) = block(parser);
	if (parser->token.type != TOK_EOF) {
		error(parser, &parser->token, "expected eof");
	}

	eat(parser);
}
