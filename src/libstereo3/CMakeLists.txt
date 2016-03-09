find_package (OpenCV REQUIRED)
include_directories (${OpenCV_INCLUDE_DIR})

find_package (Xml2 REQUIRED)
include_directories (${LIBXML2_INCLUDE_DIRS})

include_directories (
	PROJECT_SOURCE_DIR/include
	include)

add_definitions (-D__MINMAX_DEFINED)

set (SOURCES
	src/mono.cpp
	src/parser.cpp
	src/stereo_api.cpp
	src/stereo.cpp)
	
# libStereo3.a / Stereo3.lib
add_library (Stereo3 STATIC ${SOURCES})


# libStereo3.[so|dylib] / Stereo3.dll
add_library (Stereo3_shared SHARED ${SOURCES})
target_link_libraries (
	Stereo3_shared
	${OpenCV_LIBS}
	${LIBXML2_LIBRARIES})
set_target_properties (
	Stereo3_shared
	PROPERTIES OUTPUT_NAME
	"Stereo3")
