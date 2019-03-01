#include <stdio.h>
#include <stdbool.h>

#include "pseu_value.h"
#include "pseu_token.h"
#include "pseu_debug.h"
#include "pseu_string.h"
#include "pseu_visitor.h"

struct node_dump_info {
	FILE *stream;
	unsigned int depth;
};

static void print_op(FILE *stream, enum token_type type) {
	switch (type) {
		case TOK_OP_ADD:
			fprintf(stream, "+");
			break;
		case TOK_OP_SUB:
			fprintf(stream, "-");
			break;
		case TOK_OP_MUL:
			fprintf(stream, "*");
			break;
		case TOK_OP_DIV:
			fprintf(stream, "/");
			break;
		case TOK_OP_LOGICAL_NOT:
			fprintf(stream, "!");
			break;
		case TOK_OP_LOGICAL_AND:
			fprintf(stream, "&&");
			break;
		case TOK_OP_LOGICAL_OR:
			fprintf(stream, "||");
			break;

		default:
			fprintf(stream, "UNKOP(%d)", type);
			break;
	}
}

static void print_indent(FILE *stream, unsigned int depth) {
	for (int i = 0; i < depth; i++) {
		fprintf(stream, " ");
	}

	if (depth > 0) {
		fprintf(stream, "`---");
	}
}

static void dump_node(struct visitor *visitor, struct node *node) {
	struct node_dump_info *info = visitor->data;

	if (node) {
		visitor_visit(visitor, node);
	} else {
		fprintf(info->stream, "NULL");
	}
}

static void dump_ident(struct visitor *visitor,
				struct node_ident *ident) {
	struct node_dump_info *info = visitor->data;

	print_indent(info->stream, info->depth);
	fprintf(info->stream, "ident(%s)\n", ident->val);
}

static void dump_string(struct visitor *visitor,
				struct node_string *string) {
	struct node_dump_info *info = visitor->data;

	print_indent(info->stream, info->depth);
	fprintf(info->stream, "string(%zu, '%s')\n", string->val->length,
				string->val->buffer);
}

static void dump_real(struct visitor *visitor,
				struct node_real *real) {
	struct node_dump_info *info = visitor->data;

	print_indent(info->stream, info->depth);
	fprintf(info->stream, "real(%f)\n", real->val);
}

static void dump_integer(struct visitor *visitor,
				struct node_integer *integer) {
	struct node_dump_info *info = visitor->data;

	print_indent(info->stream, info->depth);
	fprintf(info->stream, "integer(%d)\n", integer->val);
}

static void dump_boolean(struct visitor *visitor,
				struct node_boolean *boolean) {
	struct node_dump_info *info = visitor->data;

	print_indent(info->stream, info->depth);
	fprintf(info->stream, "boolean(%d)\n", boolean->val);
}

static void dump_op_unary(struct visitor *visitor,
				struct node_op_unary *op_unary) {
	struct node_dump_info *info = visitor->data;

	print_indent(info->stream, info->depth);
	fprintf(info->stream, "op_unary:\n");
	info->depth++;

	print_indent(info->stream, info->depth);
	fprintf(info->stream, "op: ");
	print_op(info->stream, op_unary->op);
	fprintf(info->stream, "\n");

	dump_node(visitor, op_unary->expr);
	info->depth--;
}

static void dump_op_binary(struct visitor *visitor,
				struct node_op_binary *op_binary) {
	struct node_dump_info *info = visitor->data;

	print_indent(info->stream, info->depth);
	fprintf(info->stream, "op_binary:\n");
	info->depth++;

	dump_node(visitor, op_binary->left);

	print_indent(info->stream, info->depth);
	printf("op: ");
	print_op(info->stream, op_binary->op);
	printf("\n");

	dump_node(visitor, op_binary->right);
	info->depth--;
}

static void dump_stmt_decl(struct visitor *visitor,
				struct node_stmt_decl *decl) {
	struct node_dump_info *info = visitor->data;

	print_indent(info->stream, info->depth);
	fprintf(info->stream, "decl(%s:%s)\n", decl->ident->val,
				decl->type_ident->val);
}

static void dump_stmt_assign(struct visitor *visitor,
				struct node_stmt_assign *assign) {
	struct node_dump_info *info = visitor->data;

	print_indent(info->stream, info->depth);
	fprintf(info->stream, "assign(%s):\n", assign->ident->val);

	info->depth++;
	dump_node(visitor, assign->right);
	info->depth--;
}

static void dump_stmt_output(struct visitor *visitor,
				struct node_stmt_output *output) {
	struct node_dump_info *info = visitor->data;

	print_indent(info->stream, info->depth);
	fprintf(info->stream, "output:\n");

	info->depth++;
	dump_node(visitor, output->expr);
	info->depth--;
}

static void dump_block(struct visitor *visitor,
			struct node_block *block) {
	struct node_dump_info *info = visitor->data;

	print_indent(info->stream, info->depth);
	info->depth++;

	fprintf(info->stream, "block(%zu):\n", block->stmts.count);
	for (int i = 0; i < block->stmts.count; i++) {
		dump_node(visitor, block->stmts.data[i]);
	}
}

void pseu_dump_token(FILE *stream, struct token *token) {
	char *buffer = malloc(token->len + 1);
	token_value(token, buffer);
	fprintf(stream, "tok[%d,%d:%zu](%d, '%s')\n", token->loc.ln, token->loc.col,
					token->len, token->type, buffer);
	free(buffer);
}

void pseu_dump_node(FILE *stream, struct node *node) {	
	struct node_dump_info info = {
		.depth = 0,
		.stream = stream
	};

	struct visitor visitor;
	visitor.data = &info;
	visitor.visit_ident = dump_ident;
	visitor.visit_boolean = dump_boolean;
	visitor.visit_integer = dump_integer;
	visitor.visit_real = dump_real;
	visitor.visit_string = dump_string;
	visitor.visit_op_unary = dump_op_unary;
	visitor.visit_op_binary = dump_op_binary;
	visitor.visit_stmt_decl = dump_stmt_decl;
	visitor.visit_stmt_assign = dump_stmt_assign;
	visitor.visit_stmt_output = dump_stmt_output;
	visitor.visit_block = dump_block;
	visitor_visit(&visitor, node);
}

void pseu_dump_value(FILE *stream, struct value *value) {
	switch (value->type) {
		case VALUE_TYPE_BOOLEAN:
			fprintf(stream, "boolean(%s)", 
						value->as_bool == true ? "true" : "false");
			break;
		case VALUE_TYPE_INTEGER:
			fprintf(stream, "integer(%d)", value->as_int);
			break;
		case VALUE_TYPE_REAL:
			fprintf(stream, "real(%f)", value->as_float);
			break;
		case VALUE_TYPE_OBJECT:
			/* TODO: Properly print strings. */
			fprintf(stream, "object(%p)", value->as_object);
			break;

		default:
			fprintf(stream, "Unknown value type");
			break;
	}
}

void pseu_dump_variable(FILE *stream, struct variable *var) {
	fprintf(stream, "'%s':%s", var->ident, var->type->ident);
}

void pseu_dump_function_info(FILE *stream, struct function *fn) {
	/* TODO: Implement. */
}

void pseu_dump_function_code(FILE *stream, pseu_vm_t *vm, struct function *fn) {
	#define READ_UINT8() (*ip++)
	#define READ_UINT16() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))
	#define OP(x) VM_OP_##x

	/* Function must be a user defined function. */
	pseu_assert(fn->type == FN_TYPE_USER);

	struct closure *closure = fn->as_closure;
	code_t *ip = closure->code;
	code_t op = OP(END);
	do {
		fprintf(stream, "%03u ", (unsigned int)(ip - closure->code));
		op = READ_UINT8();

		switch (op) {
			case OP(CALL): {
				unsigned int call_fn_id = READ_UINT16();
				struct function *call_fn = vm->symbols.fns.data[call_fn_id];

				fprintf(stream, "call '%s'   ; -> [%u]\n",
							call_fn->ident, call_fn_id);
				break;
			}
			case OP(LD_CONST): {
				unsigned int index = READ_UINT8();

				fprintf(stream, "ld.const ");
				pseu_dump_value(stream, &closure->consts[index]);
				fprintf(stream, "  ; -> [%u]\n", index);
				break;
			}
			case OP(LD_LOCAL): {
				unsigned int index = READ_UINT8();

				fprintf(stream, "ld.local ");
				pseu_dump_variable(stream, &closure->locals[index]);
				fprintf(stream, "  ; -> [%u]\n", index);
				break;
			}
			case OP(ST_LOCAL): {
				unsigned int index = READ_UINT8();

				fprintf(stream, "st.local ");
				pseu_dump_variable(stream, &closure->locals[index]);
				fprintf(stream, "  ; -> [%u]\n", index);
				break;
			}
			case OP(END): {
				fprintf(stream, "end\n");
				/* Exit dumping function. */
				return;
			}

			default: {
				fprintf(stream, "UNK\n");
				/* Exit early, just in case. */
				return;
			}
		}
	} while (op != OP(END));
}
