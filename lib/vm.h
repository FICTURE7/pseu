#ifndef VM_H
#define VM_H

struct vm {
	void(*init)(struct vm *vm);
	void(*deinit)(struct vm *vm);
	void(*eval)(struct vm *vm);
};

#endif /* VM_H */
