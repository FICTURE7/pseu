set(LIBNOTPSEUDO_INCLUDE_DIR "../include")
set(LIBNOTPSEUDO_SRC
    ${LIBNOTPSEUDO_INCLUDE_DIR}/notpseudo.h
    diagnostic.h
    location.h
    vector.h
    pretty.h
    lexer.h
    token.h
    node.h
    parser.h
    visitor.h
    object.h  

    notpseudo.c
    diagnostic.c
    vector.c
    pretty.c
    lexer.c
    token.c
    parser.c
    visitor.c
    object.c
)

add_library(libnotpseudo OBJECT ${LIBNOTPSEUDO_SRC})
target_include_directories(libnotpseudo PUBLIC ${LIBNOTPSEUDO_INCLUDE_DIR})

# static library
add_library(libnotpseudo-static STATIC $<TARGET_OBJECTS:libnotpseudo>)
target_include_directories(libnotpseudo-static PUBLIC ${LIBNOTPSEUDO_INCLUDE_DIR})
set_target_properties(libnotpseudo-static PROPERTIES OUTPUT_NAME libnotpseudo)

# shared library
add_library(libnotpseudo-shared SHARED $<TARGET_OBJECTS:libnotpseudo>)
target_include_directories(libnotpseudo-shared PUBLIC ${LIBNOTPSEUDO_INCLUDE_DIR})
set_target_properties(libnotpseudo-shared PROPERTIES OUTPUT_NAME libnotpseudo)
