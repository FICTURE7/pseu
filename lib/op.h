#ifndef PSEU_OP_H
#define PSEU_OP_H

#define VM_OP 		\
	_(END) 			\
	_(LD_KFALSE) 	\
	_(LD_KTRUE) 	\
	_(LD_KVAL) 		\
	_(LD_LOCAL) 	\
	_(ST_LOCAL) 	\
	_(LD_GLOBAL) 	\
	_(ST_GLOBAL) 	\
	_(CALL) 		\
	_(RET) 			\
	_(RET_VAL)		\

/* Opcodes which the pseu virtual machine supports. */
enum code {
	#define _(x) OP_##x,
	VM_OP
	#undef  _
};

/* Represents a pseu virutal machine instruction byte code. */
typedef uint8_t code_t;

#endif /* PSEU_OP_H */
