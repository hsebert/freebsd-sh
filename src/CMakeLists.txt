cmake_minimum_required(VERSION 3.1)

set(CMAKE_C_FLAGS "-DSHELL")

include_directories(bltin gen)

add_executable(sh alias.c arith_yacc.c arith_yylex.c cd.c bltin/echo.c error.c eval.c
	exec.c expand.c
	histedit.c input.c jobs.c contrib/kill.c mail.c main.c memalloc.c miscbltin.c
	mystring.c options.c output.c parser.c contrib/printf.c redir.c show.c 
	contrib/test.c trap.c var.c
	gen/builtins.c gen/nodes.c gen/syntax.c
	gen/builtins.h gen/nodes.h gen/syntax.h gen/token.h)

target_link_libraries(sh edit)
