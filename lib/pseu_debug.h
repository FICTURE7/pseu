#ifndef PSEU_DEBUG_H
#define PSEU_DEBUG_H

#include <stdio.h>
#include <assert.h>

#include "pseu_vm.h"
#include "pseu_node.h"
#include "pseu_token.h"
#include "pseu_value.h"

#ifdef PSEU_USE_ASSERT
	#define pseu_assert(cond) do { assert(cond); } while(0)
#else
	#define pseu_assert(cond)
#endif

void pseu_dump_token(FILE *stream, struct token *token);
void pseu_dump_node(FILE *stream, struct node *node);
void pseu_dump_value(FILE *stream, struct value *value);
void pseu_dump_function_info(FILE *stream, struct function *fn);
void pseu_dump_function(FILE *stream, pseu_vm_t *vm, struct function *fn);

#endif /* PSEU_DEBUG_H */
