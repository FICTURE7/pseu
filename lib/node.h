#ifndef NODE_H
#define NODE_H

#include <stdbool.h>
#include "vector.h"
#include "token.h"

enum op_type {
	OP_ADD = TOK_OP_ADD,
	OP_SUB,
	OP_MUL,
	OP_DIV,
	OP_LOGICAL_NOT,
	OP_LOGICAL_AND,
	OP_LOGICAL_OR
};

enum node_type {
	NODE_BLOCK,

	/* literals */
	NODE_LIT_INTEGER,
	NODE_LIT_REAL,
	NODE_LIT_STRING,
	NODE_LIT_BOOLEAN,

	NODE_OP_UNARY,
	NODE_OP_BINARY,

	NODE_STMT_DECLARE,
	NODE_STMT_OUTPUT,
	NODE_STMT_IF,
	NODE_STMT_WHILE
};

struct node {
	enum node_type type;
};

struct node_block {
	struct node base;
	struct vector stmts;
};

struct node_boolean {
	struct node base;
	bool val;
};

struct node_integer {
	struct node base;
	int val;
};

struct node_real {
	struct node base;
	float val;
};

struct node_string {
	struct node base;
	char *val;
};

struct node_op_unary {
	struct node base;
	enum op_type op;
	struct node *expr;
};

struct node_op_binary {
	struct node base;
	enum op_type op;
	struct node *right;
	struct node *left;
};

struct node_stmt_decl {
	struct node base;
	char *ident;
	char *type;
};

struct node_stmt_output {
	struct node base;
	struct node *expr;
};

struct node_stmt_if {
	struct node base;
	struct node *cond;
	struct node_block *then_block;
	struct node_block *else_block;
};

struct node_stmt_while {
	struct node base;
	struct node *expr;
	struct node_block *block;
};

#endif
