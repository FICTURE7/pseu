#include <pseu.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "vm.h"
#include "core.h"

/* Default print function of the pseu virtual machine. */
static void default_print(VM *vm, const char *text) 
{
  pseu_unused(vm);
  printf("%s", text);
}

/* Default alloc function of the pseu virtual machine. */
static void *default_alloc(VM *vm, size_t size) 
{
  pseu_unused(vm);
  return malloc(size);
}

/* Default realloc function of the pseu virtual machine. */
static void *default_realloc(VM *vm, void *ptr, size_t size) 
{
  pseu_unused(vm);
  return realloc(ptr, size);
}

/* Default free function of the pseu virtual machine. */
static void default_free(VM *vm, void *ptr) 
{
  pseu_unused(vm);
  free(ptr);
}

/* Default panic handler. */
static void default_panic(VM *vm, const char *message)
{
  pseu_unused(vm);
  fprintf(stderr, "error: %s.", message);
  abort();
}

/* Initializes the default configuration, used when pseu_vm_new(NULL) is 
 * called.
 */
static void config_init_default(PseuConfig *config) 
{
  config->panic = default_panic;
  config->print = default_print;
  config->alloc = default_alloc;
  config->realloc = default_realloc;
  config->free = default_free;
}

PseuVM *pseu_vm_new(PseuConfig *config)
{
  PseuVM *vm;

  if (config)
    vm = config->alloc(NULL, sizeof(PseuVM));
  else
    vm = malloc(sizeof(PseuVM));

  if (!vm)
    goto exit;

  if (config)
    memcpy(&vm->config, config, sizeof(PseuConfig));
  else
    config_init_default(&vm->config);

  // XXX
  vm->types_count = 0;
  vm->fns_count = 0;
  vm->vars_count = 0;
  // XXX
  vm->data  = NULL;
  vm->error = NULL;
  vm->state = pseu_state_new(vm);

  if (!vm->state)
    goto exit_vm;

  pseu_core_init(vm);
  return vm;

exit_vm:
  pseu_vm_free(vm);
exit:
  return NULL;
}

int pseu_vm_eval(PseuVM *vm, const char *src) 
{
  assert(vm && src);

  Function fn;
  if (pseu_parse(vm->state, &fn, src))
    return PSEU_RESULT_ERROR;
  if (pseu_call(vm->state, &fn))
    return PSEU_RESULT_ERROR;
  return PSEU_RESULT_SUCCESS;
}

void pseu_vm_free(PseuVM *vm)
{
  /* TODO: Free the other stuff as well. */
  if (vm)
    pseu_state_free(vm->state);
}

void pseu_vm_set_data(PseuVM *vm, void *data)
{
  assert(vm);
  vm->data = data;
}

void *pseu_vm_get_data(PseuVM *vm) 
{
  assert(vm);
  return vm->data;
}
