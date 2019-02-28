#ifndef PSEU_NODE_H
#define PSEU_NODE_H

#include <stdint.h>
#include <stdbool.h>

#include "pseu_vm.h"
#include "pseu_token.h"
#include "pseu_value.h"
#include "pseu_vector.h"

/*
 * Types of nodes.
 */
enum node_type {
	NODE_BLOCK,
	NODE_IDENT,

	/* Constant literals. */
	NODE_LIT_INTEGER,
	NODE_LIT_REAL,
	NODE_LIT_STRING,
	NODE_LIT_BOOLEAN,

	/* Operations, unary/binary. */
	NODE_OP_UNARY,
	NODE_OP_BINARY,

	/* Statements. */
	NODE_STMT_DECLARE,
	NODE_STMT_ASSIGN,
	NODE_STMT_OUTPUT,
	NODE_STMT_IF,
	NODE_STMT_WHILE
};

/*
 * Represents a base node.
 */
struct node {
	/* Type of node. */
	enum node_type type;
};

struct node_block {
	struct node base;
	struct vector stmts;
};

struct node_ident {
	struct node base;
	char *val;
};

/*
 * Constant literals nodes definitions.
 */

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
	struct string_object *val;
};

/*
 * Operation nodes definitions.
 */

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

/*
 * Statement nodes definitions.
 */

struct node_stmt_decl {
	struct node base;
	struct node_ident *ident;
	struct node_ident *type_ident;
};

struct node_stmt_assign {
	struct node base;
	struct node_ident *ident;
	struct node *right;
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

void node_free(pseu_vm_t *vm, struct node *node);

#endif /* PSEU_NODE_H */
