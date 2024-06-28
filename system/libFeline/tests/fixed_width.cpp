/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <cstdint>
#include <feline/fixed_width.h>
#include <feline/tests.h>

#define TEST_TYPE(type)                                                        \
	type _test_##type = 0_##type;                                              \
	(void)_test_##type;

int main() {
	// Verify that _u?int(8|16|32|64)_t are all valid numeric literal suffixes
	TEST_TYPE(int8_t)
	TEST_TYPE(uint8_t)
	TEST_TYPE(int16_t)
	TEST_TYPE(uint16_t)
	TEST_TYPE(int32_t)
	TEST_TYPE(uint32_t)
	TEST_TYPE(int64_t)
	TEST_TYPE(uint64_t)

	// Verify that _[KMG]ib are valid numeric literal suffixes
	REQUIRE_EQ(0ull, 0_KiB)
	REQUIRE_EQ(0ull, 0_MiB)
	REQUIRE_EQ(0ull, 0_GiB)

	// Verify that they do the correct multiplication
	REQUIRE_EQ(4096ull, 4_KiB)
	REQUIRE_EQ(1024ull, 1_KiB)
	REQUIRE_EQ(1024_KiB, 1_MiB)
	REQUIRE_EQ(1024_MiB, 1_GiB)
	return 0;
}
