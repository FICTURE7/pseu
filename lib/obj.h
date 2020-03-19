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
typedef union  Object Object;
typedef struct State State;

typedef struct PseuVM VM;

/* Opcodes which the pseu virtual machine supports. */
enum OpCode {
  #define _(x) OP_##x,
  #include "op.def" 
  #undef  _
};

/* Represents a pseu virutal machine instruction byte code. */
typedef u8 BCode;

/* NOTE: The extra parameters (a, b, c, o, T) are used in vm.c */
#define PSEU_ARITH_TYPES(_, a, b, o, T) \
  _(add, +, a, b, o, T)                 \
  _(sub, -, a, b, o, T)                 \
  _(mul, *, a, b, o, T)                 \
  _(div, /, a, b, o, T)

/* Types of pseu arithmetics. */
typedef enum ArithType {
  #define ARITH_TYPE(n, _, __, ___, ____, _____) ARITH_##n,
  PSEU_ARITH_TYPES(ARITH_TYPE, _, __, ___, ____)
  #undef  ARITH_TYPE
} ArithType;

/* NOTE: The extra parameters (a, b, c, o) are used in vm.c */
#define PSEU_COMP_TYPES(_, a, b, o)     \
  _(lt, <,  a, b, o)                    \
  _(gt, >,  a, b, o)                    \
  _(le, <=, a, b, o)                    \
  _(ge, >=, a, b, o)                    \
  _(eq, ==, a, b, o)

/* Types of pseu compares. */
typedef enum CompareType {
  #define COMP_TYPE(n, _, __, ___, ____) COMP_##n,
  PSEU_COMP_TYPES(COMP_TYPE, _, __, ___)
  #undef  COMP_TYPE
} CompareType;

/* A pseu type field. */
typedef struct Field {
  const char *ident;		/* Identifier of field. */
  Type *type;		        /* Type of field. */
} Field;

#define t_isany(S, t)   ((t) == V(S)->any_type)
#define t_isint(S, t)   ((t) == V(S)->integer_type)
#define t_isfloat(S, t) ((t) == V(S)->real_type)
#define t_isarray(S, t) ((t) == V(S)->array_type)

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
#define v_isi32(v)   v_isint(v)
#define v_isf32(v)   v_isfloat(v)

#define v_asbool(v)  ((v)->as.boolean)
#define v_asobj(v)   ((v)->as.object)
#define v_asint(v)   ((v)->as.integer)
#define v_asfloat(v) ((v)->as.real)
#define v_asi32(v)   v_asint(v)
#define v_asf32(v)   v_asfloat(v)

#define v_bool(k)    ((Value) {.type = VAL_BOOL,  .as.boolean = (k)})
#define v_obj(k)     ((Value) {.type = VAL_OBJ,   .as.object = (k)})
#define v_int(k)     ((Value) {.type = VAL_INT,   .as.integer = (k)})
#define v_float(k)   ((Value) {.type = VAL_FLOAT, .as.real = (k)})
#define v_i32(k)     v_int(k)
#define v_f32(k)     v_float(k)

#define v_i2f(v)     ((float)v_asint(v))
#define v_f2i(v)     ((int32_t)v_asfloat(v))

/* A pseu variable. */
typedef struct Variable {
  bool konst;             /* Is variable a constant. */
  const char *ident;      /* Identifier of variable. */
  Value value;            /* Value of variable. */
} Variable;

#define GC_HEADER u8 marked; Type *type; Object* next

/* A pseu user object. */
typedef struct UObject {
  GC_HEADER;
  Value fields[];        /* Field values of the object. */
} UObject;

/* A pseu string object. */
typedef struct String {
  GC_HEADER;
  u32 length;	            /* Length of string. */
  u8 buffer[];            /* Buffer containing string. */
} String;

String *string_new(State *s, const char* value);

/* A pseu array object. */
typedef struct Array {
  GC_HEADER;
  u32 length;             /* Length of array. */
  u32 capacity;           /* Capacity of array. */
  Value items[];          /* Values of array. */
} Array;

Array *array_new(State *s, u32 cap);
Array *array_push(State *s, Array* a, Value *v);
Array *array_pop(State *s, Array *a);
Value *array_get(State *s, Array *a, u32 i);

/* A pseu user object. */
union Object {
  struct { GC_HEADER; } header;  /* Generic fields of an object. */
  union {
    UObject uobject;      /* As a user object. */
    String string;        /* As a string object. */
    Array array;          /* As an array. */
  } as;
};

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
/* @deprecated */
#define VM(S) ((S)->vm)
#define V(S)  ((S)->vm)

/* Reference to the state instance of VM `V`. */
#define S(V)  ((V)->state)

/* A per thread pseu state. */
struct State {
  VM *vm;                 /* Reference to owner VM state. */

  Value *sp;              /* Stack pointer. */
  size stack_size;        /* Capacity of evaluation stack. */	
  Value *stack;           /* Evaluation stack; points to bottom. */

  size frames_count;      /* Number of frames in the call frame stack. */
  size frames_size;       /* Capacity of call frame stack. */
  Frame *frames;          /* Call frame stack; points to bottom. */
};

/* A pseu garbage collector state. */
typedef struct GC {
  VM *vm;                 /* Reference to owner VM state. */

  Object *objects;        /* Linked-list of allocated objects. */
  Object *root;           /* Reference to GC root. */
} GC;

/* Global VM instance in a pseu instance. */
struct PseuVM {
  // TODO: Turn these into an Array object when we are up and running.
  u16 fns_count;
  u16 vars_count;
  u16 types_count;

  Function fns[16];
  Variable vars[16];
  Type types[16];
  // XXX
  //

  GC gc;                  /* Garbage collector of VM instance. */
  State *state;           /* Current state executing. */

  /* TODO: Wrap this in a struct called `primitives`. */
  Type *any_type;
  Type *real_type;
  Type *integer_type;
  Type *boolean_type;
  Type *array_type;

  char *error;            /* Error message set; NULL when no error. */
  void *data;             /* User attached data; pseu_vm_{set,get}_data(). */
  PseuConfig config;      /* Configuration of VM. */
};

/* @deprecated Use v_get_type(s, v) instead. */
Type *v_type(State *s, Value *v);
/* Slowly start using this one through the code base. */
#define v_get_type(s, v) v_type(s, v)
#endif /* PSEU_OBJ_H */
