set(LIBPSEU_INCLUDE_DIR "../include")
set(LIBPSEU_SRC
    ${LIBPSEU_INCLUDE_DIR}/pseu.h
	func.h
    vm.h
    diagnostic.h
    location.h
	opcode.h
    vector.h
    pretty.h
    lexer.h
    token.h
    node.h
    parser.h
    visitor.h
    value.h  
    string.h
    state.h

    pseu.c
	func.c
    vm.c
    codegen.c
    vector.c
    pretty.c
    lexer.c
    token.c
    parser.c
    visitor.c
    value.c
    string.c
    state.c
)

# objects
add_library(libpseu OBJECT ${LIBPSEU_SRC})
target_include_directories(libpseu PUBLIC ${LIBPSEU_INCLUDE_DIR})

if(CMAKE_BUILD_TYPE MATCHES Debug)
	target_compile_definitions(libpseu PRIVATE DEBUG)
endif()

# static library
add_library(libpseu-static STATIC $<TARGET_OBJECTS:libpseu>)
target_include_directories(libpseu-static PUBLIC ${LIBPSEU_INCLUDE_DIR})

# shared library
add_library(libpseu-shared SHARED $<TARGET_OBJECTS:libpseu>)
target_include_directories(libpseu-shared PUBLIC ${LIBPSEU_INCLUDE_DIR})

# use different names for different platforms
if(WIN32)
    set_target_properties(libpseu-static PROPERTIES OUTPUT_NAME "pseu")
    set_target_properties(libpseu-shared PROPERTIES OUTPUT_NAME "pseu")
else()
    set_target_properties(libpseu-static PROPERTIES OUTPUT_NAME "libpseu")
    set_target_properties(libpseu-shared PROPERTIES OUTPUT_NAME "libpseu")
endif()
