/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <feline/kernel_exceptions.h>
#include <feline/nonzero.h>
#include <feline/tests.h>

ADD_TEST(nonzero) {
	initialize_loggers();
	NonZero<unsigned> one = 1;
	REQUIRE_EQ(one, 1u);

#ifdef LIBFELINE_ONLY
	/* We can't test exceptions in the kernel, so skip this part.
	 * It should abort, which we cannot block.
	 */
	try {
		// This should abort, so if it doesn't return a 1 to signal failure
		NonZero<unsigned> zero [[maybe_unused]] = 0;
		return 1;
	} catch (FelineError &e) {
		kLog() << e.what();
	}
#endif // LIBFELINE_ONLY
	return 0;
}
