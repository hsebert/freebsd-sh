cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_FLAGS "-DSHELL -D'__FBSDID(x)='")

include_directories(bltin gen)

add_executable(mknodes mknodes.c)
add_executable(mksyntax mksyntax.c)

add_custom_command(
	OUTPUT builtins.c builtins.h
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mkbuiltins ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS mkbuiltins builtins.def)

add_executable(sh alias.c arith_yacc.c arith_yylex.c cd.c bltin/echo.c error.c eval.c
	exec.c expand.c
	histedit.c input.c jobs.c contrib/kill.c mail.c main.c memalloc.c miscbltin.c
	mystring.c options.c output.c parser.c contrib/printf.c redir.c show.c 
	contrib/test.c trap.c var.c
	${CMAKE_CURRENT_BINARY_DIR}/builtins.c gen/nodes.c gen/syntax.c
	${CMAKE_CURRENT_BINARY_DIR}/builtins.h gen/nodes.h gen/syntax.h gen/token.h)

target_include_directories(sh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(sh edit)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(sh bsd)
endif()

target_compile_options(sh PRIVATE -Wp,-w -Wall -Wextra)
