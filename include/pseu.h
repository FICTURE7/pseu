#ifndef PSEU_H
#define PSEU_H

#include <stdlib.h>

/**
 * Represents a pseu virtual machine instance.
 */
typedef struct pseu_vm pseu_vm_t;

/**
 * Type of errors.
 */
enum pseu_error_type {
	/** Runtime error. */
	PSEU_ERROR_RUNTIME,
	/** Compile time error. */
	PSEU_ERROR_COMPILE
};

/**
 * Type of warnings.
 */
enum pseu_warn_type {
	/** Runtime warning. */
	PSEU_WARN_RUNTIME,
	/** Compile time warning. */
	PSEU_WARN_COMPILE
};

/**
 * Represents the configuration of a pseu virtual machine.
 */
typedef struct pseu_config {
	/** 
	 * Callback whenever pseu has encoutered an error.
	 *
	 * @param[in] pseu Pseu instance.
	 * @param[in] type Type of error.
	 * @param[in] row Row in source which caused the error.
	 * @param[in] col Column in source which caused the error.
	 * @param[in] message Message describing the error.
	 */
	void (*onerror)(pseu_vm_t *pseu, enum pseu_error_type type,
			unsigned int row, unsigned int col, const char *message); 

	/** 
	 * Callback whenever pseu has encoutered a warning.
	 *
	 * @param[in] pseu Pseu instance.
	 * @param[in] type Type of warning.
	 * @param[in] row Row in source which caused the error.
	 * @param[in] col Column in source which caused the error.
	 * @param[in] message Message describing the error.
	 */
	void (*onwarn)(pseu_vm_t *pseu, enum pseu_warn_type type,
			unsigned int row, unsigned int col, const char *message); 

	/** 
	 * Callback whenever pseu wants to write to the standard output.
	 *
	 * @param[in] pseu Pseu instance.
	 * @param[in] text Text to write to the standard output.
	 */
	void (*print)(pseu_vm_t *pseu, const char *text); 	

	/**
	 * Fuction to allocate a block of memory of the specified size.
	 *
	 * @param[in] size Size of block to allocate.
	 * @returns Pointer to block if success; otherwise NULL if failed.
	 *
	 * @note The returned block may not necessarily return 0'd (zeroed) memory.
	 */
	void *(*alloc)(pseu_vm_t *vm, size_t size);

	/** 
	 * Function to reallocates a block of memory to a new size.
	 *
	 * @param[in] ptr Pointer to block reallocate.
	 * @param[in] size New size of block.
	 * @returns Pointer to block if success; otherwise NULL if failed.
	 */
	void *(*realloc)(pseu_vm_t *vm, void *ptr, size_t size);

	/** 
	 * Function to free a block of memory.
	 *
	 * @param[in] Pointer to block to free.
	 */
	void (*free)(pseu_vm_t *vm, void *ptr);
} pseu_config_t;

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
pseu_vm_t *pseu_vm_new(pseu_config_t *config);

/**
 * Frees the specified pseu virtual machine instance. If `pseu` is null, nothing
 * happens.
 *
 * @param[in] pseu Pseu instance to free.
 */
void pseu_vm_free(pseu_vm_t *pseu);

/**
 * Sets the user data of the specified pseu virtual machine instance.
 *
 * @param[in] pseu Pseu instance.
 * @param[in] data User data.
 */
void pseu_vm_set_data(pseu_vm_t *pseu, void *data);

/**
 * Gets the user data of the specified pseu virtual machine instance.
 *
 * @param[in] pseu Pseu instance.
 * @return User data if set; otherwise returns NULL.
 */
void *pseu_vm_get_data(pseu_vm_t *pseu);

/**
 * Interprets the specified pseu source code using the specified pseu virtual
 * machine instance.
 * 
 * @param[in] pseu Pseu instance.
 * @param[in] src Source code to interpret.
 *
 * @retval PSEU_RESULT_SUCCESS When success.
 * @retval PSEU_RESULT_ERROR When failed.
 */
int pseu_interpret(pseu_vm_t *pseu, const char *src);

#endif /* PSEU_H */
