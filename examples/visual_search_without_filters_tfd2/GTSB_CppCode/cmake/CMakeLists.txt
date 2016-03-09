cmake_minimum_required(VERSION 2.6)

project( GTSDB )

SET( LIBRARY_OUTPUT_PATH ../lib )

IF( 0 )
 STRING( REPLACE "/O2" "/Od" CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} )
 STRING( REPLACE "/O2" "/Od" CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} )
 SET( CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_C_FLAGS_RELWITHDEBINFO} CACHE STRING "compiler-flags" FORCE )
 SET( CMAKE_CXX_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO} CACHE STRING "compiler-flags" FORCE )
ENDIF()


include_directories(
	../include
)

set( INCS 
	../include/gtsdbTools.h
)

set( SRCS 
	../src/gtdsbTools.cpp
	../src/main.cpp
)

add_executable( ${PROJECT_NAME} 
	${SRCS} 
	${INCS} 
)
