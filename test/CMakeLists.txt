# test target
add_executable(libnotpseudo-test
    main.c 
    test.h
    $<TARGET_OBJECTS:libnotpseudo>
)
target_include_directories(libnotpseudo-test PUBLIC "../include" PRIVATE "../lib")
