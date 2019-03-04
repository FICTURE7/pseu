#ifndef PSEU_VISITOR_H
#define PSEU_VISITOR_H

#include "pseu_node.h"

/* 
 * Represents a node visitor which tranverses nodes/AST.
 */
struct visitor {
	/* Pointer to user data. */
	void *data;

	void (*visit_call)(struct visitor *visitor,
			struct node_call *call);
	void (*visit_ident)(struct visitor *visitor,
			struct node_ident *ident);
	void (*visit_param)(struct visitor *visitor,
			struct node_param *param);

	void (*visit_boolean)(struct visitor *visitor,
			struct node_boolean *boolean);
	void (*visit_integer)(struct visitor *visitor,
			struct node_integer *integer);
	void (*visit_real)(struct visitor *visitor,
			struct node_real *real);
	void (*visit_string)(struct visitor *visitor,
			struct node_string *string);

	void (*visit_op_unary)(struct visitor *visitor,
			struct node_op_unary *op_unary);
	void (*visit_op_binary)(struct visitor *visitor,
			struct node_op_binary *op_binary);
	
	void (*visit_stmt_decl)(struct visitor *visitor,
			struct node_stmt_decl *stmt_decl);
	void (*visit_stmt_assign)(struct visitor *visitor,
			struct node_stmt_assign *stmt_assign);
	void (*visit_stmt_output)(struct visitor *visitor,
			struct node_stmt_output *stmt_output);
	void (*visit_stmt_if)(struct visitor *visitor,
			struct node_stmt_if *stmt_if);
	void (*visit_stmt_while)(struct visitor *visitor,
			struct node_stmt_while *stmt_while);
	void (*visit_stmt_return)(struct visitor *visitor,
			struct node_stmt_return *stmt_return);

	void (*visit_block)(struct visitor *visitor,
			struct node_block *block);
	void (*visit_function)(struct visitor *visitor,
			struct node_function *fn);
};

/*
 * Visits the specified node using the appropriate visit function in the 
 * visitor.
 *
 * @param visitor Visitor to use.
 * @param node Node to visit.
 */
void visitor_visit(struct visitor *visitor, struct node *node);

#endif /* PSEU_VISITOR_H */
