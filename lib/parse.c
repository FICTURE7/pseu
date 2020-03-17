#include "vm.h"
#include "obj.h"
#include "lex.h"

#include <stdarg.h>

/* A local variable. */
typedef struct Local {
  size scope;             /* Scope of local. */
  Span ident;             /* Identifier of local. */
  Span type_ident;	      /* Type identifier of local. */
  Type *type;             /* Type of local. */
} Local;

/* A parser state. */
typedef struct Parser {
  Token tok;
  Lexer lex;
  Function *fn;

  u16 scope;
  u32 max_stack;

  size code_size;
  size code_count;
  BCode *code;	

  size consts_size;
  size consts_count;
  Value *consts;

  size vars_size;
  size vars_count;
  Local *vars;

  int failed;
} Parser;

static int spaneq(Span *a, Span *b)
{
  if (a->len != b->len)
    return 0;
  return strncmp(a->pos, b->pos, a->len) == 0;
}

static void spanval(Span *a, char *buf)
{
  memcpy(buf, a->pos, a->len);
}

/* TODO: Move this stuff to err.c */
static void parse_err(Parser *p, const char *message, ...)
{
  va_list args;
  va_start(args, message);

  size needed = vsnprintf(NULL, 0, message, args);
  char *buffer = pseu_alloc(p->lex.state, needed);

  vsprintf(buffer, message, args);
  va_end(args);

  pseu_print(p->lex.state, buffer);
  pseu_print(p->lex.state, "\n");
  pseu_free(p->lex.state, buffer);

  p->failed = 1;
}

static int resolve_local(Parser *p, const char *ident, size len) {
  Span lcl_ident = {
    .pos = (char *)ident,
    .len = len
  };

  for (size i = 0; i < p->vars_count; i++) {
    Local *olcl = &p->vars[i];
    if (olcl->scope <= p->scope && spaneq(&lcl_ident, &olcl->ident))
      return i;
  }

  return -1;
}

static u16 resolve_global(Parser *p, const char *ident, size len)
{
  return pseu_get_variable(VM(p->lex.state), ident, len);
}

static int declare_const(Parser *p, Value *v)
{
  if (p->consts_count >= PSEU_MAX_CONST)
    return -1;
  if (p->consts_count >= p->consts_size)
    pseu_vec_grow(p->lex.state, &p->consts, &p->consts_size, Value);
  p->consts[p->consts_count] = *v;
  return p->consts_count++;
}

static int declare_local(Parser *p, Local *lcl)
{
  /* Look for duplicate ident in globals. */
  u16 index = resolve_global(p, lcl->ident.pos, lcl->ident.len);
  if (index != 0xFFFF) {
    parse_err(p, "Global already defined with same identifier");
    return 1;
  }

  /* Look for duplicate ident in locals. */
  for (size i = 0; i < p->vars_count; i++) {
    Local *olcl = &p->vars[i];
    if (olcl->scope <= lcl->scope && spaneq(&lcl->ident, &olcl->ident)) {
      parse_err(p, "Local already defined with same identifier");
      return 1;
    }
  }

  if (p->vars_count >= p->vars_size)
    pseu_vec_grow(p->lex.state, &p->vars, &p->vars_size, Local);

  p->vars[p->vars_count++] = *lcl;
  return 0;
}

static void emit(Parser *p, BCode code)
{
  if (p->failed)
    return;
  if (p->code_count >= p->code_size)
    pseu_vec_grow(p->lex.state, &p->code, &p->code_size, BCode);
  p->code[p->code_count++] = code;
}

static void emit_ld_const(Parser *p, Value *v)
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

static void emit_ld_global(Parser *p, const char *ident, size len)
{
  u16 index = pseu_get_variable(VM(p->lex.state), ident, len);
  if (index == 0xFFFF) {
    parse_err(p, "Global not defined");
  } else {
    p->max_stack++;

    emit(p, OP_LD_GLOBAL);
    emit(p, index);
  }
}

static void emit_ld_local(Parser *p, const char *ident, size len)
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

static void emit_st_local(Parser *p, const char *ident, size len)
{
  int index = resolve_local(p, ident, len);
  if (index == -1) {
    parse_err(p, "Local \"%s\" not defined", ident);
  } else {
    emit(p, OP_ST_LOCAL);
    emit(p, index);
  }
}

static void emit_ld_variable(Parser *p, const char *ident, size len)
{
  int index = resolve_global(p, ident, len);
  if (index != 0xFFFF)
    emit_ld_global(p, ident, len);
  else
    emit_ld_local(p, ident, len);
}

static void emit_call(Parser *p, const char *ident)
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
static int op_precedence(Token tok)
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

#define eat(P)        ((P)->tok = pseu_lex_scan(&(P)->lex))
#define expect(P, t)	(eat(P) == t)

#define next(P)           ((P)->tok = pseu_lex_scan(&(P)->lex))
#define peek(P)           ((P)->tok)
#define expect_next(P, t) (next(P) == t)
#define expect_peek(P, t) (peek(P) == t)

/* == Forward declaration. */
static void parse_expr(Parser *p);

/* Parse an expression term. */
static int parse_expr_primary(Parser *p)
{
  switch (p->tok) {
  case '+':
    next(p);
    return parse_expr_primary(p);
  case '-':
    next(p);
    int result = parse_expr_primary(p);
    emit_call(p, "@neg");
    return result;

  case '(':
    next(p);
    parse_expr(p);
    if (p->tok == ')') {
      next(p);
      return 0;
    }
    parse_err(p, "Expected ')'");
    return 1;

  case TK_lit_real:
  case TK_lit_integer:
  case TK_lit_string:
    emit_ld_const(p, &p->lex.value);
    next(p);
    return 0;

  case TK_identifier:
    emit_ld_variable(p, p->lex.span.pos, p->lex.span.len);
    next(p);
    return 0;

  default:
    parse_err(p, "Expected expression term");
    return 1;
  }
}

/* Parse a binary expression. */
static void parse_expr_binop(Parser *p, int prece)
{
  /* Parse lhs of expression; bail if failed. */
  if (parse_expr_primary(p))
    return;

  for (;;) {
    Token op_tok = p->tok;
    int cur_prece = op_precedence(op_tok);
    if (cur_prece <= prece)
      return;

    next(p);
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
static void parse_expr(Parser *p)
{
  parse_expr_binop(p, 0);
}

/* Parse an output statement. */
static void parse_output(Parser *p)
{
  next(p);

  parse_expr(p);
  emit_call(p, "@output");
}

static void parse_declare(Parser *p)
{
  if (!expect_next(p, TK_identifier)) {
    parse_err(p, "Expected variable identifier");
    return;
  }

  Span ident = p->lex.span;

  /* TODO: Non typed variable. */
  if (next(p) == ':') {
    if (!expect_next(p, TK_identifier)) {
      parse_err(p, "Expected variable type");
    } else {
      Span type_ident = p->lex.span;

      Local lcl;
      lcl.scope = p->scope;
      lcl.ident = ident;
      lcl.type_ident = type_ident;
      lcl.type = pseu_get_type(
          V(p->lex.state),
          type_ident.pos,
          type_ident.len);
      declare_local(p, &lcl);
    }
  } 
}

static void parse_assignment(Parser *p)
{
  Span ident = p->lex.span;

  next(p);

  if (!expect_peek(p, TK_op_assign)) {
    parse_err(p, "Expected assign operator '->'.");
    return;
  }

  next(p);
  parse_expr(p);
  emit_st_local(p, ident.pos, ident.len);
}

static int parse_root(Parser *p)
{
  while (peek(p) != TK_eof) {
    switch (peek(p)) {
    case TK_newline:
      next(p);
      continue;

    case TK_kw_output:
      parse_output(p);
      break;
    case TK_kw_declare:
      parse_declare(p);
      break;
    case TK_identifier:
      parse_assignment(p);
      break;

    default:
      parse_err(p, "Expected statement");
      break;
    }

    next(p);
  }

  emit(p, OP_RET);
  return 0;
}

int pseu_parse(State *s, Function *fn, const char *src)
{
  Parser p;	
  if (pseu_lex_init(s, &p.lex, src))
    goto fail;

  p.fn = fn;
  p.failed = 0;
  p.scope = 0;
  p.max_stack  = 0;
  p.code_count = 0;
  p.code_size  = 16;

  if (pseu_vec_init(s, &p.code, p.code_size, BCode))
    goto fail;

  p.consts_count = 0;
  p.consts_size  = 8;

  if (pseu_vec_init(s, &p.consts, p.consts_size, Value))
    goto fail_code;

  p.vars_count = 0;
  p.vars_size  = 8;

  if (pseu_vec_init(s, &p.vars, p.vars_size, Local))
    goto fail_consts;

  next(&p);

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
  fn->as.pseu.locals = p.vars_count > 0 ? pseu_alloc(s, p.vars_count * sizeof(Type *)) : NULL;
  fn->as.pseu.local_count = p.vars_count;
  fn->as.pseu.max_stack = p.max_stack;

  for (size i = 0; i < fn->as.pseu.local_count; i++)
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
