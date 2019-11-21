#include "vm.h"

void dump_value(pseu_state_t *s, FILE *f, struct value *v)
{
	struct type *t = v_type(s, v);

	if (!t) {
		fprintf(f, "<unkn>(%p)\n", (void *)v->as.object);
	} else {
		fprintf(f, "%s(", t->ident);

		switch (v->type) {
		case VAL_BOOL:
			fprintf(f, v->as.boolean ? "true)\n" : "false)\n");
			break;
		case VAL_INT:
			fprintf(f, "%d)\n", v->as.integer);
			break;
		case VAL_FLOAT:
			fprintf(f, "%f)\n", v->as.real);
			break;
		case VAL_OBJ:
			fprintf(f, "%p)\n", (void *)v->as.object);
			break;
		}
	}
}

void dump_fn_sig(pseu_state_t *s, FILE *f, struct function *fn)
{
	if (fn->return_type)
		fprintf(f, "func ");
	else
		fprintf(f, "proc ");

	if (fn->ident)
		fprintf(f, "%s", fn->ident);
	else
		fprintf(f, "<#>");

	fprintf(f, "(");
	for (uint8_t i = 0; i < fn->params_count; i++) {
		fprintf(f, "%s", fn->param_types[i]->ident);
		if (i != fn->params_count - 1)
			fprintf(f, ", ");
	}
	fprintf(f, ")");

	if (fn->return_type)
		fprintf(f, ": %s", fn->return_type->ident);

	if (fn->type == FN_PSEU)
		fprintf(f, " max_stack %d\n", fn->as.pseu.max_stack);
	else
		fprintf(f, " c\n");
}

void dump_fn_consts(pseu_state_t *s, FILE *f, struct function *fn)
{
	pseu_assert(fn->type == FN_PSEU);
	fprintf(f, "consts %d\n", fn->as.pseu.const_count);
	for (uint8_t i = 0; i < fn->as.pseu.const_count; i++) {
		fprintf(f, " %03d ", i);
		dump_value(s, f, &fn->as.pseu.consts[i]);
	}
}

void dump_fn_locals(FILE *f, struct function *fn)
{
	pseu_assert(fn->type == FN_PSEU);
	fprintf(f, "locals %d\n", fn->as.pseu.local_count);
	for (uint8_t i = 0; i < fn->as.pseu.local_count; i++)
		fprintf(f, " %03d %s\n", i, fn->as.pseu.locals[i]->ident);
}

void dump_fn_code(pseu_state_t *s, FILE *f, struct function *fn)
{
	fprintf(f, "code %d\n", fn->as.pseu.code_count);
	
	#define READ_UINT8()  	(*ip++)
	#define READ_UINT16() 	(*ip++) /* FIXME: Temp solution for now. */

	#define INTERPRET 				\
		code_t op; 					\
		decode: 					\
		switch ((op = READ_UINT8()))
	#define DISPATCH_EXIT() return;
	#define DISPATCH() 		goto decode
	#define OP(x) 			case OP_##x
	#define OP_UNDEF() 		default

	#define IP				((int)(ip - ip_begin))
	#define OP_DUMP0(n)		fprintf(f, " %05d %s\n", IP, n)
	#define OP_DUMP1(n, a)	fprintf(f, " %05d %s %d\n", IP, n, a)
	
	code_t *ip_begin = fn->as.pseu.code;
	code_t *ip = ip_begin;
	
	INTERPRET {
		OP(LD_CONST): {
			uint16_t index = READ_UINT16(); 
			fprintf(f, " %05d %s ", IP, "ld.const");
			dump_value(s, f, &fn->as.pseu.consts[index]);
			DISPATCH();
		}
		OP(LD_LOCAL): {
			uint16_t index = READ_UINT8();
			OP_DUMP1("ld.local", index);
			DISPATCH();
		}
		OP(LD_GLOBAL): {
			uint16_t index = READ_UINT16(); 
			OP_DUMP1("ld.global", index);
			DISPATCH();
		}
		OP(CALL): {
			uint16_t index = READ_UINT16(); 
			struct function *nfn = &VM(s)->fns[index]; 
			fprintf(f, " %05d %s ", IP, "call");
			dump_fn_sig(s, f, nfn);
			DISPATCH();
		}
		OP(RET): {
			OP_DUMP0("ret");
			DISPATCH_EXIT();
		}
		OP_UNDEF(): {
			OP_DUMP0("undef");
			DISPATCH_EXIT();
		}
	}
}

void pseu_dump_function(pseu_state_t *s, FILE* f, struct function *fn)
{
	dump_fn_sig(s, f, fn);

	if (fn->type == FN_PSEU) {
		dump_fn_consts(s, f, fn);
		dump_fn_locals(f, fn);
		dump_fn_code(s, f, fn);
	}
}
