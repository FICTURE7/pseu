#include "vm.h"

/* Do int32 arithmetic operation 'op' on value 'a' and 'b', outputs result in
 * value 'o'. 
 */
static void arith_int(struct value *a, struct value *b, struct value *o, int op)
{
	int vr;
	switch (op) {
	case ARITH_ADD:
	  	vr = v_asint(a) + v_asint(b);
		break;
	case ARITH_SUB:
		vr = v_asint(a) - v_asint(b);
		break;
	case ARITH_MUL:
		vr = v_asint(a) * v_asint(b);
		break;
	case ARITH_DIV:
		vr = v_asint(a) / v_asint(b);
		break;
	default:
		pseu_unreachable();
		break;
	}
	o->type = VAL_INT;
	o->as.integer = vr;
}

/* Do float arithmetic operation 'op' on value 'a' and 'b', outputs result in
 * value 'o'. 
 */
static void arith_float(struct value *a, struct value *b,
		struct value *o, int op)
{
	float vr;
	switch (op) {
	case ARITH_ADD:
		vr = v_asfloat(a) + v_asfloat(b);
		break;
	case ARITH_SUB:
		vr = v_asfloat(a) - v_asfloat(b);
		break;
	case ARITH_MUL:
		vr = v_asfloat(a) * v_asfloat(b);
		break;
	case ARITH_DIV:
		vr = v_asfloat(a) / v_asfloat(b);
		break;
	default:
		pseu_unreachable();
		break;
	}
	o->type = VAL_FLOAT;
	o->as.real = vr;
}

/* Do arithmetic operation 'op' on value 'a' and 'b', outputs result in value
 * 'o'. 
 */
static void arith_num(struct value *a, struct value *b,
		struct value *o, int op)
{
	if (v_isint(a) && v_isint(b)) {
		arith_int(a, b, o, op);
	} else if (v_isfloat(a) && v_isfloat(b)) {
		arith_float(a, b, o, op);
	} else {
		struct value na;
		struct value nb;
		na.as.real = v_isint(a) ? v_i2f(a) : v_asfloat(a);
		nb.as.real = v_isint(b) ? v_i2f(b) : v_asfloat(b);
		/* No need to set type of 'na' and 'nb' since arith_float doesn't type
		 * check.
		 */
		arith_float(&na, &nb, o, op);
	}
}

/* Attempts to coerce the specified value to a numeric one (float, int). */
static int coerce_numeric(struct value *i, struct value *o)
{
	if (v_isnum(i)) {
		*o = *i;
		return 0;
	}
	return 1;
}

/* Ensures that the specified number of slots 'n' is available. */
static int vm_ensure_stack(pseu_state_t *s, size_t n)
{
	size_t k = s->sp - (s->stack + s->stack_size);
	if (k >= n)
		return 0;
	/* TODO: Grow to the power 2 number closest to n. (or something) */
	return 1;
}

/* Appends the specified function as a call frame to the call stack. */
static int vm_append_call(pseu_state_t *s, struct function *fn)
{
	size_t i;
	struct frame frame = {
		.fn = fn,
		.ip = fn->as.pseu.code,
		.bp = s->sp,
	};

	if (s->frames_count >= s->frames_size)
		pseu_vec_grow(s, s->frames, &s->frames_size, struct frame);
	s->frames[s->frames_count++] = frame;

	// XXX: Move this to another function.
	for (i = 0; i < fn->as.pseu.local_count; i++) {
		struct type *loc_type = fn->as.pseu.locals[i];
		if (t_isany(s, loc_type))
			continue;
		else if (t_isint(s, loc_type))
			frame.bp[i] = v_int(0);
		else
			return 1;
	}
	return 0;
}

/* Dispatches the last frame on the call stack. */
static int vm_dispatch(pseu_state_t *s) 
{
	#define READ_UINT8()  	(*ip++)
	#define READ_UINT16() 	(*ip++) /* FIXME: Temp solution for now. */

	#define PUSH(x) 		*s->sp++ = x
	#define POP(x)  		*(--s->sp)

	#ifdef PSEU_USE_COMPUTEDGOTO
		#error Computed gotos are not implemented yet
	#else
		#define INTERPRET 				\
			code_t op; 					\
			decode: 					\
			switch ((op = READ_UINT8()))
		#define DISPATCH_EXIT(c) 	return c;
		#define DISPATCH() 			goto decode
		#define OP(x) 				case OP_##x
	#endif

	struct frame *frame = &s->frames[s->frames_count - 1];
	code_t *ip = frame->ip;
	struct function *fn = frame->fn;

	INTERPRET {
		OP(LD_CONST): {
			uint16_t index = READ_UINT16(); 
			PUSH(fn->as.pseu.consts[index]);
			DISPATCH();
		}
		OP(LD_GLOBAL): {
			uint16_t index = READ_UINT16(); 
			PUSH(VM(s)->vars[index].value);
			DISPATCH();
		}
		OP(LD_LOCAL): {
			uint8_t index = READ_UINT8();
			PUSH(*(frame->bp + index));
			DISPATCH();
		}
		OP(CALL): {
			uint16_t index = READ_UINT16();
			struct function *f = &VM(s)->fns[index];
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
	return 1;
}

pseu_state_t *pseu_state_new(pseu_vm_t *vm)
{
	pseu_state_t *s = vm->config.alloc(vm, sizeof(*s));
	if (!s) return NULL;
	s->vm = vm;
	s->stack_size 	= PSEU_INIT_EVALSTACK_SIZE;
	s->frames_count = 0;
	s->frames_size 	= PSEU_INIT_CALLSTACK_SIZE;
	if (pseu_vec_init(s, &s->frames, s->frames_size, struct frame))
		goto free_frames;
	if (pseu_vec_init(s, &s->stack, s->stack_size, struct value))
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

void pseu_state_free(pseu_state_t *s)
{
	pseu_free(s, s->stack);
	pseu_free(s, s->frames);
	pseu_free(s, s);
}

int pseu_call(pseu_state_t *s, struct function *fn) 
{
	pseu_assert(s->sp - s->stack >= fn->params_count);
	pseu_assert(fn->type == FN_PSEU);

	if (pseu_unlikely(vm_ensure_stack(s, fn->as.pseu.max_stack)))
		return 1;
	if (pseu_unlikely(vm_append_call(s, fn)))
		return 1;
	return vm_dispatch(s);
}

void *pseu_alloc(pseu_state_t *s, size_t size) 
{
	return VM(s)->config.alloc(VM(s), size); /* XXX: Handle out of memory. */
}

void *pseu_realloc(pseu_state_t *s, void *ptr, size_t size) 
{
	return VM(s)->config.realloc(VM(s), ptr, size);
}

void pseu_free(pseu_state_t *s, void *ptr) 
{
	VM(s)->config.free(VM(s), ptr);
}

void pseu_print(pseu_state_t *s, const char *text) 
{
	VM(s)->config.print(VM(s), text);
}

void pseu_panic(pseu_state_t *s, const char *message)
{
	VM(s)->config.panic(VM(s), message);
}

char *pseu_strdup(pseu_state_t *s, const char *str)
{
	size_t len = strlen(str);
	char *nstr = pseu_alloc(s, len);
	if (pseu_likely(nstr)) {
		memcpy(nstr, str, len);
		nstr[len] = '\0';
	}
	return nstr;
}

int _pseu_vec_init(pseu_state_t *s, void **vec, size_t cap_elm, size_t size_elm)
{
	void *new_vec = pseu_alloc(s, cap_elm * size_elm);
	if (pseu_unlikely(!new_vec))
		return 1;
	*vec = new_vec;
	return 0;
}

int _pseu_vec_grow(pseu_state_t *s, void **vec, size_t *cap_elm, size_t size_elm)
{
	size_t new_cap_elm = (*cap_elm * 2);
	void *new_vec = pseu_realloc(s, *vec, new_cap_elm * size_elm);
	if (pseu_unlikely(!new_vec))
		return 1;
	*vec = new_vec;
	*cap_elm = new_cap_elm;
	return 0;
}

int pseu_arith_binary(struct value *a, struct value *b, struct value *o, int op)
{
 	if (v_isnum(a) && v_isnum(b)) {
		arith_num(a, b, o, op);
	} else {
		struct value na;
		struct value nb; 
		if (coerce_numeric(a, &na) || coerce_numeric(b, &nb))
			return 1;
		arith_num(&na, &nb, o, op);
	}
	return 0;
}

uint16_t pseu_def_type(pseu_vm_t *vm, struct type *type)
{
	uint16_t result = vm->types_count++;
	vm->types[result] = *type;
	return result;
}

uint16_t pseu_def_function(pseu_vm_t *vm, struct function *fn)
{
	uint16_t result = vm->fns_count++;
	vm->fns[result] = *fn;
	return result;
}

uint16_t pseu_def_variable(pseu_vm_t *vm, struct variable *var)
{
	uint16_t result = vm->vars_count++;
	vm->vars[result] = *var;
	return result;
}

// XXX
struct type *pseu_get_type(pseu_vm_t *vm, const char *ident, size_t len)
{
	size_t i;
	for (i = 0; i < vm->types_count; i++) {
		if (strncmp(vm->types[i].ident, ident, len) == 0)
			return &vm->types[i];
	}
	return NULL;
}

uint16_t pseu_get_function(pseu_vm_t *vm, const char *ident)
{
	size_t i;
	for (i = 0; i < vm->fns_count; i++) {
		if (strcmp(vm->fns[i].ident, ident) == 0)
			return i;
	}
	return PSEU_INVALID_FUNC;
}

uint16_t pseu_get_variable(pseu_vm_t *vm, const char *ident, size_t len)
{
	size_t i;
	for (i = 0; i < vm->vars_count; i++) {
		if (strncmp(vm->vars[i].ident, ident, len) == 0)
			return i;
	}
	return PSEU_INVALID_GLOBAL;
}
// XXX
