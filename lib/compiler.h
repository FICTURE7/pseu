#ifndef COMPILER_H
#define COMPILER_H

#include <stdint.h>
#include <stdlib.h>

#include "value.h"
#include "lexer.h"
#include "parser.h"
#include "opcode.h"

/* Maximum number of constant in a function. */
#define MAX_CONSTS 256
/* Maximum number of locals in a function. */
#define MAX_LOCALS 256

/* Represents an emitter to emit virtual machine bytecode/code_t. */
struct emitter {
	/* Number of instr in `code` block. */
	size_t count;
	/* Capacity of `code` block. */
	size_t capacity;
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

	/* Maximum size the function could occupy on the virtual machine stack. */
	size_t stack_size;

	/* Number of constants in the function. */
	uint8_t nconsts;
	/* Array of constants in the function. */
	struct value consts[MAX_CONSTS];

	/* Number of locals in the function. */
	uint8_t nlocals;
	/* Array of locals in the function.*/
	struct variable locals[MAX_LOCALS];

	/* Function we're compiling. */
	struct func *fn;
	/* Prototype of the function we're compiling. */
	struct proto *proto;
};

/**
 * Initializes the specified compiler with the specified state which owns the
 * compiler instance.
 *
 * @param[in] compiler Compiler instance.
 * @param[in] state State instance.
 */
void compiler_init(struct compiler *compiler, struct state *state);

/**
 * Compiles the specified pseu source code to a function using the specified
 * initialized compiler.
 *
 * @param[in] compiler Compiler instance.
 * @param[in] src Pseu source code to initialize.
 * @return Function compiled if success; otherwise returns NULL.
 */
struct func *compiler_compile(struct compiler *compiler, const char *src);

#endif /* COMPILER_H */
