/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <feline/tests.h>

ADD_TEST(kvector) {
	initialize_loggers();
	KVector<uint8_t, KGeneralAllocator<uint8_t>> vec;
	REQUIRE_EQ(vec.capacity(), 0uz);
	REQUIRE_EQ(vec.size(), 0uz);
	vec.reserve(8);
	REQUIRE_EQ(vec.capacity(), 8uz);
	REQUIRE_EQ(vec.size(), 0uz);

	for (size_t i = 0; i < 8; ++i) {
		vec.push_back(i);
	}

	REQUIRE_EQ(vec.capacity(), 8uz);
	REQUIRE_EQ(vec.size(), 8uz);

	for (auto &elem : vec) {
		REQUIRE_EQ(elem, std::distance(begin(vec), &elem));
	}

	vec.clear();

	REQUIRE_EQ(vec.capacity(), 8uz);
	REQUIRE_EQ(vec.size(), 0uz);

	return 0;
}
