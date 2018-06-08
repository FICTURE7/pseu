#ifndef VM_H
#define VM_H

/*
 * contains the implementaion of
 * the virtual machine
 */
struct vm {
	void *data;
	void(*init)(struct vm *vm);
	void(*deinit)(struct vm *vm);
	void(*eval)(struct vm *vm, void *ir);
};

#endif /* VM_H */
