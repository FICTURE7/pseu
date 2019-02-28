#include "pseu_vm.h"
#include "pseu_node.h"
#include "pseu_debug.h"
#include "pseu_visitor.h"

static void free_ident(struct visitor *visitor,
				struct node_ident *ident) {
	pseu_vm_t *vm = visitor->data;
	pseu_free(vm, ident->val);
	pseu_free(vm, ident);
}

static void free_boolean(struct visitor *visitor,
				struct node_boolean *boolean) {
	pseu_vm_t *vm = visitor->data;
	pseu_free(vm, boolean);
}

static void free_integer(struct visitor *visitor,
				struct node_integer *integer) {
	pseu_vm_t *vm = visitor->data;
	pseu_free(vm, integer);
}

static void free_real(struct visitor *visitor,
				struct node_real *real) {
	pseu_vm_t *vm = visitor->data;
	pseu_free(vm, real);
}

static void free_string(struct visitor *visitor,
				struct node_string *string) {
	pseu_vm_t *vm = visitor->data;
	pseu_free(vm, string);
	/* NOTE: Not freeing string->val cause the GC should take care of it. */
}

static void free_op_binary(struct visitor *visitor,
				struct node_op_binary *op_binary) {
	pseu_vm_t *vm = visitor->data;
	visitor_visit(visitor, op_binary->left);
	visitor_visit(visitor, op_binary->right);
	pseu_free(vm, op_binary);
}

static void free_op_unary(struct visitor *visitor,
				struct node_op_unary *op_unary) {
	pseu_vm_t *vm = visitor->data;
	visitor_visit(visitor, op_unary->expr);
	pseu_free(vm, op_unary);
}

static void free_stmt_output(struct visitor *visitor,
				struct node_stmt_output *stmt_output) {
	pseu_vm_t *vm = visitor->data;
	visitor_visit(visitor, stmt_output->expr);
	pseu_free(vm, stmt_output);
}

static void free_stmt_decl(struct visitor *visitor,
				struct node_stmt_decl *stmt_decl) {
	pseu_vm_t *vm = visitor->data;
	free_ident(visitor, stmt_decl->ident);
	free_ident(visitor, stmt_decl->type_ident);
	pseu_free(vm, stmt_decl);
}

static void free_block(struct visitor *visitor,
				struct node_block *block) {
	pseu_vm_t *vm = visitor->data;
	for (size_t i = 0; i < block->stmts.count; i++) {
		visitor_visit(visitor, block->stmts.data[i]);
	}
	pseu_free(vm, block);
}

void node_free(pseu_vm_t *vm, struct node *node) {
	pseu_assert(vm && node);

	struct visitor visitor = { };
	visitor.data = vm;
	visitor.visit_ident = free_ident;
	visitor.visit_boolean = free_boolean;
	visitor.visit_integer = free_integer;
	visitor.visit_real = free_real;
	visitor.visit_string = free_string;
	visitor.visit_op_binary = free_op_binary;
	visitor.visit_op_unary = free_op_unary;
	visitor.visit_stmt_output = free_stmt_output;
	visitor.visit_stmt_decl = free_stmt_decl;
	visitor.visit_block = free_block;
	visitor_visit(&visitor, node);
}
