find_package (OpenCV REQUIRED)
find_package (OpenGL REQUIRED)
find_package (GLUT REQUIRED)

include_directories (${OPENGL_INCLUDE_DIR})
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

CALL_NETCOMP (vergence)

set (SRCS
	${vergence_C_SRC}
	vergence_user_functions/vergence_user_functions.c
	vergence_user_functions/vergence_filters.c)

#ADD_LIBRARY (vergence_api ${SRCS})

ADD_EXECUTABLE (vergence ${SRCS})

set (ROBOT_LIBS
	net_conn
	GA
	m
	${OPENCV_LIBRARY}
	${OPENGL_LIBRARIES}
	${GLUT_LIBRARIES})

list (APPEND ROBOT_LIBS
	${X11_LIBRARIES}
	${XFORMS_LIBRARIES})

target_link_libraries (vergence ${ROBOT_LIBS})
