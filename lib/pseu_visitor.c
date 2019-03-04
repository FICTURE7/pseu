#include <assert.h>
#include <stdlib.h>

#include "pseu_visitor.h"

/* Generic form of function pointers in a `visitor`. */
typedef void (*visit_fn_t)(struct visitor *visitor, struct node *node);

void visitor_visit(struct visitor *visitor, struct node *node) {
	assert(visitor);

	/* If node is NULL exit early. */
	if (!node) {
		return;
	}

	/* Function to use. */
	visit_fn_t visit_fn = NULL;
	switch (node->type) {
		case NODE_BLOCK:
			visit_fn = (visit_fn_t)visitor->visit_block;
			break;
		case NODE_FUNCTION:
			visit_fn = (visit_fn_t)visitor->visit_function;
			break;

		case NODE_CALL:
			visit_fn = (visit_fn_t)visitor->visit_call;
			break;
		case NODE_IDENT:
			visit_fn = (visit_fn_t)visitor->visit_ident;
			break;
		case NODE_PARAM:
			visit_fn = (visit_fn_t)visitor->visit_param;
			break;

		case NODE_LIT_REAL:
			visit_fn = (visit_fn_t)visitor->visit_real;
			break;
		case NODE_LIT_INTEGER:
			visit_fn = (visit_fn_t)visitor->visit_integer;
			break;
		case NODE_LIT_STRING:
			visit_fn = (visit_fn_t)visitor->visit_string;
			break;
		case NODE_LIT_BOOLEAN:
			visit_fn = (visit_fn_t)visitor->visit_boolean;
			break;

		case NODE_OP_UNARY:
			visit_fn = (visit_fn_t)visitor->visit_op_unary;
			break;
		case NODE_OP_BINARY:
			visit_fn = (visit_fn_t)visitor->visit_op_binary;
			break;

		case NODE_STMT_DECLARE:
			visit_fn = (visit_fn_t)visitor->visit_stmt_decl;
			break;
		case NODE_STMT_ASSIGN:
			visit_fn = (visit_fn_t)visitor->visit_stmt_assign;
			break;
		case NODE_STMT_OUTPUT:
			visit_fn = (visit_fn_t)visitor->visit_stmt_output;
			break;
		case NODE_STMT_IF:
			visit_fn = (visit_fn_t)visitor->visit_stmt_if;
			break;
		case NODE_STMT_WHILE:
			visit_fn = (visit_fn_t)visitor->visit_stmt_while;
			break;
		case NODE_STMT_RETURN:
			visit_fn = (visit_fn_t)visitor->visit_stmt_return;
			break;
	}

	/* Check if not null before executing. */
	if (visit_fn) {
		visit_fn(visitor, node);
	}
}
