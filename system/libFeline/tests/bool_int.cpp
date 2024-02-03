/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <feline/bool_int.h>

int main() {
	// Testing INT_FALSE, INT_TRUE, IS_FALSE and IS_TRUE
	static_assert(INT_FALSE == 0, "False should be 0!");
	static_assert(INT_TRUE != 0, "True should not be 0!");
	static_assert(INT_FALSE IS_FALSE, "False is not false!");
	static_assert(INT_TRUE IS_TRUE, "True is not true!");
	static_assert(!(INT_TRUE IS_FALSE), "True is false!");
	static_assert(!(INT_FALSE IS_TRUE), "False is true!");
	return 0;
}
