#include "vm.h"
#include "obj.h"

bool mark_dfs(GC *gc, Object *o)
{
  pseu_unused(gc);

  if (!o)
    return false;

  return true;
}

bool mark(GC *gc)
{
  pseu_unused(gc);
  return mark_dfs(gc, gc->root);
}

void sweep(GC *gc)
{
  pseu_unused(gc);
}

bool pseu_gc_poll(State *s)
{
  pseu_unused(s);
  return true;
}

void pseu_gc_collect(State *s)
{
  GC *gc = &V(s)->gc;

  /* If we marked anything, we sweep all objects allocated. */
  if (mark(gc))
    sweep(gc);
}

Object *pseu_gc_new(State *s, Type *type, size n)
{
  size sz = n;
  if (t_isarray(s, type)) {
    sz += sizeof(Array);
  } else {
    pseu_unreachable();
  }

  Object *result = (Object *)pseu_alloc(s, sz);
  result->header.marked = false;
  result->header.next = NULL;
  result->header.type = type;

  /* Append object to list of allocated objects. */
  GC *gc = &V(s)->gc;
  Object **walk = &gc->objects;
  while (*walk)
    walk = &(*walk)->header.next;
  *walk = result;
  return result;
}
