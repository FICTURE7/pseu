set(LIBPSEU_INCLUDE_DIR "../include")
set(LIBPSEU_SRC
	${LIBPSEU_INCLUDE_DIR}/pseu.h
	pseu_vm.h
	pseu_core.h
	pseu_node.h
	pseu_state.h
	pseu_value.h  
	pseu_debug.h
	pseu_lexer.h
	pseu_token.h
	pseu_string.h
	pseu_symbol.h
	pseu_parser.h
	pseu_opcode.h
	pseu_vector.h
	pseu_visitor.h
	pseu_compiler.h
	pseu_location.h

	pseu.c
	pseu_vm.c
	pseu_core.c
	pseu_node.c
	pseu_state.c
	pseu_value.c
	pseu_debug.c
	pseu_lexer.c
	pseu_token.c
	pseu_string.c
	pseu_symbol.c
	pseu_parser.c
	pseu_vector.c
	pseu_visitor.c
	pseu_compiler.c
)

# objects
add_library(libpseu OBJECT ${LIBPSEU_SRC})
target_include_directories(libpseu PUBLIC ${LIBPSEU_INCLUDE_DIR})

# c pre-processor definitions
if (PSEU_USE_ASSERT)
	target_compile_definitions(libpseu PRIVATE PSEU_USE_ASSERT)
endif()

if (PSEU_DEBUG_AST)
	target_compile_definitions(libpseu PRIVATE PSEU_DEBUG_AST)
endif()

if (PSEU_DEBUG_CODE)
	target_compile_definitions(libpseu PRIVATE PSEU_DEBUG_CODE)
endif()

# static library
add_library(libpseu-static STATIC $<TARGET_OBJECTS:libpseu>)
target_include_directories(libpseu-static PUBLIC ${LIBPSEU_INCLUDE_DIR})
set_target_properties(libpseu-static PROPERTIES OUTPUT_NAME "pseu")

# shared library
add_library(libpseu-shared SHARED $<TARGET_OBJECTS:libpseu>)
target_include_directories(libpseu-shared PUBLIC ${LIBPSEU_INCLUDE_DIR})
set_target_properties(libpseu-shared PROPERTIES OUTPUT_NAME "pseu")

