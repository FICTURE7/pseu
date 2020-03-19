#include <math.h>
#include <stdio.h>

#include "core.h"

#define arg(x)      (&args[x])
#define return_v(x) do {*arg(0) = (x); return 0;} while(0)

#define PSEU_ARITH_FUNC(x)      \
  PSEU_FUNC(x)                  \
  {                             \
    pseu_unused(s);             \
    return pseu_arith_binary(   \
        arg(0),                 \
        arg(1),                 \
        arg(0),                 \
        ARITH_##x);             \
  }

#define PSEU_COMP_FUNC(x)       \
  PSEU_FUNC(x)                  \
  {                             \
    pseu_unused(s);             \
    return pseu_compare_binary( \
        arg(0),                 \
        arg(1),                 \
        arg(0),                 \
        COMP_##x);              \
  }

#define PSEU_LOGIC_FUNC(x, op)  \
  PSEU_FUNC(x)                  \
  {                             \
    pseu_unused(s);             \
    return_v(v_bool(            \
          v_asbool(arg(0)) op   \
          v_asbool(arg(1))));   \
  }

PSEU_ARITH_FUNC(add)
PSEU_ARITH_FUNC(sub)
PSEU_ARITH_FUNC(div)
PSEU_ARITH_FUNC(mul)

PSEU_COMP_FUNC(lt)
PSEU_COMP_FUNC(gt)
PSEU_COMP_FUNC(le)
PSEU_COMP_FUNC(ge)
PSEU_COMP_FUNC(eq)

PSEU_LOGIC_FUNC(and, &&)
PSEU_LOGIC_FUNC(or,  ||)

PSEU_FUNC(output)
{
  pseu_unused(s);

  // TODO: Fix this see parse_err in parse.c.
  char buffer[256];

  switch (arg(0)->type) {
  case VAL_BOOL:
    sprintf(buffer, v_asbool(arg(0)) == false ? "false\n" : "true\n");
    break;
  case VAL_INT:
    sprintf(buffer, "%d\n", v_asint(arg(0)));
    break;
  case VAL_FLOAT:
    sprintf(buffer, "%f\n", v_asfloat(arg(0)));
    break;

  default:
    sprintf(buffer, "%s<%p>\n", v_get_type(s, arg(0))->ident, (void *)arg(0));
    break;
  }

  pseu_print(s, buffer);
  return 0;
}

PSEU_FUNC(neg)
{
  pseu_unused(s);

  switch (arg(0)->type) {
  case VAL_INT:
    return_v(v_int(-v_asint(arg(0))));
  case VAL_FLOAT:
    return_v(v_float(-v_asfloat(arg(0))));

  default:
    return 1;
  }
}

PSEU_FUNC(not)
{
  pseu_unused(s);
  return_v(v_bool(!v_asbool(arg(0))));
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

  PSEU_DEF_FUNC(and,    RETURN("BOOLEAN"), PARAMS("BOOLEAN", "BOOLEAN"));
  PSEU_DEF_FUNC(or,     RETURN("BOOLEAN"), PARAMS("BOOLEAN", "BOOLEAN"));
  PSEU_DEF_FUNC(not,    RETURN("BOOLEAN"), PARAMS("BOOLEAN"));

  PSEU_DEF_FUNC(lt,     RETURN("BOOLEAN"), PARAMS("ANY", "ANY"));
  PSEU_DEF_FUNC(gt,     RETURN("BOOLEAN"), PARAMS("ANY", "ANY"));
  PSEU_DEF_FUNC(le,     RETURN("BOOLEAN"), PARAMS("ANY", "ANY"));
  PSEU_DEF_FUNC(ge,     RETURN("BOOLEAN"), PARAMS("ANY", "ANY"));
  PSEU_DEF_FUNC(eq,     RETURN("BOOLEAN"), PARAMS("ANY", "ANY"));

  PSEU_DEF_CONST(TRUE,  v_bool(1));
  PSEU_DEF_CONST(FALSE, v_bool(0));
  PSEU_DEF_CONST(PI,    v_float(M_PI));
}
