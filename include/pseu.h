#ifndef PSEU_H
#define PSEU_H

#include <stdlib.h>
#include <stdint.h>

/**
 * Represents a pseu virtual machine instance.
 */
typedef struct PseuVM PseuVM;

/**
 * Configuration flags of a pseu virtual machine.
 */
typedef enum pseu_config_flags {
	PSEU_CONFIG_DUMP_FUNCTION = 0x01,
} pseu_config_flags_t;

/**
 * Represents the configuration of a pseu virtual machine.
 */
typedef struct PseuConfig {
	/**
	 * Configuration flags.
	 */
	pseu_config_flags_t flags;

	/** 
	 * Callback whenever pseu has encoutered an error.
	 *
	 * @param[in] vm Pseu instance.
	 * @param[in] message Message describing the error.
	 */
	void (*panic)(PseuVM *vm, const char *message); 

	/** 
	 * Callback whenever pseu wants to write to the standard output.
	 *
	 * @param[in] vm Pseu instance.
	 * @param[in] text Text to write to the standard output.
	 */
	void (*print)(PseuVM *vm, const char *text); 	

	/**
	 * Fuction to allocate a block of memory of the specified size.
	 *
	 * @param[in] vm Pseu instance.
	 * @param[in] sz Size of block to allocate.
	 * @returns Pointer to block if success; otherwise NULL if failed.
	 *
	 * @note The returned block may not necessarily return 0'd (zeroed) memory.
	 */
	void *(*alloc)(PseuVM *vm, size_t sz);

	/** 
	 * Function to reallocates a block of memory to a new size.
	 *
	 * @param[in] vm Pseu instance.
	 * @param[in] ptr Pointer to block reallocate.
	 * @param[in] sz New size of block.
	 * @returns Pointer to block if success; otherwise NULL if failed.
	 */
	void *(*realloc)(PseuVM *vm, void *ptr, size_t sz);

	/** 
	 * Function to free a block of memory.
	 *
	 * @param[in] vm Pseu instance.
	 * @param[in] Pointer to block to free.
	 */
	void (*free)(PseuVM *vm, void *ptr);
} PseuConfig;

/**
 * Type of results after code interpretation.
 */
enum pseu_result {
	/** Successfully interpreted code. */
	PSEU_RESULT_SUCCESS,
	/** Error when interpreting code. */
	PSEU_RESULT_ERROR
};

/**
 * Creates a new instance of a pseu virtual machine with the specified 
 * configuration.
 *
 * @param[in] config Pseu virtual machine configuration.
 *
 * @return Pointer to pseu virtual machine instance if success; otherwise
 * returns NULL.
 */
PseuVM *pseu_vm_new(PseuConfig *config);

/**
 * Frees the specified pseu virtual machine instance. If `pseu` is null, nothing
 * happens.
 *
 * @param[in] vm Pseu instance to free.
 */
void pseu_vm_free(PseuVM *vm);

/**
 * Sets the user data of the specified pseu virtual machine instance.
 *
 * @param[in] vm Pseu instance.
 * @param[in] data User data.
 */
void pseu_vm_set_data(PseuVM *vm, void *data);

/**
 * Gets the user data of the specified pseu virtual machine instance.
 *
 * @param[in] vm Pseu instance.
 * @return User data if set; otherwise returns NULL.
 */
void *pseu_vm_get_data(PseuVM *vm);

/**
 * Interprets the specified pseu source code using the specified pseu virtual
 * machine instance.
 * 
 * @param[in] vm Pseu instance.
 * @param[in] src Source code to interpret.
 *
 * @retval PSEU_RESULT_SUCCESS When success.
 * @retval PSEU_RESULT_ERROR When failed.
 */
int pseu_vm_eval(PseuVM *vm, const char *src);

#endif /* PSEU_H */
