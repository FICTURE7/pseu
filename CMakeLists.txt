cmake_minimum_required(VERSION 3.12.2)

# Set project.
project(pseu C)

# Set project version.
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)

set(CMAKE_C_STANDARD 99)

if(MSVC)
	add_compile_options(/W4)
else()
	add_compile_options(-Wall -Wextra -pedantic)
endif()

# Directory containing the lib.
add_subdirectory(lib)
# Directory containing the src.
add_subdirectory(src)
# Directory containing the tests.
add_subdirectory(test)
