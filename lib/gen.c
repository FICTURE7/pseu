#include <stdlib.h>
#include "vm.h"
#include "visitor.h"
#include "vector.h"
#include "node.h"

static void emit(struct vm_ir *ir, int inst) {
	vector_add(&ir->instructions, (void *)inst);
}

static void emit_op(struct vm_ir *ir, enum op_type op) {
	switch (op) {
		case OP_ADD:
			emit(ir, VM_INST_ADD);
			break;
		case OP_SUB:
			emit(ir, VM_INST_SUB);
			break;
		case OP_MUL:
			emit(ir, VM_INST_MUL);
			break;
		case OP_DIV:
			emit(ir, VM_INST_DIV);
			break;
	}
}

static void gen_block(struct visitor *visitor, struct node_block *block) {
	for (unsigned int i = 0; i < block->stmts.count; i++) {
		visitor_visit(visitor, vector_get(&block->stmts, i));
	}
}

static void gen_boolean(struct visitor *visitor, struct node_boolean *boolean) {
	// emit val
}

static void gen_integer(struct visitor *visitor, struct node_integer *integer) {
	emit(visitor->data, VM_INST_PUSH);
	emit(visitor->data, integer->val);
}

static void gen_real(struct visitor *visitor, struct node_real *real) {
	// emit val
}

static void gen_string(struct visitor *visitor, struct node_string *string) {
	emit(visitor->data, VM_INST_PUSH);
	emit(visitor->data, string->val);
}

static void gen_op_unary(struct visitor *visitor, struct node_op_unary *unary) {
	visitor_visit(visitor, unary->expr);
	emit(visitor->data, 0);
	emit_op(visitor->data, unary->op);
}

static void gen_op_binary(struct visitor *visitor, struct node_op_binary *binary) {
	visitor_visit(visitor, binary->left);
	visitor_visit(visitor, binary->right);
	emit_op(visitor->data, binary->op);
}

static void gen_stmt_output(struct visitor *visitor, struct node_stmt_output *output) {
	visitor_visit(visitor, output->expr);
	emit(visitor->data, VM_INST_OUTPUT);
}

struct vm_ir *vm_gen(struct vm *vm, struct node *node) {
	struct vm_ir *ir = malloc(sizeof(struct vm_ir));
	struct visitor visitor;

	vector_init(&ir->instructions);
	visitor.data = ir;
	visitor.visit_block = gen_block;
	visitor.visit_boolean = gen_boolean;
	visitor.visit_integer = gen_integer;
	visitor.visit_real = gen_real;
	visitor.visit_string = gen_string;
	visitor.visit_op_unary = gen_op_unary;
	visitor.visit_op_binary = gen_op_binary;
	visitor.visit_stmt_output = gen_stmt_output;
	visitor_visit(&visitor, node);

	return ir;
}
