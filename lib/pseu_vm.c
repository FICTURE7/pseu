#include <stdint.h>
#include <string.h>
#include <assert.h>

#include "pseu_vm.h"
#include "pseu_value.h"
#include "pseu_debug.h"

/*
 * Ensures that there is the specified amount of free slots available on the
 * evaluation stack of the specified state, growing the stack if needed.
 */
static int vm_stack_ensure_free(struct state *state, size_t num_slot) {
	pseu_assert(state);

	/* If there is more space than requested, return 0. */
	if ((state->stack + state->stack_size) - state->sp >= num_slot) {
		return 0;
	}

	/* TODO: Implement. */
	return 1;
}

/*
 * Appends a call frame to the call stack of the specified state for the 
 * specified function closure.
 */
static void vm_append_call(struct state *state, struct closure *closure, bool init_stack) {
	pseu_assert(state && closure);
	
	/* Check if need to grow call frame stack. */
	if (state->frames_count >= state->frames_size) {
		size_t new_frames_size = state->frames_size * 2;
		struct frame *frames = pseu_realloc(state->vm, state->frames,
									new_frames_size);
		state->frames_size = new_frames_size;
		state->frames = frames;
	}

	/* Append call frame. */
	struct frame frame = {
		.closure = closure,
		.ip = closure->code,
		.bp = state->sp - closure->fn->params_count
	};

	if (init_stack) {
		/* Clean stack space containing closure locals. */
		memset(frame.bp, 0, closure->locals_count * sizeof(struct value));
		/* Initialize local variables types. */
		for (size_t i = 0; i < closure->locals_count; i++) {
			struct value *local = &frame.bp[i];
			struct variable *var = &closure->locals[i];

			if (var->type == &state->vm->void_type) {
				local->type = VALUE_TYPE_VOID;
			} else if (var->type == &state->vm->integer_type) {
				local->type = VALUE_TYPE_INTEGER;
			} else {
				/* TODO: Handle error. */
			}
		}
	}

	state->frames[state->frames_count++] = frame;
	state->sp += closure->locals_count;
}

/*
 * Dispatch loop of pseu, which is heavily inspired by wren's dispatch loop.
 */
static int vm_dispatch(struct state *state) {	
	pseu_assert(state);

	struct frame *frame;
	struct closure *closure;
	code_t *ip;

	/* Push a value on top the stack. */
	#define PUSH(x) (*state->sp++ = x)
	/* Pops a value from the top of the stack. */
	#define POP() (*(--state->sp))
	/* Discards a vaulue from the top of the stack. */
	#define DISCARD() (--state->sp)

	/* Load last call frame of the state. */
	#define LOAD_FRAME() \
		frame = &state->frames[state->frames_count - 1]; \
		closure = frame->closure; \
		ip = frame->ip

	/* Update the instruction pointer of the frame. */
	#define STORE_FRAME() \
		frame->ip = ip

	/* Reads a uint8 at the instruction pointer and advances it by 1. */
	#define READ_UINT8() (*ip++)
	/* Reads a uint16 at the instruction pointer and advances it by 2. */
	#define READ_UINT16() (ip += 2, (uint16_t)((ip[-2] << 8) | ip[-1]))

	#ifdef PSEU_USE_COMPUTEDGOTO
		#error "Computed gotos are not implemented."
	#else
		#define DECODE() \
			code_t op; \
			loop: \
				op = READ_UINT8(); \
				switch (op)

		#define CASE(x) case VM_OP_##x
		#define DISPATCH()	goto loop
	#endif

	/* Expect atleast 1 call frame on the call frame stack. */
	pseu_assert(state->frames_count > 0);
	/* Load the last call frame. */
	LOAD_FRAME();

	/* 
	 * Ensures that there is enough space on the evaluation stack to execute
	 * function.
	 *
	 * Return early if fail to ensure space.
	 */
	if (vm_stack_ensure_free(state, closure->stack_size)) {
		return 1;
	}

	/* Dispatch loop. */
	DECODE() {
		CASE(CALL): {
			/* Lookup function to call in global symbol table. */
			uint16_t fn_id = READ_UINT16();
			struct function *fn = state->vm->symbols.fns.data[fn_id];

			/* 
			 * Expect that there is enough space on the stack to call the
			 * function.
			 */
			pseu_assert(state->sp - state->stack >= fn->params_count);

			/* Perform argument type checking. */
			struct value *args = state->sp - fn->params_count;
			for (size_t i = 0; i < fn->params_count; i++) {
				struct type *param_type = fn->params_types[i];
				/* 
				 * If the parameter type is `VOID`, it means argument can be of 
				 * any type.
				 */
				if (param_type == &state->vm->void_type) {
					continue;
				}

				struct type *value_type = value_get_type(state->vm, &args[i]);
				pseu_assert(value_type);

				/*
				 * Otherwise check if the type of the value on the stack matches
				 * the type of the parameter.
				 */
				if (value_type != param_type) {
					/* TODO: Set error message. */
					return 1;
				}
			}

			/* Handle function call according to function type. */
			switch (fn->type) {
				case FN_TYPE_PRIMITIVE: {
					/* TODO: Handle primitive function errors. */

					/* Call primitive C function. */
					fn->as_primitive(state, args);
					/* 
					 * Pop function arguments from stack. If procedure pop all
					 * arguments, otherwise if function, pop all arguments,
					 * except last value which is the return value.
					 */
					state->sp -= fn->return_type == NULL ? 
								fn->params_count : fn->params_count - 1;
					break;
				}
				case FN_TYPE_USER: {
					/* TODO: Ensure stack size. */
					/* Store the instruction pointer into the current frame. */
					STORE_FRAME();
					/* Append the call to the function to the call stack. */
					vm_append_call(state, fn->as_closure, false);
					/* Load the appended call frame and continue execution. */
					LOAD_FRAME();
					break;
			   }
			}
			DISPATCH();
		}

		CASE(RET): {
			struct function *fn = closure->fn;
			struct value *return_value;

			/* 
			 * If function, check if return value type matches function 
			 * description.
			 */
			if (fn->return_type != NULL) {
				return_value = &POP();

				struct type *return_value_type = value_get_type(state->vm,
										return_value);
				if (fn->return_type != &state->vm->void_type &&
					fn->return_type != return_value_type) {
					/* TODO: Error, incorrect return type. */
					return 1;
				}
			}

			state->sp = frame->bp;

			if (closure->fn->return_type != NULL) {
				PUSH(*return_value);
			}

			/* Decrement frame count and load previous frame. */
			state->frames_count--;
			LOAD_FRAME();

			/* If error message set, return 1. */
			if (state->error) {
				return 1;
			}
			DISPATCH();
		}

		CASE(LD_CONST): {
			uint8_t index = READ_UINT8();
			pseu_assert(index < closure->consts_count);
			struct value *value = &closure->consts[index];
			
			/* Push constant's value ontop on the stack. */
			PUSH(*value);
			DISPATCH();
		}

		CASE(LD_LOCAL): {
			uint8_t index = READ_UINT8();
			pseu_assert(index < closure->locals_count);
			struct value *local = &frame->bp[index];

			/* Local's type was not initialized. */
			if (local->type == VALUE_TYPE_VOID) {
				/* TODO: Set error. */
				return 1;
			}
			
			/* Push local's value ontop of the stack. */
			PUSH(*local);
			DISPATCH();
		}

		CASE(ST_LOCAL): {
			uint8_t index = READ_UINT8();
			pseu_assert(index < closure->locals_count);

			struct value *value = &POP();
			struct variable *local = &closure->locals[index];

			/* Check if value can be stored into the variable. */
			if (local->type != &state->vm->void_type && 
				local->type != value_get_type(state->vm, value)) {
				/* TODO: Set error. */
				return 1;
			}

			/* Store value into local variable. */
			frame->bp[index] = *value;
			DISPATCH();
		}

		CASE(LD_GLOBAL): {
			uint16_t index = READ_UINT16();
			DISPATCH();
		}

		CASE(ST_GLOBAL): {
			uint16_t index = READ_UINT16();

			/* TODO: Perform type checking. */
			DISPATCH();
		}

		CASE(END): {
			/* Exit the dispatch loop. */
			return 0;
		}
	}

	return 0;
}

int vm_call(struct state *state, struct function *fn) {
	pseu_assert(state && fn);
	/* Expect Function to be a user-defined function. */
	pseu_assert(fn->type == FN_TYPE_USER);
	/*
	 * Expect that there are enough values on the evaluation stack to use as the
	 * function's arguments.
	 */
	pseu_assert(state->sp - state->stack >= fn->params_count);

	/* Append the call to the virtual machine state call stack. */
	vm_append_call(state, fn->as_closure, true);
	/* Run virtual machine dispatch loop and execute function. */
	return vm_dispatch(state);
}

void *pseu_alloc(pseu_vm_t *vm, size_t size) {
	pseu_assert(vm);
	return vm->config.alloc(vm, size);
}

void *pseu_realloc(pseu_vm_t *vm, void *ptr, size_t size) {
	pseu_assert(vm && ptr);
	return vm->config.realloc(vm, ptr, size);
}

void pseu_free(pseu_vm_t *vm, void *ptr) {
	pseu_assert(vm && ptr);
	return vm->config.free(vm, ptr);
}

void pseu_print(pseu_vm_t *vm, const char *text) {
	pseu_assert(vm);
	vm->config.print(vm, text);
}
