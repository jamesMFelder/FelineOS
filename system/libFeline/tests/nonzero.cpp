/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <feline/kernel_exceptions.h>
#include <feline/nonzero.h>
#include <feline/tests.h>

int main() {
	NonZero<unsigned> one = 1;
	REQUIRE_EQ(one, 1u);

	try {
		// This should abort, so if it doesn't return a 1 to signal failure
		NonZero<unsigned> zero [[maybe_unused]] = 0;
		return 1;
	} catch (FelineError &e) {
		std::cout << e.what().data() << std::endl;
	}
	return 0;
}
