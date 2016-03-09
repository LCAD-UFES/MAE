if (APPLE)
	set (CMAKE_C_COMPILER gcc)
endif ()

add_library (
	GA 
	ga.c
	select.c
	cross.c
	mutate.c
	replace.c
	function.c
	config.c
	pool.c
	chrom.c
	report.c)

set (LIBS GA)
if (NOT WIN32)
	list (APPEND LIBS m)
endif (NOT WIN32)

add_executable (ga-test ga-test.c)
target_link_libraries (ga-test ${LIBS})

add_executable (ga-test-int ga-test-int.c)
target_link_libraries (ga-test-int ${LIBS})

