/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <cstdint>
#include <feline/endian.h>
#include <feline/fixed_width.h>
#include <feline/tests.h>

int main() {
	const uint32_t value = 0x12345678;
	const uint32_t reversed = 0x78563412;
	// Check that reverse_endian works
	REQUIRE_EQ(value, reverse_endian(reversed));

	// Check that using native_endian doesn't reverse the value
	// (reinterpreting the pointer to `value` as a pointer to an
	// array of bytes, sizeof(uint32_t) long)
	REQUIRE_EQ(value, native_endian<const uint32_t>(
						  reinterpret_cast<const std::byte *>(&value)))

	// Check that using nonnative_endian does reverse the value
	// (creating it using the reversed value means reading it
	// should give the original value)
	REQUIRE_EQ(value, nonnative_endian<const uint32_t>(
						  reinterpret_cast<const std::byte *>(&reversed)));
	return 0;
}
