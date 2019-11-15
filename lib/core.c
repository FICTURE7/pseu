#include <math.h>
#include <stdio.h>

#include "core.h"

P_FARITH(add, +)
P_FARITH(sub, -)
P_FARITH(mul, *)
P_FARITH(div, /)

P_FDEF(output) {
	P_FPARAM(vm->any_type)
} 
P_FDEF_END(NULL)

P_FIMPL(output)
{
	char buffer[256];
	switch (args->type) {
	case VAL_BOOL:
		sprintf(buffer, args->as.boolean == 0 ? "false" : "true");
		break;
	case VAL_INT:
		sprintf(buffer, "%d\n", args->as.integer);
		break;
	case VAL_FLOAT:
		sprintf(buffer, "%f\n", args->as.real);
		break;
	default:
		sprintf(buffer, "unknown<%p>\n", (void *)args);
		break;
	}
	pseu_print(s, buffer);
	return 0;
}

int pseu_core_init(pseu_vm_t *vm)
{
	#define def_function(v, x) def_##x(v)

	def_constf(vm, "PI", M_PI);
	def_type_primitive(vm, "ANY", &vm->any_type);
	def_type_primitive(vm, "BOOLEAN", &vm->boolean_type);
	def_type_primitive(vm, "INTEGER", &vm->integer_type);
	def_type_primitive(vm, "REAL", &vm->real_type);

	def_function(vm, add);
	def_function(vm, sub);
	def_function(vm, mul);
	def_function(vm, div);
	def_function(vm, output);
	return 0;
}
