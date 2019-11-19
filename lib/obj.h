#ifndef PSEU_OBJ_H
#define PSEU_OBJ_H

#include <pseu.h>
#include "op.h"

struct type;
struct object;

typedef struct pseu_state pseu_state_t;

/* Types of pseu arithmetics. */
enum arith {
	ARITH_ADD = '+',
	ARITH_SUB = '-',
	ARITH_MUL = '*',
	ARITH_DIV = '/'
};

/* A pseu type field. */
struct field {
	const char *ident;		/* Identifier of field. */
	struct type *type;		/* Type of field. */
};

#define t_isany(S, t) ((t) == VM(S)->any_type)
#define t_isint(S, t) ((t) == VM(S)->integer_type)

/* A pseu type. */
struct type {
	uint8_t fields_count;	/* Number of fields of type. */
	struct field *fields;	/* Fields of type. */
	const char *ident;		/* Identifier of type. */
};

/* Types of pseu value. */
enum value_type {
	VAL_NIL,				/* Empty. XXX: Reconsider. */
	VAL_BOOL,				/* Boolean. */
	VAL_INT,				/* Signed 32-bit integer. */
	VAL_FLOAT, 				/* Single-precision floating point. */
	VAL_OBJ					/* Pointer to a heap allocated pseu object. */
};

#define v_isbool(v)  ((v)->type == VAL_BOOL)
#define v_isobj(v)	 ((v)->type == VAL_OBJ)
#define v_isint(v) 	 ((v)->type == VAL_INT)
#define v_isfloat(v) ((v)->type == VAL_FLOAT)
#define v_isnum(v)	 (v_isint(v) || v_isfloat(v))

#define v_asbool(v)	 ((v)->as.boolean)
#define v_asobj(v)	 ((v)->as.object)
#define v_asint(v)	 ((v)->as.integer)
#define v_asfloat(v) ((v)->as.real)

#define v_bool(k)	 ((struct value) { .type = VAL_BOOL, .as.boolean = (k) })
#define v_obj(k)	 ((struct value) { .type = VAL_OBJ, .as.object = (k) })
#define v_int(k)	 ((struct value) { .type = VAL_INT, .as.integer = (k) })
#define v_float(k)	 ((struct value) { .type = VAL_FLOAT, .as.real = (k) })

#define v_i2f(v)	 ((float)v_asint(v))
#define v_f2i(v)	 ((int32_t)v_asfloat(v))

/* A pseu value. */
struct value {
	uint8_t type; 	/* Type of value; see value_type. */
	union {
		float real;				/* As a real value. */
		int boolean;			/* As a boolean. */
		int32_t integer;		/* As an integer. */
		struct object *object;	/* As an object. */
	} as;
};

/* A pseu variable. */
struct variable {
	uint8_t k;				/* Is variable a constant. */
	const char *ident;		/* Identifier of variable. */
	struct value value;		/* Value of variable. */
};

#define GC_HEADER uint8_t flags

/* A pseu user object. */
struct gc_object {
	GC_HEADER;
	struct type *type;		/* Type of object. */
	struct value fields[1]; /* Field values of the object. */
};

/* A pseu string object. */
struct gc_string {
	GC_HEADER;
	uint32_t hash;			/* Hash of string. */
	uint32_t len;			/* Length of string. */
	uint8_t buf[1];			/* Buffer containing string. */
};

/* A pseu array object. */
struct gc_array {
	GC_HEADER;
	uint32_t start;			/* Start of array. */
	uint32_t end;			/* End of array. */
	struct value *val[1];	/* Values of array. */
};

/* A char buffer. */
struct cbuffer {
	size_t count;			/* Number of chars in buffer. */
	size_t size;			/* Size/capacity of buffer. */
	char *buffer;			/* Pointer to buffer block. */
};

int cbuf_new(pseu_state_t *s, struct cbuffer *buf, size_t size);
int cbuf_put(pseu_state_t *s, struct cbuffer *buf, char c);
void cbuf_free(pseu_state_t *s, struct cbuffer *buf);

/* Types of pseu function. */
enum function_type {
	FN_PSEU, 	/* Pseu function; consisting of VM bytecode. */
	FN_C 		/* Native C function which operates on a VM state eval stack. */
};

/* A pseu function. */
struct function_pseu {
	uint8_t const_count;	/* Number of constants in `consts`. */
	uint8_t local_count;	/* Number of locals in `locals`.*/
	uint16_t code_count;	/* Number of instructions in `code`. */

	uint32_t max_stack;		/* Maximum space the function occupies on the stack. */

	struct value *consts;	/* Constants in the function. */
	struct type **locals;	/* Locals in the function. */
	code_t *code;			/* Instructions of function. */
};

/* A C function. */
typedef int (*function_c_t)(pseu_state_t *s, struct value *args);

/* A pseu function description. It can also represent a procedure; see
 * function.return_type.
 */
struct function {
	uint8_t type;			/* Type of function; see function_type. */
	const char *ident; 		/* Identifier of function. */

	uint8_t params_count; 		/* Number of parameters (arity).*/
	struct type **param_types;	/* Types of parameters. */
	struct type *return_type;	/* Return type; NULL when procedure. */

	union {
		function_c_t c;				/* As a C function. */
		struct function_pseu pseu;	/* As a pseu function. */
	} as;
};

/* A pseu call frame. */
struct frame {
	struct function *fn;	/* Function of that frame. */
	code_t *ip; 			/* Instruction pointer. */
	struct value *bp; 		/* Base of stack frame. */
};

/* Reference to the VM instance of state `S`. */
#define VM(S) (S)->vm

/* A per thread pseu state. */
struct pseu_state {
	pseu_vm_t *vm;			/* Reference to global VM state. */

	struct value *sp;		/* Stack pointer. */
	size_t stack_size;		/* Capacity of evaluation stack. */	
	struct value *stack; 	/* Evaluation stack; points to bottom. */

	size_t frames_count; 	/* Number of frames in the call frame stack. */
	size_t frames_size; 	/* Capacity of call frame stack. */
	struct frame *frames; 	/* Call frame stack; points to bottom. */
};

/* Global pseu virtual machine in a pseu instance. */
struct pseu_vm {
	pseu_state_t *state; 	/* Current state executing. */
	pseu_config_t config; 	/* Configuration of VM. */

	// XXX
	size_t types_count;
	struct type types[8];

	size_t fns_count;
	struct function fns[8];

	size_t vars_count;
	struct variable vars[8];
	// XXX
	
	struct type *any_type;
	struct type *real_type;
	struct type *integer_type;
	struct type *boolean_type;

	void *data; 			/* User attached data; pseu_vm_{set,get}_data(). */
	char *error;			/* Error message set; NULL when no error. */
};

#endif /* PSEU_OBJ_H */
