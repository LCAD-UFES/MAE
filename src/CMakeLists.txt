find_package (BISON REQUIRED)
find_package (FLEX REQUIRED)

find_package (OpenGL REQUIRED)
include_directories (${OPENGL_INCLUDE_DIR})

# isso deve ser removido... 
find_package (X11 REQUIRED)
include_directories (${X11_INCLUDE_DIR})
find_package (XForms REQUIRED)
include_directories(${XFORMS_INCLUDE_DIR})

# gera o arquivo robot.c no diretório de "build"
macro (CALL_NETCOMP robot)
set (${robot}_C_SRC ${CMAKE_CURRENT_BINARY_DIR}/${robot}.c)
add_custom_command (
	OUTPUT ${${robot}_C_SRC}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	COMMAND ${EXECUTABLE_OUTPUT_PATH}/netcomp -o ${robot} <${CMAKE_CURRENT_SOURCE_DIR}/${robot}.con
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${robot}.con)
set_source_files_properties (
	${${robot}_C_SRC}
	PROPERTIES GENERATED TRUE)
endmacro(CALL_NETCOMP)

# gera arquivos .c do interpretador e do compilador da MAE
macro (CALL_YACC_LEX input)
	BISON_TARGET(
		${input}_yacc_PARSER
		${input}_yacc.y
		${CMAKE_CURRENT_BINARY_DIR}/${input}_yacc.tab.c
		COMPILE_FLAGS "-y")
	FLEX_TARGET(
		${input}_lex_SCANNER
		${input}_lex.l
		${CMAKE_CURRENT_BINARY_DIR}/${input}_lex.c)
	ADD_FLEX_BISON_DEPENDENCY(${input}_lex_SCANNER ${input}_yacc_PARSER)
endmacro(CALL_YACC_LEX)

CALL_YACC_LEX (netcomp)
CALL_YACC_LEX (interpreter)

include_directories (
	${CMAKE_CURRENT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include)

set (SRCS
	${BISON_interpreter_yacc_PARSER_OUTPUTS}
	${FLEX_interpreter_lex_SCANNER_OUTPUTS}
	mae_control.c
	mae_control_main_true.c
	mae_control_file_cb.c
	io_display.c
	mae_control_actions_cb.c
	global.c
	syn_neu.c
	syn_neu_io.c
	syn_neu_correlate.c
	syn_neu_default.c
	syn_neu_label.c
	syn_neu_logiredu.c
	syn_neu_mem_overwrite.c
	syn_neu_minchinton_center_surround.c
	syn_neu_minchinton_fat_fast.c
	syn_neu_minchinton.c
	syn_neu_rnd_mem_type.c
	syn_neu_wisard.c
	learning.c
	connect.c
	io.c
	filter.c
	user_functions.c)
add_library (net_conn ${SRCS})

add_executable (
	netcomp
	${BISON_netcomp_yacc_PARSER_OUTPUTS}
	${FLEX_netcomp_lex_SCANNER_OUTPUTS})

target_link_libraries (netcomp ${FLEX_LIBRARIES})
target_link_libraries (net_conn ${FLEX_LIBRARIES})

add_subdirectory (libga100)
add_subdirectory (libtsai)
add_subdirectory (libstereo)
add_subdirectory (libstereo3)

