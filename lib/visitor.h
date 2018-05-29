#ifndef VISITOR_H
#define VISITOR_H

#include "node.h"
#include "object.h"

struct visitor {
	void(*block)(struct visitor *visitor, struct node_block *block);
	void(*real)(struct visitor *visitor, struct node_real *real);
	void(*integer)(struct visitor *visitor, struct node_integer *integer);
	void(*string)(struct visitor *visitor, struct node_string *string);
	void(*boolean)(struct visitor *visitor, struct node_boolean *boolean);
	void(*op_unary)(struct visitor *visitor, struct node_op_unary *op_unary);
	void(*op_binary)(struct visitor *visitor, struct node_op_binary *op_binary);
	void(*stmt_decl)(struct visitor *visitor, struct node_stmt_decl *stmt_decl);
	void(*stmt_output)(struct visitor *visitor, struct node_stmt_output *stmt_output);
};

void visitor_visit(struct visitor *visitor, struct node *node);

#endif
