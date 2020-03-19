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

static u8 declare_const(Parser *p, Value *v)
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
  u16 index = pseu_get_variable(V(p->lex.state), lcl->ident.pos, lcl->ident.len);
  if (index != PSEU_INVALID_GLOBAL) {
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

static void emit_u8(Parser *p, u8 code)
{
  if (p->failed)
    return;
  if (p->code_count >= p->code_size)
    pseu_vec_grow(p->lex.state, &p->code, &p->code_size, BCode);
  p->code[p->code_count++] = code;
}

static void emit_u16(Parser *p, u16 value)
{
  emit_u8(p, (value >> 8) & 0xFF);
  emit_u8(p, (value) & 0xFF);
}

static void emit_ld_const(Parser *p, Value *v)
{
  int index = declare_const(p, v);
  if (index == -1) {
    parse_err(p, "Exceeded maximum number of constant in a function/procedure");
  } else {
    p->max_stack++;

    emit_u8(p, OP_LD_CONST);
    emit_u8(p, index);
  }
}

static void emit_ld_global(Parser *p, int index)
{
  p->max_stack++;

  emit_u8(p, OP_LD_GLOBAL);
  emit_u16(p, index);
}

static void emit_ld_local(Parser *p, const char *ident, size len)
{
  int index = resolve_local(p, ident, len);
  if (index == -1) {
    parse_err(p, "Local \"%s\" not defined", ident);
  } else {
    p->max_stack++;

    emit_u8(p, OP_LD_LOCAL);
    emit_u8(p, index);
  }
}

static void emit_st_local(Parser *p, const char *ident, size len)
{
  int index = resolve_local(p, ident, len);
  if (index == -1) {
    parse_err(p, "Local \"%s\" not defined", ident);
  } else {
    emit_u8(p, OP_ST_LOCAL);
    emit_u8(p, index);
  }
}

static void emit_ld_variable(Parser *p, const char *ident, size len)
{
  u16 index = pseu_get_variable(V(p->lex.state), ident, len);
  if (index != PSEU_INVALID_GLOBAL)
    emit_ld_global(p, index);
  else
    emit_ld_local(p, ident, len);
}

static void emit_calln(Parser *p, const char *ident, size len)
{
  u16 index = pseu_get_function(VM(p->lex.state), ident, len);
  if (index == PSEU_INVALID_FUNC) {
    parse_err(p, "Function or procedure \"%s\" is not defined.", ident);
  } else {
    emit_u8(p, OP_CALL);
    emit_u16(p, index);
  }
}

static void emit_call(Parser *p, const char *ident)
{
  emit_calln(p, ident, strlen(ident));
}

static int emit_br(Parser *p)
{
  emit_u8(p, OP_BR);

  int result = p->code_count;
  p->code_count += sizeof(u16);
  return result;
}

static int emit_br_false(Parser *p)
{
  emit_u8(p, OP_BR_FALSE);

  int result = p->code_count;
  p->code_count += sizeof(u16);
  return result;
}

static void patch_br(Parser *p, int offset)
{
  u16 address = p->code_count;
  p->code[offset] = (address >> 8) & 0xFF;
  p->code[offset + 1] = (address) & 0xFF;
}

/* Returns the precedence of the specifed token. */
static int op_precedence(Token tok)
{
  switch (tok) {
    case TK_kw_or:
      return 1;
    case TK_kw_and:
      return 2;

    case '=':
    case '>':
    case '<':
    case TK_op_ge:
    case TK_op_le:
      return 3;

    case '+':
    case '-':
      return 4;

    case '*':
    case '/':
      return 5;

    default:
      return -1;
  }
}

#define next(P)           ((P)->tok = pseu_lex_scan(&(P)->lex))
#define peek(P)           ((P)->tok)
#define expect_next(P, t) (next(P) == t)
#define expect_peek(P, t) (peek(P) == t)

/* == Forward declaration. */
static void parse_expr(Parser *p);
static void parse_statement(Parser *p);

/* Parse an expression term. */
static int parse_expr_primary(Parser *p)
{
  switch (p->tok) {
  case '+':
  case '-':
  case TK_kw_not: {
    Token op = peek(p);
    next(p);
    int result = parse_expr_primary(p);

    if (op == '-')
      emit_call(p, "@neg");
    else if (op == TK_kw_not)
      emit_call(p, "@not");
    return result;
  }

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
    next(p);
    emit_ld_const(p, &p->lex.value);
    return 0;

  case TK_identifier:
    next(p);
    emit_ld_variable(p, p->lex.span.pos, p->lex.span.len);
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

    case '=': emit_call(p, "@eq"); break;
    case '>': emit_call(p, "@gt"); break;
    case '<': emit_call(p, "@lt"); break;
    case TK_op_ge: emit_call(p, "@ge"); break;
    case TK_op_le: emit_call(p, "@le"); break;

    case TK_kw_and: emit_call(p, "@and"); break;
    case TK_kw_or:  emit_call(p, "@or");  break;

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

/* Parse a declare statement. */
static void parse_declare(Parser *p)
{
  if (!expect_next(p, TK_identifier)) {
    parse_err(p, "Expected variable identifier");
    return;
  }

  Span ident = p->lex.span;

  if (next(p) == ':') {
    if (!expect_next(p, TK_identifier)) {
      parse_err(p, "Expected variable type");
    } else {
      Span type_ident = p->lex.span;
      u16 type_index = pseu_get_type(V(p->lex.state), type_ident.pos, type_ident.len);
      if (type_index == PSEU_INVALID_TYPE) {
        parse_err(p, "Unknown type specified.");
      } else {
        Type *type = &V(p->lex.state)->types[type_index];
        Local lcl = {
          .scope = p->scope,
          .ident = ident,
          .type_ident = type_ident,
          .type = type
        };

        declare_local(p, &lcl);
      }

      next(p);

      /* Parse assignment after declaration if its here. */
      if (peek(p) == TK_op_assign) {
        next(p);
        parse_expr(p);
        emit_st_local(p, ident.pos, ident.len);
      }
    }
  } else {
    /* TODO: Non typed variable aka ANY. */
    parse_err(p, "Expected type.");
  }
}

/* Parse an assign statement. */
static void parse_assignment(Parser *p)
{
  Span ident = p->lex.span;

  next(p);

  if (!expect_peek(p, TK_op_assign)) {
    parse_err(p, "Expected assign operator '<-'.");
    return;
  }

  next(p);
  parse_expr(p);
  emit_st_local(p, ident.pos, ident.len);
}

static void parse_if_block(Parser *p)
{
  next(p);
  parse_expr(p);
  if (!expect_peek(p, TK_kw_then)) {
    parse_err(p, "Expected THEN keyword.");
    return;
  }

  if (!expect_next(p, TK_newline)) {
    parse_err(p, "Expected new line after THEN keyword.");
    return;
  }

  next(p);

  int if_jmp = emit_br_false(p);
  while (peek(p) != TK_kw_else && peek(p) != TK_kw_endif && peek(p) != TK_eof)
    parse_statement(p);

  /* If ELSE, parse else block. */
  if (peek(p) == TK_kw_else) {
    if (!expect_next(p, TK_newline)) {
      parse_err(p, "Expected new line after ELSE keyword.");
      return;
    }
    
    int else_jmp = emit_br(p);
    patch_br(p, if_jmp);
    while (peek(p) != TK_kw_endif && peek(p) != TK_eof)
      parse_statement(p);
    patch_br(p, else_jmp);
  } else {
    patch_br(p, if_jmp);
  }

  next(p);

  if (peek(p) != TK_newline && peek(p) != TK_eof)
    parse_err(p, "Expected new line or end of file after ENDIF.");
}

static void parse_statement(Parser *p)
{
  switch (peek(p)) {
  case TK_newline:
  case TK_eof:
    break;

  case TK_kw_output:
    parse_output(p);
    break;
  case TK_kw_declare:
    parse_declare(p);
    break;
  case TK_kw_if:
    parse_if_block(p);
    break;
  case TK_identifier:
    parse_assignment(p);
    break;

  default:
    parse_err(p, "Expected begining of statement.");
    break;
  }

  if (peek(p) != TK_newline && peek(p) != TK_eof)
    parse_err(p, "Expected new line or end of file.");
  next(p);
}

static int parse_root(Parser *p)
{
  while (peek(p) != TK_eof)
    parse_statement(p);
  emit_u8(p, OP_RET);
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
