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
	struct token token = parser->token;
	eat(parser);

	struct node_string *node = malloc(sizeof(struct node_string));
	node->base.type = NODE_LIT_STRING;
	node->val = malloc(token.len + 1);
	node->val[token.len] = '\0';
	memcpy(node->val, token.loc.pos, token.len);

	return node;
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
	return val;
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
	return val;
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

	return node;
}

/*
 *	primary = number / boolean / ("+"/"-"/"NOT") primary / "(" expression ")"
 */
static struct node *primary(struct parser *parser) {
	switch (parser->token.type) {
		/* unary operations */
		case TOK_OP_ADD:
		case TOK_OP_SUB:
		case TOK_OP_LOGICAL_NOT:
			struct node_op_unary *unop = malloc(sizeof(struct node_op_unary));
			unop->base.type = NODE_OP_UNARY;
			unop->op = parser->token.type;
			eat(parser);

			unop->expr = primary(parser);
			return unop;

		/* expressions in parenthesis */
		case TOK_LPAREN:
			/* eat '(' */
			eat(parser);
			struct node *node = expression(parser);
			if (!node) {
				error(parser, &parser->token, "expected an expression");
			}
			if (parser->token.type != TOK_RPAREN) {
				error(parser, &parser->token, "expected a ')'");
			}
			/* eat ')' */
			eat(parser);
			return node;

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
			struct node *new_rhs = expression_rhs(parser, lhs, cur_precedence + 1);
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
		lhs = new_lhs;
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
	return decl;
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

	return output;
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
			/* err */
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

	if (parser->token.type != TOK_LF && parser->token.type != TOK_EOF) {
		error(parser, &parser->token, "expected '\\n' (linefeed)");
		/* skip tokens until a line feed or eof */
		do {
			eat(parser);
		} while (parser->token.type != TOK_LF && parser->token.type != TOK_EOF);
	}

	while (parser->token.type == TOK_LF) {
		eat(parser);
		stmt = statement(parser);
		if (stmt) {
			vector_add(&block->stmts, stmt);
		}
	}

	return block;
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