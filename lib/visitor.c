#include <stdlib.h>
#include "visitor.h"

void visitor_visit(struct visitor *visitor, struct node *node) {
	switch (node->type) {
		case NODE_BLOCK:
			visitor->block(visitor, (struct node_block *)node);
			break;

		case NODE_LIT_REAL:
			visitor->real(visitor, (struct node_real *)node);
			break;
		case NODE_LIT_INTEGER:
			visitor->integer(visitor, (struct node_integer *)node);
			break;
		case NODE_LIT_STRING:
			visitor->string(visitor, (struct node_string *)node);
			break;
		case NODE_LIT_BOOLEAN:
			visitor->boolean(visitor, (struct node_boolean *)node);
			break;

		case NODE_OP_UNARY:
			visitor->op_unary(visitor, (struct node_op_unary *)node);
			break;
		case NODE_OP_BINARY:
			visitor->op_binary(visitor, (struct node_op_binary *)node);
			break;

		case NODE_STMT_DECLARE:
			visitor->stmt_decl(visitor, (struct node_stmt_decl *)node);
			break;
		case NODE_STMT_OUTPUT:
			visitor->stmt_output(visitor, (struct node_stmt_output *)node);
			break;
	}
}
