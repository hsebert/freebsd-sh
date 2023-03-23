cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_FLAGS "-DSHELL -D'__FBSDID(x)='")

include_directories(bltin gen)

add_executable(mknodes mknodes.c)
add_executable(mksyntax mksyntax.c)

add_custom_command(
	OUTPUT builtins.c builtins.h
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/mkbuiltins ${CMAKE_CURRENT_SOURCE_DIR}
	DEPENDS mkbuiltins builtins.def)

add_custom_command(
	OUTPUT syntax.c syntax.h
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mksyntax
	DEPENDS mksyntax.c)

add_custom_command(
	OUTPUT token.h
	COMMAND /bin/sh ${CMAKE_CURRENT_SOURCE_DIR}/mktokens
	DEPENDS mktokens)

add_custom_command(
	OUTPUT nodes.c nodes.h
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/mknodes ${CMAKE_CURRENT_SOURCE_DIR}/nodetypes
	    ${CMAKE_CURRENT_SOURCE_DIR}/nodes.c.pat
	DEPENDS mknodes nodetypes nodes.c.pat)

add_executable(sh alias.c arith_yacc.c arith_yylex.c cd.c bltin/echo.c error.c eval.c
	exec.c expand.c
	histedit.c input.c jobs.c contrib/kill.c mail.c main.c memalloc.c miscbltin.c
	mystring.c options.c output.c parser.c contrib/printf.c redir.c show.c
	contrib/test.c trap.c var.c
	${CMAKE_CURRENT_BINARY_DIR}/builtins.c
	${CMAKE_CURRENT_BINARY_DIR}/builtins.h
	${CMAKE_CURRENT_BINARY_DIR}/nodes.c
	${CMAKE_CURRENT_BINARY_DIR}/nodes.h
	${CMAKE_CURRENT_BINARY_DIR}/syntax.c
	${CMAKE_CURRENT_BINARY_DIR}/syntax.h
	${CMAKE_CURRENT_BINARY_DIR}/token.h)

target_include_directories(sh PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(sh edit)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
	target_link_libraries(sh bsd)
endif()

target_compile_options(sh PRIVATE -Wp,-w -Wall -Wextra)
