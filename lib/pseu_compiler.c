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

static void compiler_init(struct state *state, struct compiler *compiler,
				struct compiler *top_compiler, struct function *fn);
static void compiler_set_visitor(struct compiler *compiler,
				struct visitor *visitor);
static struct closure *compiler_compile_closure(struct compiler *compiler,
				struct node_block *node);

static const char *strclone(pseu_vm_t *vm, const char *str) {
	size_t len = strlen(str);
	char *clone = pseu_alloc(vm, len + 1);
	strcpy(clone, str);

	return clone;
}

static void error(struct compiler *compiler, const char *format, ...) {
	compiler->error_count++;

	/* TODO: Format error and pass to compiler->state->vm->config.onerror. */
}

static int resolve_local(struct compiler *compiler, const char *ident) {
	/* TODO: Scope check. */

	for (size_t i = 0; i < compiler->locals_count; i++) {
		struct variable *local = &compiler->locals[i];
		if (!strcmp(local->ident, ident)) {
			return i;
		}
	}

	return -1;
}

static struct type *resolve_type(struct compiler *compiler, const char *ident) {
	pseu_assert(compiler && ident);

	struct symbol_table *symbols = &compiler->state->vm->symbols;
	int type_id = symbol_table_get_type(symbols, ident);

	/* Check if type was found in the symbol table. */
	if (type_id == -1) {
		return NULL;
	}

	return symbols->types.data[type_id];
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
static struct variable *declare_local(struct compiler *compiler,
				const char *ident, const char *type_ident) {
	/* Check if variable has already been declared. */
	if (resolve_local(compiler, ident) != -1) {
		error(compiler, "Already declared variable with same identifier");
		return NULL;
	}

	/* Check if exceeded maximum number of local variables. */
	if (compiler->locals_count >= MAX_LOCALS) {
		/* TODO: Prevent spamming of this error message. */
		error(compiler, "Exceeded maximum number of variables in a closure");
		return NULL;
	}

	struct type *type = resolve_type(compiler, type_ident);
	if (!type) {
		return NULL;
	}

	/* Clone ident, so it can be freed straight away. */
	const char *ident_clone = strclone(compiler->state->vm, ident);
	struct variable var = {
		.ident = ident_clone,
		.type = type
	};

	struct variable *ret = &compiler->locals[compiler->locals_count++];

	*ret = var;
	return ret;
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

static void gen_call(struct visitor *visitor,
				struct node_call *node) {
	struct compiler *compiler = visitor->data;
	
	/* Generate code for call arguments. */
	for (size_t i = 0; i < node->args.count; i++) {
		visitor_visit(visitor, node->args.data[i]);
	}

	emit_call(compiler, node->fn_ident->val);
}

static void gen_ident(struct visitor *visitor,
				struct node_ident *node) {
	struct compiler *compiler = visitor->data;

	/* 
	 * Look for the local with the specified identifier name in the list of
	 * locals.
	 */
	int index = resolve_local(compiler, node->val);
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
				struct node_boolean *node) {
	/* TODO: Optimize the VM bytecode to save some slots in the closure consts. */
	struct compiler *compiler = visitor->data;
	struct value value = {
		.type = VALUE_TYPE_BOOLEAN,
		.as_bool = node->val
	};

	emit_ld_const(compiler, &value);
}

static void gen_integer(struct visitor *visitor,
				struct node_integer *node) {
	struct compiler *compiler = visitor->data;
	struct value value = {
		.type = VALUE_TYPE_INTEGER,
		.as_int = node->val
	};

	emit_ld_const(compiler, &value);
}

static void gen_real(struct visitor *visitor,
				struct node_real *node) {
	struct compiler *compiler = visitor->data;
	struct value value = {
		.type = VALUE_TYPE_REAL,
		.as_float = node->val
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
				struct node_op_binary *node) {
	/* Generate code for right operand and then left operand. */
	visitor_visit(visitor, node->right);
	visitor_visit(visitor, node->left);

	const char *fn_ident = op_binary_fn_idents[node->op - OP_ADD];

	/* Generate code for coresponding operation primitive function. */
	struct compiler *compiler = visitor->data;
	emit_call(compiler, fn_ident);
}

static void gen_op_unary(struct visitor *visitor,
			struct node_op_unary *node) {
	/* Generate code for expression. */
	visitor_visit(visitor, node->expr);

	/* If minus or not operator, call @neg primitive function. */
	/* TODO: Implement @neg primitive function. */
	if (node->op != OP_ADD) {
		struct compiler *compiler = visitor->data;
		emit_call(compiler, "@neg");
	}
}

static void gen_stmt_decl(struct visitor *visitor,
				struct node_stmt_decl *node) {
	struct compiler *compiler = visitor->data;
	declare_local(compiler, node->ident->val, node->type_ident->val);
}

static void gen_stmt_assign(struct visitor *visitor,
				struct node_stmt_assign *node) {
	struct compiler *compiler = visitor->data;

	/* Generate code for expression with the variable will be assigned to. */
	visitor_visit(visitor, node->right);

	int index = resolve_local(compiler, node->ident->val);
	if (index == -1) {
		return;
	}

	emit_u8(compiler, VM_OP_ST_LOCAL);
	emit_u8(compiler, index);
}

static void gen_stmt_output(struct visitor *visitor,
				struct node_stmt_output *node) {
	/* Generate code for expression. */
	visitor_visit(visitor, node->expr);

	/* Generate code for call to primitive OUTPUT function. */
	struct compiler *compiler = visitor->data;
	emit_call(compiler, "@output");
}

static void gen_stmt_return(struct visitor *visitor,
				struct node_stmt_return *node) {
	/* Generate code for expression to return. */
	visitor_visit(visitor, node->expr);

	struct compiler *compiler = visitor->data;
	emit_u8(compiler, VM_OP_RET);
}

static void gen_block(struct visitor *visitor,
				struct node_block *node) {
	/* Generate code for each statement in block. */
	for (size_t i = 0; i < node->stmts.count; i++) {
		visitor_visit(visitor, node->stmts.data[i]);
	}
}

static void gen_function(struct visitor *visitor,
				struct node_function *node) {
	/* Top-level compiler. */
	struct compiler *top_compiler = visitor->data;
	struct state *state = top_compiler->state;
	
	/* Function we're going to compile from the node_function. */
	struct function *sub_fn = pseu_alloc(state->vm, sizeof(struct function));
	sub_fn->ident = strclone(state->vm, node->ident->val);
	sub_fn->params_count = node->params.count;

	/* Initialize compiler which will compile the new function. */
	struct compiler sub_compiler;
	compiler_init(state, &sub_compiler, top_compiler, sub_fn);

	/* Initialize new function's parameters. */
	if (sub_fn->params_count > 0) {
		sub_fn->params_types = pseu_alloc(state->vm,
					sizeof(struct type *) * sub_fn->params_count);

		for (size_t i = 0; i < node->params.count; i++) {
			struct node_param *param = node->params.data[i];
			struct variable *param_var = declare_local(&sub_compiler,
							param->ident->val, param->type_ident->val);

			if (param_var) {
				sub_fn->params_types[i] = param_var->type;
			}
		}
	} else {
		sub_fn->params_types = NULL;
	}

	struct type *return_type;
	if (node->return_type_ident) {
		return_type = resolve_type(top_compiler, node->return_type_ident->val);
		if (!return_type) {
			error(top_compiler, "Unknown return type");
		}
	} else {
		return_type = &top_compiler->state->vm->void_type;
	}

	/* Compile the body of the new function. */
	struct closure *closure = compiler_compile_closure(&sub_compiler,
										node->body);
	closure->fn = sub_fn;

	sub_fn->type = FN_TYPE_USER;
	sub_fn->return_type = return_type;
	sub_fn->as_closure = closure;
	
	/* Register to symbol table. */
	if (symbol_table_add_function(&top_compiler->state->vm->symbols, sub_fn)) {
		error(top_compiler, "Already defined function with same identifier");
	}

#ifdef PSEU_DEBUG_CODE
	pseu_dump_function_info(stdout, sub_fn);
	pseu_dump_function_code(stdout, state->vm, sub_fn);
#endif
}

/* Initializes a compiler to its default state. */
static void compiler_init(struct state *state, struct compiler *compiler,
				struct compiler *top_compiler, struct function *fn) {
	pseu_assert(state && compiler && fn);

	compiler->state = state;
	compiler->fn = fn;
	compiler->top = top_compiler;
	compiler->stack_size = 0;
	compiler->consts_count = 0;
	compiler->locals_count = 0;
	compiler->error_count = 0;

	emitter_init(compiler);
}

/* Initialize a visitor to use appropriate visiting functions for code gen. */
static void compiler_set_visitor(struct compiler *compiler,
				struct visitor *visitor) {
	visitor->data = compiler;
	visitor->visit_call = gen_call;
	visitor->visit_ident = gen_ident;
	visitor->visit_boolean = gen_boolean;
	visitor->visit_integer = gen_integer;
	visitor->visit_real = gen_real;
	visitor->visit_op_binary = gen_op_binary;
	visitor->visit_op_unary = gen_op_unary;
	visitor->visit_stmt_decl = gen_stmt_decl;
	visitor->visit_stmt_assign = gen_stmt_assign;
	visitor->visit_stmt_output = gen_stmt_output;
	visitor->visit_stmt_return = gen_stmt_return;
	visitor->visit_block = gen_block;
	visitor->visit_function = gen_function;
}

static struct closure *compiler_compile_closure(struct compiler *compiler,
				struct node_block *node) {
	pseu_assert(compiler && node);

	struct state *state = compiler->state;

	/* 
	 * Initialize visitor which is going to transverse the nodes and generate
	 * code.
	 */
	struct visitor visitor = { };
	compiler_set_visitor(compiler, &visitor);
	visitor_visit(&visitor, (struct node *)node);

	/* Finalize block. */
	emit_u8(compiler, VM_OP_END);

	/* Allocate and initialize closure. */
	struct closure *closure = pseu_alloc(state->vm, sizeof(struct closure));
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
	return closure;
}

int compiler_compile(struct state *state, struct compiler *compiler,
				struct function *fn, const char *src) {
	pseu_assert(state && compiler && fn && src);

	/* Initialize top-level compiler. */
	compiler_init(state, compiler, NULL, fn);

	int result = 0;
	/* Lexer of the compiler, used by the parser. */
	struct lexer lexer;
	/* Parser of the compiler. */
	struct parser parser;

	lexer_init(&lexer, src);
	parser_init(&parser, &lexer, state);

	/* Parse source code into abstract syntax tree. */
	struct node *node = parser_parse(&parser);

#ifdef PSEU_DEBUG_AST
	pseu_dump_node(stdout, node);
#endif

	/* Check if parse failed. */
	if (parser.error_count > 0) {
		result = 1;
		goto exit;
	}

	/* Compile top-level closure & functions declartions in it. */
	struct closure *closure = compiler_compile_closure(compiler, 
										(struct node_block *)node);
	if (compiler->error_count > 0) {
		result = 1;
		goto exit;
	}

	closure->fn = fn;
	if (!closure) {
		return 1;
	}

	/* Set top-level function. */
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

exit:
	node_free(state->vm, node);
	return result;
}
