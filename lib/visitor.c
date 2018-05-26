#include <stdlib.h>
#include "visitor.h"

struct object *visitor_visit(struct visitor *visitor, struct node *node) {
	switch (node->type) {
		case NODE_BLOCK:
			return visitor->block(visitor, (struct node_block *)node);

		case NODE_LIT_REAL:
			return visitor->real(visitor, (struct node_real *)node);
		case NODE_LIT_INTEGER:
			return visitor->integer(visitor, (struct node_integer *)node);
		case NODE_LIT_STRING:
			return visitor->string(visitor, (struct node_string *)node);
		case NODE_LIT_BOOLEAN:
			return visitor->boolean(visitor, (struct node_boolean *)node);

		case NODE_OP_UNARY:
			return visitor->op_unary(visitor, (struct node_op_unary *)node);
		case NODE_OP_BINARY:
			return visitor->op_binary(visitor, (struct node_op_binary *)node);

		case NODE_STMT_DECLARE:
			return visitor->stmt_decl(visitor, (struct node_stmt_decl *)node);
		case NODE_STMT_OUTPUT:
			return visitor->stmt_output(visitor, (struct node_stmt_output *)node);
	}
	return NULL;
}
