include_directories (include)

find_package (OpenGL REQUIRED)
include_directories (${OPENGL_INCLUDE_DIR})

# isso deve ir embora...
find_package (X11 REQUIRED)
include_directories (${X11_INCLUDE_DIR})

add_library (
	Stereo
	src/stereo_system.cpp
	src/stereo_api.cpp
	src/camera.cpp)

add_executable (
	test_stereo
	src/stereo_system.cpp
	src/stereo_api.cpp
	src/camera.cpp)

set (LIBS Tsai ${X11_LIBRARIES} ${OPENGL_LIBRARIES})
if (NOT WIN32)
	list (APPEND LIBS m)
endif (NOT WIN32)

target_link_libraries (test_stereo ${LIBS})

set_target_properties (
	test_stereo
	PROPERTIES COMPILE_DEFINITIONS
	"_TESTING_STEREO_API")
set_target_properties (
	test_stereo
	PROPERTIES OUTPUT_NAME
	"stereo")
