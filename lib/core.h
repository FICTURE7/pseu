#ifndef PSEU_CORE_H
#define PSEU_CORE_H

#include "vm.h"

#define ARR_PARAMS(...) (const char *[]) { __VA_ARGS__ }
#define NUM_PARAMS(...) sizeof(ARR_PARAMS(__VA_ARGS__)) / sizeof(char *)

#define PARAMS(...) ARR_PARAMS(__VA_ARGS__), NUM_PARAMS(__VA_ARGS__)
#define RETURN(x) 	x

#define PSEU_DEF_CONST(x, v)     def_const(vm, #x, v)
#define PSEU_DEF_TYPE(x, r)      def_type(vm, #x, r)
#define PSEU_DEF_PROC(x, pt)     def_func(vm, "@" #x, NULL, pt, impl_##x)
#define PSEU_DEF_FUNC(x, rt, pt) def_func(vm, "@" #x, rt, pt, impl_##x)

#define PSEU_FUNC(x) \
  static int impl_##x(State *s, Value *args)

static inline
Type *resolve_type(VM *vm, const char *type_ident, const char *error_message)
{
  Type *result = pseu_get_type(vm, type_ident, strlen(type_ident));
  if (result == NULL)
    pseu_panic(S(vm), error_message);
  return result;
}

static inline
void def_const(VM *vm, const char *ident, Value konst_value)
{
  Variable var = {
    .ident = pseu_strdup(S(vm), ident),
    .value = konst_value,
    .konst = true
  };

  u16 index = pseu_def_variable(vm, &var);
  if (index == PSEU_INVALID_GLOBAL)
    pseu_panic(S(vm), "Reached maximum number of globals");
}

static inline
void def_type(VM *vm, const char *ident, Type **out)
{
  Type type = {
    .ident = pseu_strdup(S(vm), ident),
    .fields = NULL,
    .fields_count = -1
  };

  u16 index = pseu_def_type(vm, &type);
  if (index == PSEU_INVALID_TYPE)
    pseu_panic(S(vm), "Reached maximum number of types.");
  *out = &vm->types[index];
}

static inline
void def_func(VM *vm, 
              const char *ident, const char *return_type_ident,
              const char **param_types_idents, u8 params_count,
              FunctionC cfn)
{
  Type *return_type = NULL;
  if (return_type_ident)
    return_type = resolve_type(vm, return_type_ident, "Unknown return type.");

  Type **param_types = pseu_alloc_nt(S(vm), Type *, params_count);
  for (u8 i = 0; i < params_count; i++)
    param_types[i] = resolve_type(vm, param_types_idents[i], "Unknown parameter type.");

  Function fn = {
    .type = FN_C,
    .ident = pseu_strdup(S(vm), ident),
    .param_types = param_types,
    .params_count = params_count,
    .return_type = return_type,
    .as.c = cfn
  };

  if (pseu_def_function(vm, &fn) == PSEU_INVALID_FUNC)
    pseu_panic(S(vm), "Reached maximum number of functions.");
}

void pseu_core_init(VM *vm);

#endif /* PSEU_CORE_H */
