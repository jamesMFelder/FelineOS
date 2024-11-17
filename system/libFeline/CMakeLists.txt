add_library(feline STATIC
	src/allocator/kallocator.cpp
	src/string/itostr.cpp
	src/vector/kvector.cpp
	src/locking/spinlock.cpp
	src/logging/logger.cpp
	src/settings/settings.cpp
	)

target_include_directories(feline PUBLIC include)

if (${LIBFELINE_ONLY})
function(felineTest testname exename source)
	set(oneValueArgs TESTNAME)
	set(multiValueArgs SOURCES)
	cmake_parse_arguments(PARSE_ARGV 0 FELINE_TEST "" "${oneValueArgs}" "${multiValueArgs}")
	add_executable("${FELINE_TEST_TESTNAME}" "${FELINE_TEST_SOURCES}")
	target_link_libraries("${FELINE_TEST_TESTNAME}" feline)
	add_test(NAME "${FELINE_TEST_TESTNAME}" COMMAND "${FELINE_TEST_TESTNAME}")
endfunction()
else()
add_library(felineTest INTERFACE)
function(felineTest testname exename source)
	set(oneValueArgs TESTNAME)
	set(multiValueArgs SOURCES)
	cmake_parse_arguments(PARSE_ARGV 0 FELINE_TEST "" "${oneValueArgs}" "${multiValueArgs}")
	target_sources(felineTest INTERFACE ${FELINE_TEST_SOURCES})
	set_property(SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/${FELINE_TEST_SOURCES}"  APPEND PROPERTY COMPILE_DEFINITIONS TESTNAME=${FELINE_TEST_TESTNAME})
endfunction()
endif()

felineTest(TESTNAME bool_int SOURCES tests/bool_int.cpp)
felineTest(TESTNAME align SOURCES tests/align.cpp)
felineTest(TESTNAME endian SOURCES tests/endian.cpp)
felineTest(TESTNAME fixed_width SOURCES tests/fixed_width.cpp)
felineTest(TESTNAME kvector SOURCES tests/kvector.cpp)
felineTest(TESTNAME nonzero SOURCES tests/nonzero.cpp)
felineTest(TESTNAME ranges SOURCES tests/ranges.cpp)
