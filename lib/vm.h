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

#ifdef PSEU_USE_ASSERT
	#define pseu_assert(cond)	assert(cond);
#else
	#define pseu_assert(cond)
#endif

#ifdef __GNUC__
	#define pseu_likely(x)		__builtin_expect(!!(x), 1)
	#define pseu_unlikely(x)	__builtin_expect(!!(x), 0)
#else
	#define pseu_likely(x)		(x)
	#define pseu_unlikely(x)	(x)
#endif

#define pseu_unreachable(x)		assert(0 && "Unreachable code reached")
#define pseu_unused(x)			(void)(x)

#define pseu_config_flag(S, x)	((VM(S)->config.flags & (x)) != 0)

pseu_state_t *pseu_state_new(pseu_vm_t *vm);
void pseu_state_free(pseu_state_t *s);

void *pseu_alloc(pseu_state_t *s, size_t size);
void *pseu_realloc(pseu_state_t *s, void *ptr, size_t size);
void pseu_free(pseu_state_t *s, void *ptr);
void pseu_print(pseu_state_t *s, const char *text);
void pseu_panic(pseu_state_t *s, const char *message);

char *pseu_strdup(pseu_state_t *s, const char *str);

#define pseu_vec_init(S, v, c, t) _pseu_vec_init(S, (void **)(v), c, sizeof(t))
#define pseu_vec_grow(S, v, c, t) _pseu_vec_grow(S, (void **)(v), c, sizeof(t))

int _pseu_vec_init(pseu_state_t *s, void **vec, 
		size_t cap_elm, size_t size_elm);
int _pseu_vec_grow(pseu_state_t *s, void **vec,
		size_t *cap_elm, size_t size_elm);

int pseu_call(pseu_state_t *s, struct function *fn);
int pseu_parse(pseu_state_t *s, struct function *fn, const char *src);

void pseu_dump_stack(pseu_state_t *s, FILE* f);
void pseu_dump_function(pseu_state_t *s, FILE* f, struct function *fn);

int pseu_arith_unary(struct value *a, int op);
int pseu_arith_binary(struct value *a, struct value *b,
		struct value *o, int op);

uint16_t pseu_def_type(pseu_vm_t *vm, struct type *type);
uint16_t pseu_def_variable(pseu_vm_t *vm, struct variable *var);
uint16_t pseu_def_function(pseu_vm_t *vm, struct function *fn);

/* ----WIP---- */
struct type *pseu_get_type(pseu_vm_t *vm, const char *ident, size_t len);
uint16_t pseu_get_variable(pseu_vm_t *vm, const char *ident, size_t len);
uint16_t pseu_get_function(pseu_vm_t *vm, const char *ident);
/* ----WIP---- */

#endif /* PSEU_VM_H */
