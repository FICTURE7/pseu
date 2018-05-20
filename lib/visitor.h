#ifndef VISITOR_H
#define VISITOR_H

#include "node.h"
#include "object.h"

struct visitor {
	struct object *(*block)(struct visitor *visitor, struct node_block *block);
	struct object *(*integer)(struct visitor *visitor, struct node_integer *integer);
	struct object *(*string)(struct visitor *visitor, struct node_string *string);
	struct object *(*boolean)(struct visitor *visitor, struct node_boolean *boolean);
	struct object *(*op_unary)(struct visitor *visitor, struct node_op_unary *op_unary);
	struct object *(*op_binary)(struct visitor *visitor, struct node_op_binary *op_binary);
	struct object *(*stmt_decl)(struct visitor *visitor, struct node_stmt_decl *stmt_decl);
	struct object *(*stmt_output)(struct visitor *visitor, struct node_stmt_output *stmt_output);
};

struct object *visitor_visit(struct visitor *visitor, struct node *node);

#endif
