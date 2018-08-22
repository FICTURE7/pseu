#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "func.h"
#include "value.h"
#include "lexer.h"
#include "utils.h"
#include "parser.h"
#include "visitor.h"
#include "compiler.h"

static inline void emitter_init(struct emitter *emitter) {
	emitter->count = 0;
	emitter->capacity = 32;
	emitter->code = malloc(sizeof(instr_t) * emitter->capacity);
}

static void emit(struct emitter *emitter, instr_t instr) {
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

static inline void emit_push(struct emitter *emitter, uint8_t index) {
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
	emit(emitter, op - OP_ADD + 1); /* TODO: fix this stuff */
}

static inline int define_const(struct compiler *compiler, struct value val) {
	if (compiler->nconsts >= MAX_CONSTS) {
		/* TODO: error */
		return -1;
	}
	
	compiler->consts[compiler->nconsts] = val;
	return ++compiler->nconsts;
}

static inline int declare_local(struct compiler *compiler, struct variable *var) {
	if (compiler->nlocals >= MAX_LOCALS) {
		/* TODO: error */
		return -1;
	}
	
	compiler->locals[compiler->nlocals] = *var;
	return ++compiler->nlocals;
}

static inline int declare_global(struct compiler *compiler, struct variable *var) {
	return symbol_table_add(compiler->state->symbols, NULL);
}

static inline int declare_func(struct compiler *compiler, struct func *fn) {
	return -1;
}

static void gen_block(struct visitor *visitor, struct node_block *block) {
	for (size_t i = 0; i < block->stmts.count; i++) {
		visitor_visit(visitor, block->stmts.items[i]);
	}
}

static void gen_ident(struct visitor *visitor, struct node_ident *ident) {
	/* TODO: look up in table or something */
	struct compiler *compiler = visitor->data;
	bool found = false;

	for (size_t i = 0; i < compiler->nlocals; i++) {
		if (!strcmp(compiler->locals[i].ident, ident->val)) {
			found = true;
			break;
		}
	}

	if (!found) {
		
	}
}

static void gen_boolean(struct visitor *visitor, struct node_boolean *boolean) {	
	struct compiler *compiler = visitor->data;

	/* add boolean to list of constants */
	compiler->consts[compiler->nconsts] = (struct value) {
		.type = VALUE_TYPE_BOOLEAN,
		.as_bool = boolean->val
	};

	emit_push(&compiler->emitter, compiler->nconsts++);
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

static void gen_real(struct visitor *visitor, struct node_real *real) {
	struct compiler *compiler = visitor->data;

	/* add real to list of constants */
	compiler->consts[compiler->nconsts] = (struct value) {
		.type = VALUE_TYPE_REAL,
		.as_float = real->val
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
			ASSERT(false, "expected either OP_ADD or OP_SUB");
			break;
	}
}

static void gen_op_binary(struct visitor *visitor, struct node_op_binary *op_binary) {
	struct compiler *compiler = visitor->data;
	visitor_visit(visitor, op_binary->left);
	visitor_visit(visitor, op_binary->right);
	emit_op(&compiler->emitter, op_binary->op);
}

static void gen_stmt_decl(struct visitor *visitor, struct node_stmt_decl *decl) {
	struct compiler *compiler = visitor->data;
	struct variable var = {
		.ident = decl->ident->val
	};

	/* look for the type of the variable */
	if (!strcmp(decl->type->val, "INTEGER")) {
		
	} else {
		/* look up in type table */
	}

	/* add new variable to array of locals */
	declare_local(compiler, &var);
	/* add new variable to symbol table if top level compiler */
	if (compiler->top == NULL) {
		declare_global(compiler, &var);
	}
}

static void gen_stmt_output(struct visitor *visitor, struct node_stmt_output *output) {
	struct compiler *compiler = visitor->data;

	/* emit the expression first */
	visitor_visit(visitor, output->expr);
	emit_output(&compiler->emitter);
}

void compiler_init(struct compiler *compiler, struct state *state) {
	compiler->state = state;
	compiler->top = NULL; /* set NULL to indicate top level compiler */

	compiler->nlocals = 0;
	compiler->nconsts = 0;
	compiler->proto = calloc(sizeof(struct proto), 1);
	compiler->proto->rett = state->void_type;

	compiler->fn = calloc(sizeof(struct func), 1);
	compiler->fn->proto = compiler->proto;
}

struct func *compiler_compile(struct compiler *compiler, const char *src) {
	struct node *root; /* root syntax tree */
	struct visitor visitor; /* visitor to transverse the syntax tree */

	/* make sure we got stuff allocated */
	if (compiler->proto == NULL || compiler->fn == NULL) {
		return NULL;
	}

	/* initialize the lexer & the parser */
	lexer_init(&compiler->lexer, src);
	parser_init(&compiler->parser, &compiler->lexer, compiler->state);

	/* parse the src into a syntax tree */
	root = parser_parse(&compiler->parser);
	/* check if parser failed to parse the code */
	if (root == NULL) {
		/* TODO: clean up resources */
		/* TODO: pass errors to pseu->config.onerror*/
		return NULL;
	}

	/* initialize the code emitter */
	emitter_init(&compiler->emitter);

	/* initialize the visitor */
	visitor.data = compiler;
	visitor.visit_block = gen_block;
	visitor.visit_ident = gen_ident;
	visitor.visit_integer = gen_integer;
	visitor.visit_real = gen_real;
	visitor.visit_string = gen_string;
	visitor.visit_boolean = gen_boolean;
	visitor.visit_op_unary = gen_op_unary;
	visitor.visit_op_binary = gen_op_binary;
	visitor.visit_stmt_decl = gen_stmt_decl;
	visitor.visit_stmt_output = gen_stmt_output;
	visitor_visit(&visitor, root);

	/* emit end of func */
	emit_halt(&compiler->emitter);

	/* set the function's consts */
	compiler->fn->nconsts = compiler->nconsts;
	compiler->fn->consts = malloc(sizeof(struct value) * compiler->nconsts);
	memcpy(compiler->fn->consts, compiler->consts, sizeof(struct value) * compiler->nconsts);

	/* set the function's code */
	compiler->fn->ncode = compiler->emitter.count;
	compiler->fn->code = realloc(compiler->emitter.code, sizeof(instr_t) * compiler->fn->ncode); /* trim excess */

	/* TODO: free `root` node */
	return compiler->fn;
}
