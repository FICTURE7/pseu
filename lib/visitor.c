#include <stdlib.h>
#include "visitor.h"

void visitor_visit(struct visitor *visitor, struct node *node) {
	switch (node->type) {
		case NODE_BLOCK:
			visitor->visit_block(visitor, (struct node_block *)node);
			break;

		case NODE_LIT_REAL:
			visitor->visit_real(visitor, (struct node_real *)node);
			break;
		case NODE_LIT_INTEGER:
			visitor->visit_integer(visitor, (struct node_integer *)node);
			break;
		case NODE_LIT_STRING:
			visitor->visit_string(visitor, (struct node_string *)node);
			break;
		case NODE_LIT_BOOLEAN:
			visitor->visit_boolean(visitor, (struct node_boolean *)node);
			break;

		case NODE_OP_UNARY:
			visitor->visit_op_unary(visitor, (struct node_op_unary *)node);
			break;
		case NODE_OP_BINARY:
			visitor->visit_op_binary(visitor, (struct node_op_binary *)node);
			break;

		case NODE_STMT_DECLARE:
			visitor->visit_stmt_decl(visitor, (struct node_stmt_decl *)node);
			break;
		case NODE_STMT_OUTPUT:
			visitor->visit_stmt_output(visitor, (struct node_stmt_output *)node);
			break;
	}
}
