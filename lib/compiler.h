#ifndef COMPILER_H
#define COMPILER_H

#include <stdbool.h>
#include "func.h"
#include "value.h"
#include "lexer.h"
#include "parser.h"
#include "opcode.h"

/* maximum number of constant in a function */
#define MAX_CONSTS 256
/* maximum number of locals in a function */
#define MAX_LOCALS 256

/* represents an emitter to emit bytecode */
struct emitter {
	size_t count; /* number of instr in `code` */
	size_t capacity; /* size of `code` */
	instr_t *code; /* buffer containing the instr */
};

/* represents a compiler */
struct compiler {
	struct state *state; /* state which owns this compiler instance */

	struct lexer lexer; /* lexer of the compiler, used by the parser */
	struct parser parser; /* parser of the compiler */
	struct emitter emitter; /* emitter to emit byte code */
	struct compiler *top; /* top level compiler, NULL if already top */

	uint8_t nconsts; /* number of constants in the fn */
	struct value consts[MAX_CONSTS]; /* array of constants in the fn */

	uint8_t nlocals; /* number of locals in the fn */
	struct variable locals[MAX_LOCALS]; /* array of locals in the fn */

	struct func *fn; /* function we're compiling */
	struct proto *proto; /* proto of the function we're compiling */
};

void compiler_init(struct compiler *compiler, struct state *state);
struct func *compiler_compile(struct compiler *compiler, const char *src);

#endif /* COMPILER_H */
