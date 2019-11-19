#include "vm.h"

void dump_value(pseu_state_t *s, FILE *f, struct value *v)
{
	pseu_unused(s);

	switch (v->type) {
	case VAL_BOOL:
		fprintf(f, v->as.boolean ? "(boolean true)" : "(boolean false)");
		break;
	case VAL_INT:
		fprintf(f, "(integer %d)\n", v->as.integer);
		break;
	case VAL_FLOAT:
		fprintf(f, "(real %f)\n", v->as.real);
		break;
	case VAL_OBJ:
		fprintf(f, "(object %p)\n", v->as.object);
		break;
	default:
		fprintf(f, "(ukwn %p)\n", v->as.object);
		break;
	}
}

void pseu_dump_function(pseu_state_t *s, FILE* f, struct function *fn)
{
	pseu_unused(s);

	if (fn->return_type)
		fprintf(f, "func ");
	else
		fprintf(f, "proc ");

	if (fn->ident)
		fprintf(f, "%s", fn->ident);
	else
		fprintf(f, "*");

	fprintf(f, "(");
	for (uint8_t i = 0; i < fn->params_count; i++) {
		fprintf(f, "%s", fn->param_types[i]->ident);
		if (i != fn->params_count - 1)
			fprintf(f, ", ");
	}
	fprintf(f, ")");

	if (fn->return_type)
		fprintf(f, " returns %s", fn->return_type->ident);

	if (fn->type == FN_C) {
		fprintf(f, " C");
	} else {
		fprintf(f, " max_stack %d\n", fn->as.pseu.max_stack);
		fprintf(f, "consts %d\n", fn->as.pseu.const_count);
		for (uint8_t i = 0; i < fn->as.pseu.const_count; i++) {
			fprintf(f, " %03d ", i);
			dump_value(s, f, &fn->as.pseu.consts[i]);
		}

		fprintf(f, "locals %d\n", fn->as.pseu.local_count);
		for (uint8_t i = 0; i < fn->as.pseu.local_count; i++)
			fprintf(f, " %03d %s\n", i, fn->as.pseu.locals[i]->ident);

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
			OP(LD_KVAL): {
				uint16_t index = READ_UINT16(); 
				OP_DUMP1("ld.kval", index);
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
				OP_DUMP1("ld.call", index);
				DISPATCH();
			}
			OP(RET): {
				OP_DUMP0("ret");
				DISPATCH_EXIT();
			}
			OP_UNDEF(): {
				fprintf(f, "%p undef\n", ip);
				DISPATCH_EXIT();
			}
		}
	}
	
	fprintf(f, "\n");
}
