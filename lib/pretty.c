#include <stdio.h>
#include "string.h"
#include "visitor.h"
#include "token.h"
#include "pretty.h"

void prettyprint_token(struct token *token) {
	char buffer[255];
	token_value(token, buffer);
	printf("tok[%d,%d:%zu](%d, '%s')\n", token->loc.ln, token->loc.col, token->len, token->type, buffer);
}

/* ¯\_(ツ)_/¯ */
int depth;

static void indent() {
	for (int i = 0; i < depth; i++) {
		printf("    ");
	}

	if (depth > 0) {
		printf("\\----");
	}
}

static void print_node(struct visitor *visitor, struct node *node) {
	if (node != NULL) {
		visitor_visit(visitor, node);
	} else {
		indent();
		printf("NULL\n");
	}
}

static void print_op(enum token_type type) {
	switch (type) {
		case TOK_OP_ADD: /* + */
			printf("+");
			break;
		case TOK_OP_SUB: /* - */
			printf("-");
			break;
		case TOK_OP_MUL: /* * */
			printf("*");
			break;
		case TOK_OP_DIV: /* / */
			printf("/");
			break;
		case TOK_OP_LOGICAL_NOT: /* NOT */
			printf("!");
			break;
		case TOK_OP_LOGICAL_AND: /* AND */
			printf("&&");
			break;
		case TOK_OP_LOGICAL_OR: /* OR */
			printf("||");
			break;
		default:
			printf("%d", type);
			break;
	}
}

static void print_block(struct visitor *visitor, struct node_block *block) {
	indent();
	printf("block(%zu):\n", block->stmts.count);
	depth++;
	for (int i = 0; i < block->stmts.count; i++) {
		print_node(visitor, vector_get(&block->stmts, i));
	}
}

static void print_ident(struct visitor *visitor, struct node_ident *ident) {
	indent();
	printf("ident(%s)\n", ident->val);
}

static void print_string(struct visitor *visitor, struct node_string *string) {
	indent();
	printf("string(%zu, '%s')\n", string->val->length, string->val->buffer);
}

static void print_real(struct visitor *visitor, struct node_real *real) {
	indent();
	printf("real(%f)\n", real->val);
}

static void print_integer(struct visitor *visitor, struct node_integer *integer) {
	indent();
	printf("int(%d)\n", integer->val);
}

static void print_boolean(struct visitor *visitor, struct node_boolean *boolean) {
	indent();
	printf("boolean(%d)\n", boolean->val);
}

static void print_op_unary(struct visitor *visitor, struct node_op_unary *op_unary) {
	indent();
	printf("op_unary:\n");
	depth++;

	indent();
	printf("op: ");
	print_op(op_unary->op);
	printf("\n");

	print_node(visitor, op_unary->expr);
	depth--;
}

static void print_op_binary(struct visitor *visitor, struct node_op_binary *op_binary) {
	indent();
	printf("op_binary:\n");
	depth++;
	print_node(visitor, op_binary->left);

	indent();
	printf("op: ");
	print_op(op_binary->op);
	printf("\n");

	print_node(visitor, op_binary->right);
	depth--;
}

static void print_stmt_decl(struct visitor *visitor, struct node_stmt_decl *decl) {
	indent();
	printf("decl(%s, %s)\n", decl->ident->val, decl->type_ident->val);
}

static void print_stmt_assign(struct visitor *visitor, struct node_stmt_assign *assign) {
	indent();
	printf("assign(%s):\n", assign->ident->val);
	depth++;
	print_node(visitor, assign->right);
	depth--;
}

static void print_stmt_output(struct visitor *visitor, struct node_stmt_output *output) {
	indent();
	printf("output:\n");
	depth++;
	print_node(visitor, output->expr);
	depth--;
}

void prettyprint_node(struct node *node) {
	struct visitor visitor;
	visitor.visit_block = print_block;
	visitor.visit_ident = print_ident;
	visitor.visit_real = print_real;
	visitor.visit_integer = print_integer;
	visitor.visit_string = print_string;
	visitor.visit_boolean = print_boolean;
	visitor.visit_op_unary = print_op_unary;
	visitor.visit_op_binary = print_op_binary;
	visitor.visit_stmt_decl = print_stmt_decl;
	visitor.visit_stmt_assign = print_stmt_assign;
	visitor.visit_stmt_output = print_stmt_output;

	visitor_visit(&visitor, node);
}
