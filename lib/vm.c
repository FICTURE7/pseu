#include "vm.h"
#include "obj.h"

/* TODO: Figure out some way to reduce the amount of code duplication in here.
 * Will probably end up with some kind of macro mayhem though.
 */

/* Do int32 arithmetic operation 'op' on value 'a' and 'b', outputs result in
 * value 'o'. 
 */
static i32 arith_i32(i32 a, i32 b, ArithType op)
{
  switch (op) {
  case ARITH_add: return a + b;
  case ARITH_sub: return a - b;
  case ARITH_mul: return a * b;
  case ARITH_div: return a / b;
  default:
    pseu_unreachable();
  }
}

/* Do float arithmetic operation 'op' on value 'a' and 'b', outputs result in
 * value 'o'. 
 */
static f32 arith_f32(f32 a, f32 b, ArithType op)
{
  switch (op) {
  case ARITH_add: return a + b;
  case ARITH_sub: return a - b;
  case ARITH_mul: return a * b;
  case ARITH_div: return a / b;
  default:
    pseu_unreachable();
  }
}

/* Do arithmetic operation 'op' on value 'a' and 'b', outputs result in value
 * 'o'. 
 */
static void arith_num(Value *a, Value *b, Value *o, ArithType op)
{
  if (v_isi32(a) && v_isi32(b)) {
    i32 ia = v_asi32(a);
    i32 ib = v_asi32(b);

    *o = v_i32(arith_i32(ia, ib, op));
  } else {
    float fa;
    float fb;

    if (v_isf32(a) && v_isf32(b)) {
      fa = v_asf32(a);
      fb = v_asf32(b);
    } else {
      /* Convert both values to float. */
      fa = v_isi32(a) ? v_i2f(a) : v_asf32(a);
      fb = v_isi32(b) ? v_i2f(b) : v_asf32(b);
    }

    *o = v_float(arith_f32(fa, fb, op));
  }
}

static bool compare_i32(i32 a, i32 b, CompareType op)
{
  switch (op) {
  case COMP_lt: return a < b;
  case COMP_gt: return a > b;
  case COMP_le: return a <= b;
  case COMP_ge: return a >= b;
  case COMP_eq: return a == b;
  default:
    pseu_unreachable();
  }
}

static bool compare_f32(f32 a, f32 b, CompareType op)
{
  switch (op) {
  case COMP_lt: return a < b;
  case COMP_gt: return a > b;
  case COMP_le: return a <= b;
  case COMP_ge: return a >= b;
  case COMP_eq: return a == b;
  default:
    pseu_unreachable();
  }
}

static void compare_num(Value *a, Value *b, Value *o, CompareType op)
{
  if (v_isi32(a) && v_isi32(b)) {
    int ia = v_asi32(a);
    int ib = v_asi32(b);

    *o = v_bool(compare_i32(ia, ib, op));
  } else {
    float fa;
    float fb;

    if (v_isf32(a) && v_isf32(b)) {
      fa = v_asf32(a);
      fb = v_asf32(b);
    } else {
      /* Convert both values to float. */
      fa = v_isi32(a) ? v_i2f(a) : v_asf32(a);
      fb = v_isi32(b) ? v_i2f(b) : v_asf32(b);
    }

    *o = v_bool(compare_f32(fa, fb, op));
  }
}

/* Attempts to coerce the specified value to a numeric one (float, int). */
static int coerce_num(Value *i, Value *o)
{
  if (v_isnum(i)) {
    *o = *i;
    return 0;
  }

  /* TODO: Convert string objects to integers when they are implemented. */
  return 1;
}

/* Ensures that the specified number of slots 'n' is available. */
static int ensure_stack(State *s, size n)
{
  size k = s->sp - (s->stack + s->stack_size);

  if (k >= n)
    return 0;

  /* TODO: Grow to the power 2 number closest to n. (or something) 
   * Should also be careful to update frame pointers and stuff. */
  pseu_assert(0);
  return 1;
}

/* Initialize stack space for the specified frame. */
static int init_stack(State *s, Frame *frame)
{
  Function *fn = frame->fn;
  pseu_assert(fn->type == FN_PSEU);

  for (size i = 0; i < fn->as.pseu.local_count; i++) {
    Type *loc_type = fn->as.pseu.locals[i];
    if (t_isany(s, loc_type))
      continue;

    if (t_isint(s, loc_type))
      frame->bp[i] = v_int(0);
    else
      return 1;
  }

  return 0;
}

/* Appends the specified function as a call frame to the call stack. */
static int append_call(State *s, Function *fn)
{
  if (s->frames_count >= s->frames_size)
    pseu_vec_grow(s, s->frames, &s->frames_size, Frame);

  s->frames[s->frames_count] = (Frame) {
    .fn = fn,
    .ip = fn->as.pseu.code,
    .bp = s->sp,
  };

  init_stack(s, &s->frames[s->frames_count]);
  s->frames_count++;
  return 0;
}

/* Dispatches the last frame on the call stack. */
static int dispatch(State *s) 
{
  #define READ_U8()  	(*ip++)
  #define READ_U16() 	(ip += 2, (u16)(ip[-2] << 8 | ip[-1]))

  #define PUSH(x) 		*s->sp++ = x
  #define POP(x)  		(*(--s->sp))

  #ifdef PSEU_USE_COMPUTEDGOTO
    #error Computed gotos are not implemented yet
  #else
    #define INTERPRET  \
      BCode op; 			 \
      decode: 				 \
      switch ((op = READ_U8()))
    #define DISPATCH_EXIT(c) 	return c
    #define DISPATCH() 			  goto decode
    #define OP(x) 				    case OP_##x
  #endif

  Frame *frame = &s->frames[s->frames_count - 1];
  BCode *ip = frame->ip;
  Function *fn = frame->fn;

  INTERPRET {
    OP(LD_CONST): {
      u8 index = READ_U8(); 

      PUSH(fn->as.pseu.consts[index]);
      DISPATCH();
    }
    OP(LD_GLOBAL): {
      u16 index = READ_U16(); 

      PUSH(V(s)->vars[index].value);
      DISPATCH();
    }
    OP(LD_LOCAL): {
      u8 index = READ_U8();

      PUSH(*(frame->bp + index));
      DISPATCH();
    }
    OP(ST_LOCAL): {
      u8 index = READ_U8();

      /* TODO: Type checking. */
      *(frame->bp + index) = POP();
      DISPATCH();
    }
    OP(BR): {
      u16 index = READ_U16();

      ip = &fn->as.pseu.code[index];
      DISPATCH();
    }
    OP(BR_FALSE): {
      u16 index = READ_U16();

      if (!POP().as.boolean)
        ip = &fn->as.pseu.code[index];
      DISPATCH();
    }
    OP(CALL): {
      u16 index = READ_U16();
      Function *f = &V(s)->fns[index];

      pseu_assert((s->sp - frame->bp) >= f->params_count);

      if (f->type == FN_C) {
        f->as.c(s, s->sp - f->params_count);	
        
        if (f->return_type != NULL)
          s->sp -= f->params_count - 1;
        else
          s->sp -= f->params_count;
      } else if (f->type == FN_PSEU) {
        pseu_unreachable();
      } else {
        pseu_unreachable();
      }
      DISPATCH();
    }
    OP(RET): {
      DISPATCH_EXIT(0);
    }
  }
  
  /* Check if we need to do a garbage collection. */
  if (pseu_gc_poll(s))
    pseu_gc_collect(s);

  return 1;
}

State *pseu_state_new(VM *vm)
{
  State *s = (State *)vm->config.alloc(vm, sizeof(*s));

  if (!s)
    return NULL;
  
  s->vm = vm;
  s->stack_size = PSEU_INIT_EVALSTACK_SIZE;
  s->frames_count = 0;
  s->frames_size = PSEU_INIT_CALLSTACK_SIZE;

  if (pseu_vec_init(s, &s->frames, s->frames_size, Frame))
    goto free_frames;
  if (pseu_vec_init(s, &s->stack, s->stack_size, Value))
    goto free_stack;

  s->sp = s->stack;
  return s;

free_stack:
  pseu_free(s, s->stack);
free_frames:
  pseu_free(s, s->frames);
  pseu_free(s, s);
  return NULL;
}

void pseu_state_free(State *s)
{
  pseu_free(s, s->stack);
  pseu_free(s, s->frames);
  pseu_free(s, s);
}

int pseu_call(State *s, Function *fn) 
{
  pseu_assert(s->sp - s->stack >= fn->params_count);
  pseu_assert(fn->type == FN_PSEU);

  if (pseu_unlikely(ensure_stack(s, fn->as.pseu.max_stack)))
    return 1;
  if (pseu_unlikely(append_call(s, fn)))
    return 1;
  return dispatch(s);
}

void *pseu_alloc(State *s, size sz) 
{
  return V(s)->config.alloc(V(s), sz); /* XXX: Handle out of memory. */
}

void *pseu_realloc(State *s, void *ptr, size sz) 
{
  return V(s)->config.realloc(V(s), ptr, sz);
}

void pseu_free(State *s, void *ptr) 
{
  V(s)->config.free(V(s), ptr);
}

void pseu_print(State *s, const char *text) 
{
  V(s)->config.print(V(s), text);
}

void pseu_panic(State *s, const char *message)
{
  // TODO: Jump to somewhere safe. longjmp and stuff.
  V(s)->config.panic(V(s), message);
}

char *pseu_strdup(State *s, const char *str)
{
  size len = strlen(str);
  char *nstr = pseu_alloc(s, len);
  if (pseu_unlikely(!nstr))
    return NULL;

  memcpy(nstr, str, len);
  nstr[len] = '\0';
  return nstr;
}

int _pseu_vec_init(State *s, void **vec, size cap_elm, size sz_elm)
{
  void *new_vec = pseu_alloc(s, cap_elm * sz_elm);
  if (pseu_unlikely(!new_vec))
    return 1;

  *vec = new_vec;
  return 0;
}

int _pseu_vec_grow(State *s, void **vec, size *cap_elm, size sz_elm)
{
  size new_cap_elm = (*cap_elm * 2);
  void *new_vec = pseu_realloc(s, *vec, new_cap_elm * sz_elm);
  if (pseu_unlikely(!new_vec))
    return 1;

  *vec = new_vec;
  *cap_elm = new_cap_elm;
  return 0;
}

int pseu_arith_binary(Value *a, Value *b, Value *o, ArithType op)
{
  if (v_isnum(a) && v_isnum(b)) {
    arith_num(a, b, o, op);
    return 0;
  }

  Value na;
  Value nb; 
  if (coerce_num(a, &na) || coerce_num(b, &nb))
    return 1;

  arith_num(&na, &nb, o, op);
  return 0;
}

int pseu_compare_binary(Value *a, Value *b, Value *o, CompareType op)
{
  if (v_isnum(a) && v_isnum(b)) {
    compare_num(a, b, o, op);
    return 0;
  }

  Value na;
  Value nb;
  if (coerce_num(a, &na) || coerce_num(b, &nb))
    return 1;

  compare_num(&na, &nb, o, op);
  return 0;
}

u16 pseu_def_type(VM *vm, Type *type)
{
  u16 result = vm->types_count++;

  vm->types[result] = *type;
  return result;
}

u16 pseu_def_function(VM *vm, Function *fn)
{
  u16 result = vm->fns_count++;

  vm->fns[result] = *fn;
  return result;
}

u16 pseu_def_variable(VM *vm, Variable *var)
{
  u16 result = vm->vars_count++;

  vm->vars[result] = *var;
  return result;
}

u16 pseu_get_type(VM *vm, const char *ident, size len)
{
  for (size i = 0; i < vm->types_count; i++) {
    if (strncmp(vm->types[i].ident, ident, len) == 0)
      return i;
  }

  return PSEU_INVALID_TYPE;
}

u16 pseu_get_function(VM *vm, const char *ident, size len)
{
  for (size i = 0; i < vm->fns_count; i++) {
    if (strncmp(vm->fns[i].ident, ident, len) == 0)
      return i;
  }

  return PSEU_INVALID_FUNC;
}

u16 pseu_get_variable(VM *vm, const char *ident, size len)
{
  for (size i = 0; i < vm->vars_count; i++) {
    if (strncmp(vm->vars[i].ident, ident, len) == 0)
      return i;
  }

  return PSEU_INVALID_GLOBAL;
}
