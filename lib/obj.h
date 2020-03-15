#ifndef PSEU_OBJ_H
#define PSEU_OBJ_H

#include <pseu.h>
#include <stdint.h>
#include <stdbool.h>

typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;

typedef float     f32;
typedef double    f64;

typedef size_t    size;

/* ** Forward references. ** */
typedef struct Type Type;
typedef struct Object Object;

typedef struct PseuVM VM;
typedef struct PseuState State;

/* Opcodes which the pseu virtual machine supports. */
enum code {
  #define _(x) OP_##x,
  #include "op.def" 
  #undef  _
};

/* Represents a pseu virutal machine instruction byte code. */
typedef u8 BCode;

/* Types of pseu arithmetics. */
typedef enum Arith {
  ARITH_ADD = '+',
  ARITH_SUB = '-',
  ARITH_MUL = '*',
  ARITH_DIV = '/'
} ArithType;

/* A pseu type field. */
typedef struct Field {
  const char *ident;		/* Identifier of field. */
  Type *type;		        /* Type of field. */
} Field;

#define t_isany(S, t) ((t) == VM(S)->any_type)
#define t_isint(S, t) ((t) == VM(S)->integer_type)

/* A pseu type. */
struct Type {
  u8 fields_count;      /* Number of fields of type. */
  Field *fields;        /* Fields of type. */
  const char *ident;    /* Identifier of type. */
};

/* Types of pseu value. */
typedef enum ValueType {
  VAL_NIL,              /* Empty. XXX: Reconsider. */
  VAL_BOOL,             /* Boolean. */
  VAL_INT,              /* Signed 32-bit integer. */
  VAL_FLOAT,            /* Single-precision floating point. */
  VAL_OBJ               /* Pointer to a heap allocated pseu object. */
} ValueType;

/* A pseu value. */
typedef struct Value {
  u8 type;              /* Type of value; see value_type. */
  union {
    float real;         /* As a real value. */
    bool boolean;       /* As a boolean. */
    i32 integer;        /* As an integer. */
    Object *object;     /* As an object. */
  } as;
} Value;

#define v_isbool(v)  ((v)->type == VAL_BOOL)
#define v_isobj(v)   ((v)->type == VAL_OBJ)
#define v_isint(v)   ((v)->type == VAL_INT)
#define v_isfloat(v) ((v)->type == VAL_FLOAT)
#define v_isnum(v)   (v_isint(v) || v_isfloat(v))

#define v_asbool(v)  ((v)->as.boolean)
#define v_asobj(v)   ((v)->as.object)
#define v_asint(v)   ((v)->as.integer)
#define v_asfloat(v) ((v)->as.real)

#define v_bool(k)    ((Value) {.type = VAL_BOOL,  .as.boolean = (k)})
#define v_obj(k)     ((Value) {.type = VAL_OBJ,   .as.object = (k)})
#define v_int(k)     ((Value) {.type = VAL_INT,   .as.integer = (k)})
#define v_float(k)   ((Value) {.type = VAL_FLOAT, .as.real = (k)})

#define v_i2f(v)     ((float)v_asint(v))
#define v_f2i(v)     ((int32_t)v_asfloat(v))

/* A pseu variable. */
typedef struct Variable {
  bool konst;             /* Is variable a constant. */
  const char *ident;      /* Identifier of variable. */
  Value value;            /* Value of variable. */
} Variable;

#define GC_HEADER u8 flags

/* A pseu user object. */
struct Object {
  GC_HEADER;
  Type *type;             /* Type of object. */
  Value fields[1];        /* Field values of the object. */
};

/* A pseu string object. */
typedef struct String {
  GC_HEADER;
  u32 hash;               /* Hash of string. */
  u32 length;	            /* Length of string. */
  u8 buffer[1];           /* Buffer containing string. */
} String;

/* A pseu array object. */
typedef struct Array {
  GC_HEADER;
  u32 start;              /* Start of array. */
  u32 end;                /* End of array. */
  Value *val[1];   /* Values of array. */
} Array;

/* A char buffer. */
typedef struct CBuffer {
  size_t count;           /* Number of chars in buffer. */
  size_t size;            /* Size/capacity of buffer. */
  char *buffer;           /* Pointer to buffer block. */
} CBuffer;

int cbuf_new(State *s, CBuffer *buf, size sz);
int cbuf_put(State *s, CBuffer *buf, char c);
void cbuf_free(State *s, CBuffer *buf);

/* Types of pseu function. */
typedef enum FunctionType {
  FN_PSEU,                /* Pseu function; consisting of VM bytecode. */
  FN_C                    /* Native C function which operates on a VM state
                           * evalulation stack. */
} FunctionType;

/* A pseu function. */
typedef struct FunctionPseu {
  u8 const_count;         /* Number of constants in `consts`. */
  u8 local_count;         /* Number of locals in `locals`.*/
  u16 code_count;         /* Number of instructions in `code`. */

  u32 max_stack;          /* Maximum space the function occupies on the stack. */

  Value *consts;          /* Constants in the function. */
  Type **locals;          /* Locals in the function. */
  BCode *code;            /* Instructions of function. */
} FunctionPseu;

/* A C function. */
typedef int (*FunctionC)(State *s, Value *args);

/* A pseu function description. It can also represent a procedure; see
 * function.return_type.
 */
typedef struct Function {
  u8 type;                /* Type of function; see function_type. */
  const char *ident;      /* Identifier of function. */

  u8 params_count;        /* Number of parameters (arity).*/
  Type **param_types;     /* Types of parameters. */
  Type *return_type;      /* Return type; NULL when procedure. */

  union {
    FunctionC c;          /* As a C function. */
    FunctionPseu pseu;    /* As a pseu function. */
  } as;
} Function;

/* A pseu call frame. */
typedef struct Frame {
  Function *fn;           /* Function of that frame. */
  BCode *ip;              /* Instruction pointer. */
  Value *bp;              /* Base of stack frame. */
} Frame;

/* Reference to the VM instance of state `S`. */
#define VM(S) ((S)->vm)
#define V(S)  ((S)->vm)

/* Reference to the state instance of VM `V`. */
#define S(V)  ((V)->state)

/* A per thread pseu state. */
typedef struct PseuState {
  VM *vm;                 /* Reference to global VM state. */

  Value *sp;              /* Stack pointer. */
  size stack_size;        /* Capacity of evaluation stack. */	
  Value *stack;           /* Evaluation stack; points to bottom. */

  size frames_count;      /* Number of frames in the call frame stack. */
  size frames_size;       /* Capacity of call frame stack. */
  Frame *frames;          /* Call frame stack; points to bottom. */
} PseuState;

/* Global pseu virtual machine in a pseu instance. */
struct PseuVM {
  PseuState *state;       /* Current state executing. */
  PseuConfig config;      /* Configuration of VM. */

  // XXX
  size fns_count;
  size vars_count;
  size types_count;

  Function fns[8];
  Variable vars[8];
  Type types[8];
  // XXX

  Type *any_type;
  Type *real_type;
  Type *integer_type;
  Type *boolean_type;

  char *error;            /* Error message set; NULL when no error. */
  void *data;             /* User attached data; pseu_vm_{set,get}_data(). */
};

Type *v_type(PseuState *s, Value *v);
#endif /* PSEU_OBJ_H */
