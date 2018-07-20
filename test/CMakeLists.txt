# test target
add_executable(libpseu-test
    main.c 
    test.h
    $<TARGET_OBJECTS:libpseu>
)
target_include_directories(libpseu-test PUBLIC "../include" PRIVATE "../lib")
