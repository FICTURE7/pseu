# main target
add_executable(notpseudo main.c)
# link the core static library to it
target_link_libraries(notpseudo libnotpseudo-static)
