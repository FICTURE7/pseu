#include <math.h>
#include <stdio.h>

#include "core.h"

#define ARG(x) (&args[x])
#define RET(x) *ARG(0) = (x)

#define PSEU_ARITH_FUNC(x, op) \
  PSEU_FUNC(x) 					       \
  {                            \
    pseu_unused(s);            \
    return pseu_arith_binary(  \
        ARG(0),                \
        ARG(1),                \
        ARG(0),                \
        #op[0]                 \
    );                         \
  }

PSEU_ARITH_FUNC(add, +)
PSEU_ARITH_FUNC(sub, -)
PSEU_ARITH_FUNC(div, /)
PSEU_ARITH_FUNC(mul, *)

PSEU_FUNC(output)
{
  pseu_unused(s);

  // TODO: Fix this see parse_err in parse.c.
  char buffer[256];

  switch (ARG(0)->type) {
  case VAL_BOOL:
    sprintf(buffer, ARG(0)->as.boolean == 0 ? "false\n" : "true\n");
    break;
  case VAL_INT:
    sprintf(buffer, "%d\n", ARG(0)->as.integer);
    break;
  case VAL_FLOAT:
    sprintf(buffer, "%f\n", ARG(0)->as.real);
    break;

  default:
    sprintf(buffer, "%s<%p>\n", v_type(s, ARG(0))->ident, (void *)ARG(0));
    break;
  }

  pseu_print(s, buffer);
  return 0;
}

PSEU_FUNC(neg)
{
  pseu_unused(s);

  switch (ARG(0)->type) {
  case VAL_BOOL:
    RET(v_bool(!ARG(0)->as.boolean));
    break;
  case VAL_INT:
    RET(v_int(-ARG(0)->as.integer));
    break;
  case VAL_FLOAT:
    RET(v_float(-ARG(0)->as.real));
    break;

  default:
    return 1;
  }

  return 0;
}

void pseu_core_init(VM *vm)
{
  PSEU_DEF_TYPE(ANY,     &vm->any_type);
  PSEU_DEF_TYPE(REAL,    &vm->real_type);
  PSEU_DEF_TYPE(BOOLEAN, &vm->boolean_type);
  PSEU_DEF_TYPE(INTEGER, &vm->integer_type);
  PSEU_DEF_TYPE(ARRAY,   &vm->array_type);

  PSEU_DEF_PROC(output, PARAMS("ANY"));

  PSEU_DEF_FUNC(add, 	  RETURN("ANY"), PARAMS("ANY", "ANY"));
  PSEU_DEF_FUNC(sub, 	  RETURN("ANY"), PARAMS("ANY", "ANY"));
  PSEU_DEF_FUNC(mul, 	  RETURN("ANY"), PARAMS("ANY", "ANY"));
  PSEU_DEF_FUNC(div, 	  RETURN("ANY"), PARAMS("ANY", "ANY"));
  PSEU_DEF_FUNC(neg,    RETURN("ANY"), PARAMS("ANY"));

  PSEU_DEF_CONST(TRUE,  v_bool(1));
  PSEU_DEF_CONST(FALSE, v_bool(0));
  PSEU_DEF_CONST(PI,    v_float(M_PI));
}
