#include <pseu.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "vm.h"
#include "core.h"

/* Default print function of the pseu virtual machine. */
static void default_print(pseu_vm_t *vm, const char *text) 
{
	pseu_unused(vm);
	printf("%s", text);
}

/* Default alloc function of the pseu virtual machine. */
static void *default_alloc(pseu_vm_t *vm, size_t size) 
{
	pseu_unused(vm);
	return malloc(size);
}

/* Default realloc function of the pseu virtual machine. */
static void *default_realloc(pseu_vm_t *vm, void *ptr, size_t size) 
{
	pseu_unused(vm);
	return realloc(ptr, size);
}

/* Default free function of the pseu virtual machine. */
static void default_free(pseu_vm_t *vm, void *ptr) 
{
	pseu_unused(vm);
	free(ptr);
}

/* Default onerror handler. */
static void default_panic(pseu_vm_t *vm, const char *message)
{
	pseu_unused(vm);
	fprintf(stderr, "error: %s.", message);
}

/* Initializes the default configuration, used when pseu_vm_new(NULL) is 
 * called.
 */
static void config_init_default(pseu_config_t *config) 
{
	config->panic = default_panic;
	config->print = default_print;
	config->alloc = default_alloc;
	config->realloc = default_realloc;
	config->free = default_free;
}

pseu_vm_t *pseu_vm_new(pseu_config_t *config)
{
	pseu_vm_t *vm;

	if (config)
		vm = config->alloc(NULL, sizeof(pseu_vm_t));
	else
		vm = malloc(sizeof(pseu_vm_t));

	if (!vm) goto exit;

	if (config)
		memcpy(&vm->config, config, sizeof(pseu_config_t));
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
	if (!vm->state) goto exit_vm;
	pseu_core_init(vm);
	return vm;

exit_vm:
	pseu_vm_free(vm);
exit:
	return NULL;
}

int pseu_vm_eval(pseu_vm_t *vm, const char *src) 
{
	assert(vm && src);
	struct function fn;
	if (pseu_parse(vm->state, &fn, src))
		return PSEU_RESULT_ERROR;
	if (pseu_call(vm->state, &fn))
		return PSEU_RESULT_ERROR;
	// XXX: Consider memory leak?
	return PSEU_RESULT_SUCCESS;
}

void pseu_vm_free(pseu_vm_t *vm)
{
	if (vm) pseu_state_free(vm->state);
}

void pseu_vm_set_data(pseu_vm_t *vm, void *data)
{
	assert(vm);
	vm->data = data;
}

void *pseu_vm_get_data(pseu_vm_t *vm) 
{
	assert(vm);
	return vm->data;
}
