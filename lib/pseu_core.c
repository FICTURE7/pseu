#include <stdio.h>
#include <string.h>

#include "pseu_vm.h"
#include "pseu_core.h"
#include "pseu_state.h"
#include "pseu_debug.h"

#define DEF_PRIMITIVE(name, ret, ...) \
	struct type *params_types_##name[] = { __VA_ARGS__ }; \
	struct function *fn_##name = pseu_alloc(vm, sizeof(struct function)); \
	fn_##name->type = FN_TYPE_PRIMITIVE; \
	fn_##name->ident = "@" #name; \
	fn_##name->params_count = sizeof(params_types_##name) \
								/ sizeof(struct type *); \
	fn_##name->params_types = pseu_alloc(vm, sizeof(params_types_##name)); \
	fn_##name->return_type = ret; \
	fn_##name->as_primitive = name; \
	memcpy(fn_##name->params_types, params_types_##name, \
								sizeof(params_types_##name)); \
	symbol_table_add_function(&vm->symbols, fn_##name)

#define TO_INTEGER(a) ((struct value) { \
		.type = VALUE_TYPE_INTEGER, .as_int = (a) \
	})
#define TO_REAL(a) ((struct value) { \
		.type = VALUE_TYPE_REAL, .as_float = (a) \
	})

static void output(struct state *state, struct value *args) {
	pseu_assert(state && args);

	/* TODO: Implement something more solid than this. */
	char buffer[255];
	struct value *value = args;
	switch (value->type) {
		case VALUE_TYPE_INTEGER:
			sprintf(buffer, "%d\n", value->as_int);
			break;

		default:
			sprintf(buffer, "unknown(%d)<%p>\n", value->type, value);
			break;
	}

	pseu_print(state->vm, buffer);
}

static void add(struct state *state, struct value *args) {
	pseu_assert(state && args);

	struct value *a = &args[0];
	struct value *b = &args[1];
	if (a->type == VALUE_TYPE_INTEGER && b->type == VALUE_TYPE_INTEGER) {
		args[0] = TO_INTEGER(a->as_int + b->as_int);
	} else if (a->type == VALUE_TYPE_REAL && b->type == VALUE_TYPE_REAL) {
		args[0] = TO_REAL(a->as_float + b->as_float);
	}
}

static void define_primitive_type(pseu_vm_t *vm, struct type *type,
				const char *ident) {
	type->ident = ident;
	type->fields_count = 0;
	
	symbol_table_add_type(&vm->symbols, type);
}

void core_primitives_init(pseu_vm_t *vm) {
	pseu_assert(vm);
	
	define_primitive_type(vm, &vm->void_type, "VOID");
	define_primitive_type(vm, &vm->boolean_type, "BOOLEAN");
	define_primitive_type(vm, &vm->integer_type, "INTEGER");
	define_primitive_type(vm, &vm->real_type, "REAL");
	define_primitive_type(vm, &vm->string_type, "STRING");
	/* define_primitive_type(vm, &vm->array_type, "ARRAY"); */

	DEF_PRIMITIVE(output, NULL, 
			&vm->void_type);
	DEF_PRIMITIVE(add, &vm->void_type, 
			&vm->void_type, &vm->void_type);
}
