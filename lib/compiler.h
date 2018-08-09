#ifndef COMPILER_H
#define COMPILER_H

#include "func.h"
#include "lexer.h"
#include "parser.h"
#include "opcode.h"

/* represents the type of symbol */
enum symbol_type {
	SYMBOL_VAR,
	SYMBOL_FUNC
};

/* represents a symbol */
struct symbol {
	enum symbol_type type;
};

struct var_symbol {
	struct symbol base;

	char *name;
	struct type *type;
};

struct func_symbol {
	struct symbol base;

	struct proto *proto;
};

/* represents a symbol table */
struct symbol_table {
	size_t count;
	size_t capacity;
	struct symbol *symbols;
};

/* represents a local in a function */
struct local {
	char *name; /* name of the local */
	struct type *type; /* type of the local */
};

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

	int8_t nconsts; /* number of constants in the fn */
	struct value consts[256]; /* array of constants in the fn */

	int8_t nlocals; /* number of locals in the fn */
	struct local locals[256]; /* array of locals in the fn */
};

void compiler_init(struct compiler *compiler, struct state *state);
struct func *compiler_compile(struct compiler *compiler, char *src);

#endif /* COMPILER_H */
