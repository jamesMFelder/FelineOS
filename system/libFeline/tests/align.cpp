/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <feline/align.h>
#include <feline/tests.h>
#include <limits>

ADD_TEST(align) {
	initialize_loggers();
	// Testing round_up_to_alignment with uintptr and void* with an explicit
	// alignment. Also the template that rounds up to natural alignment
	const uintptr_t zero = 0;
	// 0 rounded up should always be 0
	for (size_t align : {1zu, 3zu, 4zu, std::numeric_limits<size_t>::max()}) {
		REQUIRE_EQ(round_up_to_alignment(zero, align), 0ul);
	}
	// 1 rounded up should always be align
	for (size_t align : {1zu, 3zu, 4zu, std::numeric_limits<size_t>::max()}) {
		REQUIRE_EQ(round_up_to_alignment(1, align), align);
	}
	// FIXME: don't wrap around
	REQUIRE_EQ(round_up_to_alignment(std::numeric_limits<size_t>::max(), 2),
	           0ul)
	// Detecting existence of overload from
	// https://www.cppstories.com/2019/07/detect-overload-from-chars/
	if constexpr (!requires(void *addr) { round_up_to_alignment(addr); }) {
		kCritical()
			<< "Void should not have a natural alignment, thus "
			   "round_up_to_alignment should need an explicit alignment "
			   "with it!";
		return 1;
	}
	REQUIRE_EQ(round_up_to_alignment(0x1, sizeof(uint8_t)), 0x1ul);
	REQUIRE_EQ(round_up_to_alignment(0x1, sizeof(uint16_t)), 0x2ul);
	REQUIRE_EQ(round_up_to_alignment(0x1, sizeof(uint32_t)), 0x4ul);
	REQUIRE_EQ(round_up_to_alignment(0x1, sizeof(uint64_t)), 0x8ul);
	REQUIRE_EQ(round_up_to_alignment((uint8_t *)0x1), (uint8_t *)0x1);
	REQUIRE_EQ(round_up_to_alignment((uint16_t *)0x1), (uint16_t *)0x2);
	REQUIRE_EQ(round_up_to_alignment((uint32_t *)0x1), (uint32_t *)0x4);
	/* Disabled for now, because i686 aligns uint64_t pointers to 0x4, while
	 * x86_64 (my host system) and arm align them to 0x8 */
	// REQUIRE_EQ(round_up_to_alignment((uint64_t *)0x1), (uint64_t *)0x8);
	return 0;
}
