#ifndef VISITOR_H
#define VISITOR_H

#include "node.h"
#include "object.h"

struct visitor {
	void *data;
	void(*visit_block)(struct visitor *visitor, struct node_block *block);
	void(*visit_real)(struct visitor *visitor, struct node_real *real);
	void(*visit_integer)(struct visitor *visitor, struct node_integer *integer);
	void(*visit_string)(struct visitor *visitor, struct node_string *string);
	void(*visit_boolean)(struct visitor *visitor, struct node_boolean *boolean);
	void(*visit_op_unary)(struct visitor *visitor, struct node_op_unary *op_unary);
	void(*visit_op_binary)(struct visitor *visitor, struct node_op_binary *op_binary);
	void(*visit_stmt_decl)(struct visitor *visitor, struct node_stmt_decl *stmt_decl);
	void(*visit_stmt_output)(struct visitor *visitor, struct node_stmt_output *stmt_output);
};

void visitor_visit(struct visitor *visitor, struct node *node);

#endif
