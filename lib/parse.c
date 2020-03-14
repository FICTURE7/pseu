#include "vm.h"
#include "lex.h"

#include <stdarg.h>

/* A local variable. */
struct local {
  size_t scope;           /* Scope of local. */
  struct span ident;      /* Identifier of local. */
  struct span type_ident;	/* Type identifier of local. */
  struct type *type;      /* Type of local. */
};

/* A parser state. */
struct parser {
  token_t tok;
  struct lexer lex;
  struct function *fn;

  uint16_t scope;
  uint32_t max_stack;

  size_t code_size;
  size_t code_count;
  code_t *code;	

  size_t consts_size;
  size_t consts_count;
  struct value *consts;

  size_t vars_size;
  size_t vars_count;
  struct local *vars;

  int failed;
};

static int spaneq(struct span *a, struct span *b)
{
  if (a->len != b->len)
    return 0;
  return strncmp(a->pos, b->pos, a->len) == 0;
}

static void parse_err(struct parser *p, const char *message, ...)
{
  va_list args;
  va_start(args, message);

  size_t needed = vsnprintf(NULL, 0, message, args);
  char *buffer = pseu_alloc(p->lex.state, needed);

  vsprintf(buffer, message, args);
  va_end(args);

  pseu_print(p->lex.state, buffer);
  pseu_print(p->lex.state, "\n");
  pseu_free(p->lex.state, buffer);

  p->failed = 1;
}

static int resolve_local(struct parser *p, const char *ident, size_t len)
  size_t i;
  struct span lcl_ident = {
    .pos = (char *)ident,
    .len = len
  };

  for (size_t i = 0; i < p->vars_count; i++) {
    struct local *olcl = &p->vars[i];

    if (olcl->scope <= p->scope && spaneq(&lcl_ident, &olcl->ident))
      return i;
  }

  return -1;
}

static uint16_t resolve_global(struct parser *p, const char *ident, size_t len)
{
  return pseu_get_variable(VM(p->lex.state), ident, len);
}

static int declare_const(struct parser *p, struct value *v)
{
  if (p->consts_count >= PSEU_MAX_CONST)
    return -1;

  if (p->consts_count >= p->consts_size)
    pseu_vec_grow(p->lex.state, &p->consts, &p->consts_size, struct value);

  p->consts[p->consts_count] = *v;
  return p->consts_count++;
}

static int declare_local(struct parser *p, struct local *lcl)
{
  /* Look for duplicate ident in globals. */
  uint16_t index = resolve_global(p, lcl->ident.pos, lcl->ident.len);

  if (index != 0xFFFF) {
    parse_err(p, "Global already defined with same identifier");
    return 1;
  }

  /* Look for duplicate ident in locals. */
  for (size_t i = 0; i < p->vars_count; i++) {
    struct local *olcl = &p->vars[i];

    if (olcl->scope <= lcl->scope && spaneq(&lcl->ident, &olcl->ident)) {
      parse_err(p, "Local already defined with same identifier");
      return 1;
    }
  }

  if (p->vars_count >= p->vars_size)
    pseu_vec_grow(p->lex.state, &p->vars, &p->vars_size, struct local);

  p->vars[p->vars_count++] = *lcl;
  return 0;
}

static void emit(struct parser *p, code_t code)
{
  if (p->failed)
    return;

  if (p->code_count >= p->code_size)
    pseu_vec_grow(p->lex.state, &p->code, &p->code_size, code_t);

  p->code[p->code_count++] = code;
}

static void emit_ld_const(struct parser *p, struct value *v)
{
  int index = declare_const(p, v);

  if (index == -1) {
    parse_err(p, "Exceeded maximum number of constant in a function/procedure");
  } else {
    p->max_stack++;

    emit(p, OP_LD_CONST);
    emit(p, index);
  }
}

static void emit_ld_global(struct parser *p, const char *ident, size_t len)
{
  uint16_t index = pseu_get_variable(VM(p->lex.state), ident, len);

  if (index == 0xFFFF) {
    parse_err(p, "Global not defined");
  } else {
    p->max_stack++;

    emit(p, OP_LD_GLOBAL);
    emit(p, index);
  }
}

static void emit_ld_local(struct parser *p, const char *ident, size_t len)
{
  int index = resolve_local(p, ident, len);

  if (index == -1) {
    parse_err(p, "Local \"%s\" not defined", ident);
  } else {
    p->max_stack++;

    emit(p, OP_LD_LOCAL);
    emit(p, index);
  }
}

static void emit_ld_variable(struct parser *p, const char *ident, size_t len)
{
  int index = resolve_global(p, ident, len);

  if (index != 0xFFFF)
    emit_ld_global(p, ident, len);
  else
    emit_ld_local(p, ident, len);
}

static void emit_call(struct parser *p, const char *ident)
{
  int index = pseu_get_function(VM(p->lex.state), ident);

  if (index == PSEU_INVALID_FUNC) {
    parse_err(p, "Function or procedure \"%s\" is not defined.", ident);
  } else {
    emit(p, OP_CALL);
    emit(p, index);
  }
}

/* Returns the precedence of the specifed token. */
static int op_precedence(token_t tok)
{
  switch (tok) {
    case '+':
    case '-':
      return 1;
    case '*':
    case '/':
      return 2;

    default:
      return -1;
  }
}

#define eat(P)        ((p)->tok = pseu_lex_scan(&(p)->lex))
#define expect(P, t)	(eat(p) == t)

/* == Forward declaration. */
static void parse_expr(struct parser *p);

/* Parse an expression term. */
static int parse_expr_primary(struct parser *p)
{
  switch (p->tok) {
  case '+':
  case '-':
    eat(p);
    /* TODO: Emit call to add or sub, or something. */
    return parse_expr_primary(p);

  case '(':
    eat(p);
    parse_expr(p);
    if (p->tok == ')') {
      eat(p);
      return 0;
    }
    parse_err(p, "Expected ')'");
    return 1;

  case TK_lit_real:
  case TK_lit_integer:
  case TK_lit_string:
    emit_ld_const(p, &p->lex.value);
    eat(p);
    return 0;

    /*
       case TK_lit_true:
       emit(p, OP_LD_KTRUE);
       eat(p);
       return 0;
       case TK_lit_false:
       emit(p, OP_LD_KFALSE);
       eat(p);
       return 0;
       */

  case TK_identifier:
    emit_ld_variable(p, p->lex.span.pos, p->lex.span.len);
    eat(p);
    return 0;

  default:
    parse_err(p, "Expected expression term");
    return 1;
  }
}

/* Parse a binary expression. */
static void parse_expr_binop(struct parser *p, int prece)
{
  /* Parse lhs of expression; bail if failed. */
  if (parse_expr_primary(p))
    return;

  for (;;) {
    token_t op_tok = p->tok;
    int cur_prece = op_precedence(op_tok);
    if (cur_prece <= prece)
      return;

    eat(p);
    /* Parse rhs of expression. */
    parse_expr_binop(p, cur_prece);

    switch (op_tok) {
    case '+': emit_call(p, "@add"); break;
    case '-': emit_call(p, "@sub"); break;
    case '*': emit_call(p, "@mul"); break;
    case '/': emit_call(p, "@div"); break;

    default:
      pseu_unreachable();
      break;
    }
  }
}

/* Parse an expression. */
static void parse_expr(struct parser *p)
{
  parse_expr_binop(p, 0);
}

/* Parse an output statement. */
static void parse_output(struct parser *p)
{
  eat(p);

  parse_expr(p);
  emit_call(p, "@output");
}

static void parse_declare(struct parser *p)
{
  struct span type_ident;
  struct span ident;

  if (!expect(p, TK_identifier)) {
    parse_err(p, "Expected variable identifier");
    return;
  }

  ident = p->lex.span;

  /* TODO: Non typed variable. */
  if (eat(p) == ':') {
    if (!expect(p, TK_identifier)) {
      parse_err(p, "Expected variable type");
    } else {
      type_ident = p->lex.span;

      struct local lcl;
      lcl.scope = p->scope;
      lcl.ident = ident;
      lcl.type_ident = type_ident;
      lcl.type = pseu_get_type(
          VM(p->lex.state),
          type_ident.pos,
          type_ident.len);
      declare_local(p, &lcl);
    }
  } 
}

static int parse_root(struct parser *p)
{
  while (eat(p) != TK_eof) {
    switch (p->tok) {
    case TK_newline:
      continue;
    case TK_kw_output:
      parse_output(p);
      break;
    case TK_kw_declare:
      parse_declare(p);
      break;
    default:
      parse_err(p, "Expected statement");
      break;
    }
  }

  emit(p, OP_RET);
  return 0;
}

int pseu_parse(pseu_state_t *s, struct function *fn, const char *src)
{
  struct parser p;	
  if (pseu_lex_init(s, &p.lex, src))
    goto fail;

  p.fn = fn;
  p.failed = 0;
  p.scope = 0;
  p.max_stack  = 0;
  p.code_count = 0;
  p.code_size  = 16;

  if (pseu_vec_init(s, &p.code, p.code_size, code_t))
    goto fail;

  p.consts_count = 0;
  p.consts_size  = 8;

  if (pseu_vec_init(s, &p.consts, p.consts_size, struct value))
    goto fail_code;

  p.vars_count = 0;
  p.vars_size  = 8;

  if (pseu_vec_init(s, &p.vars, p.vars_size, struct local))
    goto fail_consts;
  if (parse_root(&p))
    goto fail_vars;

  fn->type  = FN_PSEU;
  fn->ident = NULL;
  fn->params_count = 0;
  fn->param_types  = NULL;
  fn->return_type  = NULL;
  fn->as.pseu.code = p.code;
  fn->as.pseu.code_count = p.code_count;
  fn->as.pseu.consts = p.consts;
  fn->as.pseu.const_count = p.consts_count;
  fn->as.pseu.locals = p.vars_count > 0 ? pseu_alloc(s, p.vars_count * sizeof(struct type *)) : NULL;
  fn->as.pseu.local_count = p.vars_count;
  fn->as.pseu.max_stack = p.max_stack;

  for (size_t i = 0; i < fn->as.pseu.local_count; i++)
    fn->as.pseu.locals[i] = p.vars[i].type;

  if (pseu_config_flag(s, PSEU_CONFIG_DUMP_FUNCTION))
    pseu_dump_function(s, stdout, fn);

  return p.failed;

fail_vars:
  pseu_free(s, p.vars);
fail_consts:
  pseu_free(s, p.consts);
fail_code:
  pseu_free(s, p.code);
fail:
  return 1;
}
