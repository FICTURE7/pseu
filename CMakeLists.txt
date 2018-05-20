cmake_minimum_required(VERSION 3.0.2)

# set project
project(notpseudo C)

# set project version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 1)
set(VERSION_PATCH 0)

# directory containing the lib
add_subdirectory(lib)
# directory containing the src
add_subdirectory(src)
# directory containing the tests
add_subdirectory(test)
