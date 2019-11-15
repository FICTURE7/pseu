# Test runner.
add_executable(libpseu-test main.c)
target_link_libraries(libpseu-test libpseu-static)
target_include_directories(libpseu-test PUBLIC "../include" PRIVATE "../lib")
