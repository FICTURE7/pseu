# main target
add_executable(pseu main.c)
# link the core static library to it
target_link_libraries(pseu libpseu-static)
