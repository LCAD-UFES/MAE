
cmake_minimum_required (VERSION 2.6)

project (MAE)

FIND_PACKAGE(OpenMP REQUIRED)
if(OPENMP_FOUND)
        message("OPENMP FOUND")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")
endif()

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Wall -mpopcnt -mfpmath=sse -msse2 -msse4.2 -ffast-math -fassociative-math -mtune=native -march=native")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Wall -mpopcnt -mfpmath=sse -msse2 -msse4.2 -ffast-math -fassociative-math -mtune=native -march=native")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Wall -mpopcnt -mfpmath=sse -msse2 -msse4.2 -ffast-math -fassociative-math -mtune=native -march=native")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -mpopcnt -mfpmath=sse -msse2 -msse4.2 -ffast-math -fassociative-math -mtune=native -march=native")

# módulos adicionais do CMake (não incluídos na distribuição padrão)
set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/modules/")

set (EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set (LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

if (WIN32)
	add_definitions (-D_CRT_SECURE_NO_WARNINGS)
endif (WIN32)

add_subdirectory (src)
add_subdirectory (examples)
