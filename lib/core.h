#ifndef PSEU_CORE_H
#define PSEU_CORE_H

#include "vm.h"

/*
 * TODO: Document this macro soccery.
 */

#define P_FDEF(x) 												\
	static int impl_##x(pseu_state_t *s, struct value *stack); 	\
	static void def_##x(pseu_vm_t *vm) { 						\
		function_c_t c = impl_##x; 								\
		const char *n = "@" #x; 								\
		struct type *t[] = 										
#define P_FPARAM(d) (d)
#define P_FDEF_END(r) 													\
		;																\
		def_function_primitive(vm, n, c, r, t, sizeof(t) / sizeof(*t)); \
	}

#define P_FIMPL(x) \
	static int impl_##x(pseu_state_t *s, struct value * args)

#define P_FARITH(x, o) 								\
	P_FDEF(x) { 									\
		P_FPARAM(vm->any_type), 					\
		P_FPARAM(vm->any_type) 						\
	} 												\
	P_FDEF_END(vm->any_type) 						\
	P_FIMPL(x) 										\
	{ 												\
		pseu_unused(s); 							\
		struct value *a = &args[0]; 				\
		struct value *b = &args[1]; 				\
		return pseu_arith_binary(a, b, a, #o[0]); 	\
	}

static void def_constf(pseu_vm_t *vm, const char *ident, float value)
{
	struct variable v;
	const char *nident = pseu_strdup(vm->state, ident);

	v.k = 1;
	v.ident = nident;
	v.value = (struct value) {
		.type = VAL_FLOAT,
		.as.real = value
	};
	
	pseu_def_variable(vm, &v);
}

static void def_type_primitive(pseu_vm_t *vm, const char *ident, 
		struct type **out)
{
	struct type t;
	const char *nident = pseu_strdup(vm->state, ident);

	t.ident  = nident;
	t.fields = NULL;
	t.fields_count = 0;

	*out = pseu_def_type(vm, &t);
}

static void def_function_primitive(
		pseu_vm_t *vm, 
		const char *ident,
		function_c_t cfn, 
		struct type *return_type,
		struct type **param_types,
		uint8_t params_count)
{
	struct function f;
	const char *nident = pseu_strdup(vm->state, ident);

	f.type  = FN_C;
	f.ident = nident;
	f.params_count = params_count;
	f.return_type  = return_type;
	f.as.c  = cfn;

	if (params_count == 0) {
		f.param_types = NULL;
	} else {
		size_t n = params_count * sizeof(*f.param_types);
		f.param_types = pseu_alloc(vm->state, n);
		memcpy(f.param_types, param_types, n);
	}

	pseu_def_function(vm, &f);
}

int pseu_core_init(pseu_vm_t *vm);

#endif /* PSEU_CORE_H */
