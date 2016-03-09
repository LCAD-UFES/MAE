find_package (OpenCV REQUIRED)
find_package (OpenGL REQUIRED)
find_package (GLUT REQUIRED)

include_directories (${OpenCV_INCLUDE_DIR})
include_directories (${OpenGL_INCLUDE_DIR})
include_directories (${GLUT_INCLUDE_DIR})

# Isso deve ser removido...
find_package (X11 REQUIRED)
include_directories (${X11_INCLUDE_DIR})
find_package (XForms REQUIRED)
include_directories (${XFORMS_INCLUDE_DIR})

# GCC deve ser o compilador padrão ao invés do clang
if (APPLE)
	set (CMAKE_CXX_COMPILER g)
	set (CMAKE_C_COMPILER gcc)
endif ()

include_directories (
	${PROJECT_SOURCE_DIR}/include
	${CMAKE_CURRENT_BINARY_DIR})

CALL_NETCOMP (character)

set (SRCS
	${character_C_SRC}
		character_user_functions/character_user_functions.c)

#ADD_LIBRARY (character_api ${SRCS})

ADD_EXECUTABLE (character_label ${SRCS})

set (ROBOT_LIBS
	net_conn
	GA
	m
        ${OpenCV_LIBS}
        ${OPENGL_LIBRARIES}
	${GLUT_LIBRARIES})

list (APPEND ROBOT_LIBS
	${X11_LIBRARIES}
	${XFORMS_LIBRARIES})

target_link_libraries (character_label ${ROBOT_LIBS})
