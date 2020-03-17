set(LIBPSEU_INC_DIR "../include")
set(LIBPSEU_SRC
	${LIBPSEU_INC_DIR}/pseu.h
	pseu.c

	vm.h
	vm.c
	obj.h
	obj.c
	lex.h
	lex.c
	parse.c
	buf.c
	gc.c
	core.c
	core.h
	dump.c
)

# Achieve faster build time in Debug configuration by building an intermediary
# object library target `libpseu`. However since `libpseu-shared` is a shared
# library it requires position independent code and therefore `libpseu` must
# also be compiled with position independent code, reducing performance (in
# debug config).
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
	# Objects.
	add_library(libpseu OBJECT ${LIBPSEU_SRC})
	set_target_properties(libpseu PROPERTIES POSITION_INDEPENDENT_CODE ON)
	target_include_directories(libpseu PUBLIC ${LIBPSEU_INC_DIR})

	# Static library.
	add_library(libpseu-static STATIC $<TARGET_OBJECTS:libpseu>)
	# Shared library.
	add_library(libpseu-shared SHARED $<TARGET_OBJECTS:libpseu>)
	set_target_properties(libpseu-shared PROPERTIES POSITION_INDEPENDENT_CODE ON)
else()
	# Static library.
	add_library(libpseu-static STATIC ${LIBPSEU_SRC})
	# Shared library.
	add_library(libpseu-shared STATIC ${LIBPSEU_SRC})
	set_target_properties(libpseu-shared PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif()

set_target_properties(libpseu-static PROPERTIES OUTPUT_NAME "pseu")
set_target_properties(libpseu-shared PROPERTIES OUTPUT_NAME "pseu")
target_include_directories(libpseu-static PUBLIC ${LIBPSEU_INC_DIR})
target_include_directories(libpseu-shared PUBLIC ${LIBPSEU_INC_DIR})
