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
  static int impl_##x(pseu_state_t *s, struct value *args)

static inline struct type *resolve_type(pseu_vm_t *vm, 
                                        const char *type_ident,
                                        const char *error_message)
{
  struct type *result = pseu_get_type(vm, type_ident, strlen(type_ident));

  if (result == NULL)
    pseu_panic(vm->state, error_message);

  return result;
}

static inline void def_const(pseu_vm_t *vm, 
                             const char *ident,
                             struct value konst_value)
{
  struct variable v = {
    .ident = pseu_strdup(vm->state, ident),
    .value = konst_value,
    .k = 1
  };

  uint16_t index = pseu_def_variable(vm, &v);

  if (index == PSEU_INVALID_GLOBAL)
    pseu_panic(vm->state, "Reached maximum number of globals");
}

static inline void def_type(pseu_vm_t *vm,
                            const char *ident,
                            struct type **out)
{
  struct type t = {
    .ident = pseu_strdup(vm->state, ident),
    .fields = NULL,
    .fields_count = -1
  };

  uint16_t index = pseu_def_type(vm, &t);

  if (index == PSEU_INVALID_TYPE)
    pseu_panic(vm->state, "Reached maximum number of types.");

  *out = &vm->types[index];
}

static inline void def_func(pseu_vm_t *vm, 
                            const char *ident, 
                            const char *return_type_ident,
                            const char **param_types_ident,
                            uint8_t params_count,
                            function_c_t fn)
{
  struct type *return_type = NULL;

  if (return_type_ident)
    return_type = resolve_type(vm, return_type_ident, "Unknown return type.");

  struct type **param_types = pseu_alloc_nt(vm->state, struct type *, params_count);

  for (uint8_t i = 0; i < params_count; i++)
    param_types[i] = resolve_type(vm, param_types_ident[i], "Unknown parameter type.");

  struct function f = {
    .type = FN_C,
    .ident = pseu_strdup(vm->state, ident),
    .param_types = param_types,
    .params_count = params_count,
    .return_type = return_type,
    .as.c = fn
  };

  if (pseu_def_function(vm, &f) == PSEU_INVALID_FUNC)
    pseu_panic(vm->state, "Reached maximum number of functions.");
}

void pseu_core_init(pseu_vm_t *vm);

#endif /* PSEU_CORE_H */
