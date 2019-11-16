#ifndef PSEU_CORE_H
#define PSEU_CORE_H

#include "vm.h"

#define ARR_PARAMS(...) (const char *[]) { __VA_ARGS__ }
#define NUM_PARAMS(...) sizeof(ARR_PARAMS(__VA_ARGS__)) / sizeof(char *)

#define PARAMS(...) ARR_PARAMS(__VA_ARGS__), NUM_PARAMS(__VA_ARGS__)
#define RETURN(x) 	x

#define PSEU_DEF_CONST(x, v)	 def_const(vm, #x, v)
#define PSEU_DEF_TYPE(x, r) 	 def_type(vm, #x, r)
#define PSEU_DEF_PROC(x, pt) 	 def_func(vm, "@" #x, impl_##x, NULL, pt)
#define PSEU_DEF_FUNC(x, rt, pt) def_func(vm, "@" #x, impl_##x, rt, pt)

#define PSEU_FUNC(x) \
	static int impl_##x(pseu_state_t *s, struct value * args)

static void def_const(pseu_vm_t *vm, const char *ident, struct value k)
{
	struct variable v = {
		.k = 1,
		.ident = pseu_strdup(vm->state, ident),
		.value = k
	};
	int index = pseu_def_variable(vm, &v);
	if (index == PSEU_INVALID_GLOBAL)
		pseu_panic(vm->state, "Reached maximum number of globals");
}

static void def_type(pseu_vm_t *vm, const char *ident, struct type **out)
{
	struct type t = {
		.ident = pseu_strdup(vm->state, ident),
		.fields = NULL,
		.fields_count = -1
	};
	int index = pseu_def_type(vm, &t);
	if (index == PSEU_INVALID_TYPE)
		pseu_panic(vm->state, "Reached maximum number of types.");
	*out = &vm->types[index];
}

static void def_func(pseu_vm_t *vm, const char *ident, function_c_t fn,
		const char *ret_type, const char **par_types, uint8_t params_count)
{
	int index;
	struct type *return_type;
	struct type **param_types;
	struct function f = {
		.type = FN_C,
		.ident = pseu_strdup(vm->state, ident),
		.as.c = fn
	};

	if (!ret_type) {
		return_type = NULL;
	} else {
		return_type = pseu_get_type(vm, ret_type, strlen(ret_type));
		if (return_type == NULL)
			pseu_panic(vm->state, "Unknown return type.");
	}

	param_types = pseu_alloc(vm->state, sizeof(struct type *) * params_count);

	for (uint8_t i = 0; i < params_count; i++) {
		param_types[i] = pseu_get_type(vm, par_types[i], 
				strlen(par_types[i]));
		if (param_types[i] == NULL)
			pseu_panic(vm->state, "Unknown parameter type.");
	}

	f.param_types = param_types;
	f.params_count = params_count;
	f.return_type = return_type;
	index = pseu_def_function(vm, &f);
	if (index == PSEU_INVALID_FUNC)
		pseu_panic(vm->state, "Reached maximum number of functions.");
}

void pseu_core_init(pseu_vm_t *vm);

#endif /* PSEU_CORE_H */
