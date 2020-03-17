#include "vm.h"
#include "obj.h"

void dump_value(State *s, FILE *f, Value *v)
{
  Type *t = v_type(s, v);

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

void dump_variable(State *s, FILE *f, Variable *v)
{
  printf("\'%s\':", v->ident);
  dump_value(s, f, &v->value);
}

void dump_fn_sig(FILE *f, Function *fn)
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
  for (u8 i = 0; i < fn->params_count; i++) {
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

void dump_fn_consts(State *s, FILE *f, Function *fn)
{
  pseu_assert(fn->type == FN_PSEU);

  fprintf(f, "consts %d\n", fn->as.pseu.const_count);
  for (u8 i = 0; i < fn->as.pseu.const_count; i++) {
    fprintf(f, " %03d ", i);
    dump_value(s, f, &fn->as.pseu.consts[i]);
  }
}

void dump_fn_locals(FILE *f, Function *fn)
{
  pseu_assert(fn->type == FN_PSEU);

  fprintf(f, "locals %d\n", fn->as.pseu.local_count);
  for (u8 i = 0; i < fn->as.pseu.local_count; i++)
    fprintf(f, " %03d %s\n", i, fn->as.pseu.locals[i]->ident);
}

void dump_fn_code(State *s, FILE *f, Function *fn)
{
  fprintf(f, "code %d\n", fn->as.pseu.code_count);

  #define READ_UINT8()  	(*ip++)
  #define READ_UINT16() 	(*ip++) /* FIXME: Temp solution for now. */

  #define INTERPRET               \
    BCode op;                     \
    decode:                       \
    switch ((op = READ_UINT8()))
  #define DISPATCH_EXIT() return;
  #define DISPATCH()      goto decode
  #define OP(x)           case OP_##x
  #define OP_UNDEF()      default

  #define IP              ((int)(ip - ip_begin))
  #define OP_DUMP0(n)     fprintf(f, " %05d %s\n", IP, n)
  #define OP_DUMP1(n, a)  fprintf(f, " %05d %s %d\n", IP, n, a)

  BCode *ip_begin = fn->as.pseu.code;
  BCode *ip = ip_begin;

  INTERPRET {
    OP(LD_CONST): {
      u16 index = READ_UINT16(); 

      fprintf(f, " %05d %s ", IP, "ld.const");
      dump_value(s, f, &fn->as.pseu.consts[index]);
      DISPATCH();
    }
    OP(LD_LOCAL): {
      u8 index = READ_UINT8();

      OP_DUMP1("ld.local", index);
      DISPATCH();
    }
    OP(ST_LOCAL): {
      u8 index = READ_UINT8();

      OP_DUMP1("st.local", index);
      DISPATCH();
    }
    OP(LD_GLOBAL): {
      u16 index = READ_UINT16(); 

      fprintf(f, " %05d %s ", IP, "ld.global");
      dump_variable(s, f, &VM(s)->vars[index]);
      DISPATCH();
    }
    OP(CALL): {
      u16 index = READ_UINT16(); 
      Function *nfn = &VM(s)->fns[index]; 

      fprintf(f, " %05d %s ", IP, "call");
      dump_fn_sig(f, nfn);
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

void pseu_dump_function(State *s, FILE* f, Function *fn)
{
  dump_fn_sig(f, fn);

  if (fn->type == FN_PSEU) {
    dump_fn_consts(s, f, fn);
    dump_fn_locals(f, fn);
    dump_fn_code(s, f, fn);
  }
}
