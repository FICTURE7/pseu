#ifndef PSEU_COMPILER_H
#define PSEU_COMPILER_H

#include <stdint.h>
#include <stdlib.h>

#include "pseu_value.h"
#include "pseu_lexer.h"
#include "pseu_parser.h"
#include "pseu_opcode.h"

/* Maximum number of constants in a function. */
#define MAX_CONSTS (1 << 8)
/* Maximum number of local variables in a function. */
#define MAX_LOCALS (1 << 8)

/* Represents an emitter to emit virtual machine bytecode/code_t. */
struct emitter {
	/* Number of instr in `code` block. */
	size_t count;
	/* Capacity of `code` block. */
	size_t size;
	/* Block of memory containing the code. */
	code_t *code; 
};

/* 
 * Represents a pseu source code compiler, which compiles source code to a
 * function. 
 */
struct compiler {
	/* State which owns this compiler instance. */
	struct state *state;

	/* Lexer of the compiler, used by the parser. */
	struct lexer lexer;
	/* Parser of the compiler. */
	struct parser parser;
	/* Emitter to emit virtual machine bytecode. */
	struct emitter emitter;
	/* Top level compiler; NULL if already top. */
	struct compiler *top;

	/* 
	 * Maximum size the function could occupy on the virtual machine evaluation
	 * stack.
	 */
	size_t stack_size;

	/* Number of constants in the function. */
	uint8_t consts_count;
	/* Array of constants in the function. */
	struct value consts[MAX_CONSTS];

	/* Number of locals in the function. */
	uint8_t locals_count;
	/* Array of locals in the function.*/
	struct variable locals[MAX_LOCALS];

	/* Function we're compiling. */
	struct function *fn;

	/* Number of errors during code gen process. */
	unsigned int error_count;
};

/*
 * Compiles the specified pseu source code to a function using the specified
 * compiler.
 *
 * @param[in] state State instance.
 * @param[in] compiler Compiler instance.
 * @param[in] fn Function which will be compiled.
 * @param[in] src Pseu source code to compile.
 *
 * @return 0 if success; otherwise 1.
 */
int compiler_compile(struct state *state, struct compiler *compiler,
				struct function *fn, const char *src);

#endif /* PSEU_COMPILER_H */
