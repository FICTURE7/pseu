#include <string.h>
#include <stdlib.h>
#include "func.h"
#include "lexer.h"
#include "parser.h"
#include "visitor.h"
#include "compiler.h"

static inline void emitter_init(struct emitter *emitter) {
	emitter->count = 0;
	emitter->capacity = 32;
	emitter->code = malloc(sizeof(instr_t) * emitter->capacity);
}

static inline void emit(struct emitter *emitter, instr_t instr) {
	/* grow emitter if needed */
	if (emitter->count >= emitter->capacity) {
		emitter->capacity += 32;
		emitter->code = realloc(emitter->code, emitter->capacity);
	}

	emitter->code[emitter->count++] = instr;
}

static inline void emit_halt(struct emitter *emitter) {
	emit(emitter, VM_OP_HALT);
}

static inline void emit_push(struct emitter *emitter, unsigned int index) {
	emit(emitter, VM_OP_PUSH);
	emit(emitter, index);
}

static inline void emit_pop(struct emitter *emitter) {
	emit(emitter, VM_OP_POP);
}

static inline void emit_output(struct emitter *emitter) {
	emit(emitter, VM_OP_OUTPUT);
}

static inline void emit_op(struct emitter *emitter, enum op_type op) {
	emit(emitter, op - OP_ADD + 1);
}

static void gen_block(struct visitor *visitor, struct node_block *block) {
	for (size_t i = 0; i < block->stmts.count; i++) {
		visitor_visit(visitor, block->stmts.items[i]);
	}
}

static void gen_integer(struct visitor *visitor, struct node_integer *integer) {	
	struct compiler *compiler = visitor->data;

	/* add string to list of constants */
	compiler->consts[compiler->nconsts] = (struct value) {
		.type = VALUE_TYPE_INTEGER,
		.as_int = integer->val
	};

	emit_push(&compiler->emitter, compiler->nconsts++);
}

static void gen_string(struct visitor *visitor, struct node_string *string) {
	struct compiler *compiler = visitor->data;

	/* add string to list of constants */
	compiler->consts[compiler->nconsts] = (struct value) {
		.type = VALUE_TYPE_OBJECT,
		.as_object = (struct object *)string->val
	};

	emit_push(&compiler->emitter, compiler->nconsts++);
}

static void gen_op_unary(struct visitor *visitor, struct node_op_unary *op_unary) {
	struct compiler *compiler = visitor->data;
	switch (op_unary->op) {
		case OP_ADD:
			visitor_visit(visitor, op_unary->expr);
			break;
		case OP_SUB:
			visitor_visit(visitor, op_unary->expr);
			emit_op(&compiler->emitter, OP_SUB);
			break;

		default:
			/* error */
			break;
	}
}

static void gen_op_binary(struct visitor *visitor, struct node_op_binary *op_binary) {
	struct compiler *compiler = visitor->data;
	visitor_visit(visitor, op_binary->left);
	visitor_visit(visitor, op_binary->right);
	emit_op(&compiler->emitter, op_binary->op);
}

static void gen_stmt_output(struct visitor *visitor, struct node_stmt_output *output) {
	struct compiler *compiler = visitor->data;

	/* emit the expression first */
	visitor_visit(visitor, output->expr);
	emit_output(&compiler->emitter);
}

void compiler_init(struct compiler *compiler, struct state *state) {
	compiler->state = state;
}

struct func *compiler_compile(struct compiler *compiler, char *src) {
	struct visitor visitor; /* visitor which will transverse syntax tree */
	struct proto *proto; /* prototype of the function its compiling */
	struct node *root; /* root node/syntax tree */
	struct func *fn; /* function its compiling */

	proto = malloc(sizeof(struct proto));
	fn = malloc(sizeof(struct func));

	/* make sure we got stuff allocated */
	if (proto == NULL || fn == NULL) {
		return NULL;
	}

	/* initialize the lexer & the parser */
	lexer_init(&compiler->lexer, src);
	parser_init(&compiler->parser, &compiler->lexer, compiler->state);

	/* parse the src into a syntax tree */
	parser_parse(&compiler->parser, &root);
	/* check if parser failed to parse the code */
	if (root == NULL) {
		/* TODO: pass errors to pseu->config.onerror*/
		return NULL;
	}

	/* initialize the code emitter */
	emitter_init(&compiler->emitter);

	visitor.data = &compiler;
	visitor.visit_block = gen_block;
	visitor.visit_integer = gen_integer;
	visitor.visit_string = gen_string;
	visitor.visit_op_unary = gen_op_unary;
	visitor.visit_op_binary = gen_op_binary;
	visitor.visit_stmt_output = gen_stmt_output;
	visitor_visit(&visitor, root);

	/* emit end of func */
	emit_halt(&compiler->emitter);

	/* set the fn's prototype */
	fn->proto = proto;
	/* set the function's consts */
	fn->nconsts = compiler->nconsts;
	fn->consts = malloc(sizeof(struct value) * compiler->nconsts);
	memcpy(fn->consts, compiler->consts, sizeof(struct value) * compiler->nconsts);

	/* set the function's code */
	fn->ncode = compiler->emitter.count;
	fn->code = realloc(compiler->emitter.code, sizeof(instr_t) * fn->ncode); /* trim excess */
	return fn;
}
