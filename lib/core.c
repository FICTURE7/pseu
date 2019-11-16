#include <math.h>
#include <stdio.h>

#include "core.h"

#define ARGS(x) (&args[x])

#define PSEU_ARITH_FUNC(x, op) 		\
	PSEU_FUNC(x) 					\
	{ 								\
		pseu_unused(s);				\
		return pseu_arith_binary(	\
				ARGS(0), 			\
				ARGS(1), 			\
				ARGS(0), 			\
				#op[0]);  			\
	}

PSEU_ARITH_FUNC(add, +)
PSEU_ARITH_FUNC(sub, -)
PSEU_ARITH_FUNC(div, /)
PSEU_ARITH_FUNC(mul, *)

PSEU_FUNC(output)
{
	char buffer[256];

	pseu_unused(s);

	switch (ARGS(0)->type) {
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

void pseu_core_init(pseu_vm_t *vm)
{
	PSEU_DEF_TYPE(ANY,		&vm->any_type);
	PSEU_DEF_TYPE(REAL,		&vm->real_type);
	PSEU_DEF_TYPE(BOOLEAN,	&vm->boolean_type);
	PSEU_DEF_TYPE(INTEGER,	&vm->integer_type);

	PSEU_DEF_PROC(output, PARAMS("ANY"));

	PSEU_DEF_FUNC(add, 	  RETURN("ANY"), 	PARAMS("ANY", "ANY"));
	PSEU_DEF_FUNC(sub, 	  RETURN("ANY"), 	PARAMS("ANY", "ANY"));
	PSEU_DEF_FUNC(mul, 	  RETURN("ANY"), 	PARAMS("ANY", "ANY"));
	PSEU_DEF_FUNC(div, 	  RETURN("ANY"), 	PARAMS("ANY", "ANY"));

	PSEU_DEF_CONST(PI,	  v_float(M_PI));
}
