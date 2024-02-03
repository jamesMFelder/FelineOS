/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#ifndef FELINE_TESTS_H
#define FELINE_TESTS_H 1

#ifdef LIBFELINE_ONLY
#include <iostream>

#define REQUIRE_EQ(m_lhs, m_rhs)                                               \
	{                                                                          \
		auto lhs = m_lhs;                                                      \
		auto rhs = m_rhs;                                                      \
		if (lhs != rhs) {                                                      \
			std::clog << "FAILED " #m_lhs "==" #m_rhs " (got " << lhs          \
					  << "==" << rhs << ")" << std::endl;                      \
			return 1;                                                          \
		}                                                                      \
	}
#else
#error "Freestanding output for tests not figured out yet!"
#endif

#endif /* FELINE_TESTS_H */
