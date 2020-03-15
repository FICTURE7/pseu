#ifndef PSEU_VM_H
#define PSEU_VM_H

#include <pseu.h>
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "obj.h"

/* Initial size of the evaluation stack. */
#define PSEU_INIT_EVALSTACK_SIZE 256
/* Initial size of the call stack. */
#define PSEU_INIT_CALLSTACK_SIZE 8

/* Maximum number of constants in a function. */
#define PSEU_MAX_CONST  (1 << 8)
/* Maximum number of local variables in a function. */
#define PSEU_MAX_LOCAL  (1 << 8)
/* Maximum number of globals in a pseu virtual machine instance. */
#define PSEU_MAX_GLOBAL ((1 << 16) - 1)
/* Maximum number of functions in a pseu virtual machine instance. */
#define PSEU_MAX_FUNC   ((1 << 16) - 1)
/* Maximum number of types in a pseu virtual machine instance. */
#define PSEU_MAX_TYPE   ((1 << 16) - 1)

/* Represents an invalid constant handle. */
#define PSEU_INVALID_CONST  PSEU_MAX_CONST
/* Represents an invalid local handle. */
#define PSEU_INVALID_LOCAL  PSEU_MAX_LOCAL
/* Represents an invalid global handle. */
#define PSEU_INVALID_GLOBAL PSEU_MAX_GLOBAL
/* Represents an invalid function handle. */
#define PSEU_INVALID_FUNC   PSEU_MAX_FUNC
/* Represents an invalid type handle. */
#define PSEU_INVALID_TYPE   PSEU_MAX_TYPE

#define pseu_unused(x)    (void)(x)

#if defined(PSEU_USE_ASSERT)
#define pseu_assert(cond)	assert(cond);
#else
#define pseu_assert(cond)
#endif

#if defined(__GNUC__)
#define pseu_likely(x)		__builtin_expect(!!(x), 1)
#define pseu_unlikely(x)	__builtin_expect(!!(x), 0)
#else
#define pseu_likely(x)		(x)
#define pseu_unlikely(x)	(x)
#endif

#if defined(__GNUC__)
#define pseu_unreachable(x)	 assert(0 && "Unreachable code reached")
#elif defined(MSVC)
#define pseu_unreachable(x)  __assume(0 && "Unreachable code reached")
#endif

#define pseu_config_flag(S, x)	  ((VM(S)->config.flags & (x)) != 0)

#define pseu_alloc_t(s, t)        pseu_alloc((s), sizeof(t))
#define pseu_alloc_nt(s, t, n)    pseu_alloc((s), sizeof(t) * (n))

#define pseu_vec_init(S, v, c, t) _pseu_vec_init(S, (void **)(v), c, sizeof(t))
#define pseu_vec_grow(S, v, c, t) _pseu_vec_grow(S, (void **)(v), c, sizeof(t))

State *pseu_state_new(VM *vm);
void pseu_state_free(State *s);

void *pseu_alloc(State *s, size_t sz);
void *pseu_realloc(State *s, void *ptr, size_t sz);
void pseu_free(State *s, void *ptr);
void pseu_print(State *s, const char *text);
void pseu_panic(State *s, const char *message);

char *pseu_strdup(State *s, const char *str);

int _pseu_vec_init(State *s, void **vec, size_t cap_elm, size_t size_elm);
int _pseu_vec_grow(State *s, void **vec, size_t *cap_elm, size_t size_elm);

int pseu_call(State *s, Function *fn);
int pseu_parse(State *s, Function *fn, const char *src);

void pseu_dump_stack(State *s, FILE* f);
void pseu_dump_function(State *s, FILE* f, Function *fn);

int pseu_arith_unary(Value *a, ArithType op);
int pseu_arith_binary(Value *a, Value *b, Value *o, ArithType op);

u16 pseu_def_type(VM *vm, Type *type);
u16 pseu_def_variable(VM *vm, Variable *var);
u16 pseu_def_function(VM *vm, Function *fn);

/* ----WIP---- */
Type *pseu_get_type(VM *vm, const char *ident, size_t len);
u16 pseu_get_variable(VM *vm, const char *ident, size_t len);
u16 pseu_get_function(VM *vm, const char *ident);
/* ----WIP---- */

#endif /* PSEU_VM_H */
