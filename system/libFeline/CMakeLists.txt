set(FELINE_OBJS
	system/libFeline/src/allocator/kallocator.cpp
	system/libFeline/src/string/itostr.cpp
	system/libFeline/src/vector/kvector.cpp
	system/libFeline/src/locking/spinlock.cpp
	system/libFeline/src/logging/logger.cpp
	system/libFeline/src/settings/settings.cpp
	PARENT_SCOPE)

if (${LIBFELINE_ONLY})
function(felineTest testname exename source)
	set(oneValueArgs TESTNAME)
	set(multiValueArgs SOURCES)
	cmake_parse_arguments(PARSE_ARGV 0 FELINE_TEST "" "${oneValueArgs}" "${multiValueArgs}")
	add_executable("${FELINE_TEST_TESTNAME}" "${FELINE_TEST_SOURCES}")
	target_link_libraries("${FELINE_TEST_TESTNAME}" feline)
	add_test(NAME "${FELINE_TEST_TESTNAME}" COMMAND "${FELINE_TEST_TESTNAME}")
endfunction()

felineTest(TESTNAME bool_int SOURCES tests/bool_int.cpp)
felineTest(TESTNAME align SOURCES tests/align.cpp)
felineTest(TESTNAME endian SOURCES tests/endian.cpp)
felineTest(TESTNAME fixed_width SOURCES tests/fixed_width.cpp)
felineTest(TESTNAME nonzero SOURCES tests/nonzero.cpp)
endif()
