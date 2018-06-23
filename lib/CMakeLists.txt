set(LIBNOTPSEUDO_INCLUDE_DIR "../include")
set(LIBNOTPSEUDO_SRC
    ${LIBNOTPSEUDO_INCLUDE_DIR}/notpseudo.h
	function.h
    vm.h
    diagnostic.h
    location.h
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

    notpseudo.c
	function.c
    vm.c
    gen.c
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
add_library(libnotpseudo OBJECT ${LIBNOTPSEUDO_SRC})
target_include_directories(libnotpseudo PUBLIC ${LIBNOTPSEUDO_INCLUDE_DIR})

# static library
add_library(libnotpseudo-static STATIC $<TARGET_OBJECTS:libnotpseudo>)
target_include_directories(libnotpseudo-static PUBLIC ${LIBNOTPSEUDO_INCLUDE_DIR})

# shared library
add_library(libnotpseudo-shared SHARED $<TARGET_OBJECTS:libnotpseudo>)
target_include_directories(libnotpseudo-shared PUBLIC ${LIBNOTPSEUDO_INCLUDE_DIR})

# use different names for different platforms
if(WIN32)
    set_target_properties(libnotpseudo-static PROPERTIES OUTPUT_NAME "notpseudo")
    set_target_properties(libnotpseudo-shared PROPERTIES OUTPUT_NAME "notpseudo")
else()
    set_target_properties(libnotpseudo-static PROPERTIES OUTPUT_NAME "libnotpseudo")
    set_target_properties(libnotpseudo-shared PROPERTIES OUTPUT_NAME "libnotpseudo")
endif()
