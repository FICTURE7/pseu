#include <string.h>
#include <stdint.h>

#include "pseu_vm.h"
#include "pseu_node.h"
#include "pseu_value.h"
#include "pseu_debug.h"
#include "pseu_opcode.h"
#include "pseu_symbol.h"
#include "pseu_visitor.h"
#include "pseu_compiler.h"

static void error(struct compiler *compiler, const char *format, ...) {
	compiler->error_count++;

	/* TODO: Format error and pass to compiler->state->vm->config.onerror. */
}

/* Defines a constant value in the specified closure. */
static int declare_const(struct compiler *compiler, struct value *value) {
	if (compiler->consts_count >= MAX_CONSTS) {
		return 1;
	}
	
	compiler->consts[compiler->consts_count++] = *value;
	return 0;
}

/* Defines a local variable in the specified closure. */
static int declare_local(struct compiler *compiler, struct variable *var) {
	/* TODO: Implement scoping and check if local had already been declared. */
	if (compiler->locals_count >= MAX_LOCALS) {
		return 1;
	}

	compiler->locals[compiler->locals_count++] = *var;
	return 0;
}

static int declare_global(struct compiler *compiler, struct variable *var) {
	return 1;
}

static int resolve_variable(struct compiler *compiler, const char *ident) {
	for (size_t i = 0; i < compiler->locals_count; i++) {
		struct variable *local = &compiler->locals[i];
		if (!strcmp(local->ident, ident)) {
			return i;
		}
	}

	return -1;
}

/* 
 * Default capacity of when an emitter is initialized with emitter_init.
 */
#define EMITTER_INIT_SIZE 64

static void emitter_init(struct compiler *compiler) {
	pseu_assert(compiler);
	
	struct emitter *emitter = &compiler->emitter;
	emitter->count = 0;
	emitter->size = EMITTER_INIT_SIZE;
	emitter->code = pseu_alloc(compiler->state->vm,
				sizeof(code_t) * emitter->size);
}

static void emit_u8(struct compiler *compiler, uint8_t value) {
	struct emitter *emitter = &compiler->emitter;

	/* Grow emitter buffer if needed. */
	if (emitter->count >= emitter->size) {
		size_t new_size = emitter->size * 2;
		code_t *code = pseu_realloc(compiler->state->vm,
				emitter->code, new_size);
		emitter->size = new_size;
		emitter->code = code;
	}

	emitter->code[emitter->count++] = value;	
}

static void emit_u16(struct compiler *compiler, uint16_t value) {
	/* NOTE: Room for optimization here. */
	emit_u8(compiler, (value >> 8) && 0xFF);
	emit_u8(compiler, value & 0xFF);
}

static void emit_call(struct compiler *compiler, const char *ident) {
	/* Look up function in symbol table. */
	int id = symbol_table_get_function(&compiler->state->vm->symbols, ident);
	if (id == -1) {
		/* TODO: Error, function not defined. */
		return;
	}

	emit_u8(compiler, VM_OP_CALL);
	emit_u16(compiler, id);
}

static int emit_ld_const(struct compiler *compiler,	struct value *value) { 
	/* TODO: Could save some slots by checking for constants with same value. */
	if (declare_const(compiler, value)) {
		error(compiler, "Exceeded maximum number of constants in a closure");
		return 1;
	}
	
	emit_u8(compiler, VM_OP_LD_CONST);
	emit_u8(compiler, compiler->consts_count - 1);
	compiler->stack_size++;
	return 0;
}

static void gen_ident(struct visitor *visitor,
				struct node_ident *ident) {
	struct compiler *compiler = visitor->data;

	/* 
	 * Look for the local with the specified identifier name in the list of
	 * locals.
	 */
	int index = resolve_variable(compiler, ident->val);
	if (index == -1) {
		/* TODO: Look for global variables. */
		/* TODO: Set error or create local with type VOID. */
		return;
	}

	emit_u8(compiler, VM_OP_LD_LOCAL);
	emit_u8(compiler, index);
	compiler->stack_size++;
}

static void gen_boolean(struct visitor *visitor,
				struct node_boolean *boolean) {
	/* TODO: Optimize the VM bytecode to save some slots in the closure consts. */
	struct compiler *compiler = visitor->data;
	struct value value = {
		.type = VALUE_TYPE_BOOLEAN,
		.as_bool = boolean->val
	};

	emit_ld_const(compiler, &value);
}

static void gen_integer(struct visitor *visitor,
				struct node_integer *integer) {
	struct compiler *compiler = visitor->data;
	struct value value = {
		.type = VALUE_TYPE_INTEGER,
		.as_int = integer->val
	};

	emit_ld_const(compiler, &value);
}

static void gen_real(struct visitor *visitor,
				struct node_real *real) {
	struct compiler *compiler = visitor->data;
	struct value value = {
		.type = VALUE_TYPE_REAL,
		.as_float = real->val
	};

	emit_ld_const(compiler, &value);
}

/* 
 * Array mapping `enum op_type` to their primitive function identifiers.
 *
 * NOTE: Could probably map `enum op_type` to their primitive function symbol
 * index instead to gain some perf.
 */
const char *op_binary_fn_idents[] = {
	"@add",
	"@sub",
	"@mul",
	"@div"
};

static void gen_op_binary(struct visitor *visitor,
				struct node_op_binary *op_binary) {
	/* Generate code for right operand and then left operand. */
	visitor_visit(visitor, op_binary->right);
	visitor_visit(visitor, op_binary->left);

	const char *fn_ident = op_binary_fn_idents[op_binary->op - OP_ADD];

	/* Generate code for coresponding operation primitive function. */
	struct compiler *compiler = visitor->data;
	emit_call(compiler, fn_ident);
}

static void gen_op_unary(struct visitor *visitor,
			struct node_op_unary *op_unary) {
	/* Generate code for expression. */
	visitor_visit(visitor, op_unary->expr);

	/* If minus operator, call @neg primitive function. */
	if (op_unary->op == OP_SUB) {
		struct compiler *compiler = visitor->data;
		emit_call(compiler, "@neg");
	}
}

static void gen_stmt_decl(struct visitor *visitor,
				struct node_stmt_decl *decl) {
	/* TODO: Detect if global variable. */

	struct compiler *compiler = visitor->data;
	int type_id = symbol_table_get_type(&compiler->state->vm->symbols,
					decl->type_ident->val);
	if (type_id == -1) {
		/* TODO: Register to list where this could be resolved. */
		return;
	}

	/* 
	 * Clone the node_stmt_decl's ident ident_node, so we can free the nodes
	 * and still work the var->ident.
	 */
	size_t ident_len = strlen(decl->ident->val);
	char *ident = pseu_alloc(compiler->state->vm, ident_len + 1);
	strcpy(ident, decl->ident->val);

	struct variable var = {
		.ident = ident,
		.type = compiler->state->vm->symbols.types.data[type_id]
	};

	if (declare_local(compiler, &var)) {
		error(compiler, "Exceeded maximum number of locals in a closure");
	}
}

static void gen_stmt_assign(struct visitor *visitor,
				struct node_stmt_assign *assign) {
	struct compiler *compiler = visitor->data;

	/* Generate code for expression with the variable will be assigned to. */
	visitor_visit(visitor, assign->right);

	int index = resolve_variable(compiler, assign->ident->val);
	if (index == -1) {
		return;
	}

	emit_u8(compiler, VM_OP_ST_LOCAL);
	emit_u8(compiler, index);
}

static void gen_stmt_output(struct visitor *visitor,
				struct node_stmt_output *output) {
	/* Generate code for expression. */
	visitor_visit(visitor, output->expr);

	/* Generate code for call to primitive OUTPUT function. */
	struct compiler *compiler = visitor->data;
	emit_call(compiler, "@output");
}

static void gen_block(struct visitor *visitor,
				struct node_block *block) {
	/* Generate code for each statement in block. */
	for (size_t i = 0; i < block->stmts.count; i++) {
		visitor_visit(visitor, block->stmts.data[i]);
	}
}

int compiler_compile(struct state *state, struct compiler *compiler,
				struct function *fn, const char *src) {
	pseu_assert(state && compiler && fn && src);

	compiler->state = state;
	compiler->fn = fn;
	compiler->top = NULL;
	compiler->stack_size = 0;
	compiler->consts_count = 0;
	compiler->locals_count = 0;
	compiler->error_count = 0;

	lexer_init(&compiler->lexer, src);
	parser_init(&compiler->parser, &compiler->lexer, state);
	emitter_init(compiler);

	/* Parse source code into abstract syntax tree. */
	struct node *node = parser_parse(&compiler->parser);
	/* Check if parse failed. */
	if (compiler->parser.error_count > 0) {
		node_free(state->vm, node);
		return 1;
	}

#ifdef PSEU_DEBUG_AST
	pseu_dump_node(stdout, node);
#endif

	/* 
	 * Initialize visitor that will do the code generation with the appropriate
	 * code gen functions.
	 */
	struct visitor visitor = { };
	visitor.data = compiler;
	visitor.visit_ident = gen_ident;
	visitor.visit_boolean = gen_boolean;
	visitor.visit_integer = gen_integer;
	visitor.visit_real = gen_real;
	visitor.visit_op_binary = gen_op_binary;
	visitor.visit_op_unary = gen_op_unary;
	visitor.visit_stmt_decl = gen_stmt_decl;
	visitor.visit_stmt_assign = gen_stmt_assign;
	visitor.visit_stmt_output = gen_stmt_output;
	visitor.visit_block = gen_block;
	visitor_visit(&visitor, node);
	/* Free node once code gen is done. */
	node_free(state->vm, node);

	/* Check if code gen failed. */
	if (compiler->error_count > 0) {
		return 1;
	}

	/* Finalize block. */
	emit_u8(compiler, VM_OP_END);

	/* Allocate and initialize closure. */
	struct closure *closure = pseu_alloc(state->vm, sizeof(struct closure));
	closure->fn = fn;
	closure->locals_count = compiler->locals_count;
	closure->locals = pseu_alloc(state->vm,
						sizeof(struct variable) * compiler->locals_count);

	closure->consts_count = compiler->consts_count;
	closure->consts = pseu_alloc(state->vm,
						sizeof(struct value) * compiler->consts_count);

	closure->stack_size = compiler->stack_size;
	closure->code = compiler->emitter.code;

	memcpy(closure->locals, compiler->locals, 
						sizeof(struct variable) * compiler->locals_count);
	memcpy(closure->consts, compiler->consts, 
						sizeof(struct value) * compiler->consts_count);

	/* Set function. */
	fn->type = FN_TYPE_USER;
	fn->ident = NULL;
	fn->params_count = 0;
	fn->params_types = NULL;
	fn->return_type = NULL;
	fn->as_closure = closure;

#ifdef PSEU_DEBUG_CODE
	pseu_dump_function_info(stdout, fn);
	pseu_dump_function_code(stdout, state->vm, fn);
#endif
	return 0;
}
