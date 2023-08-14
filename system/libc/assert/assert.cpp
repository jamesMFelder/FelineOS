/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <cassert>
#include <cstdio>
#include <cstdlib>

[[noreturn]] void _Assert(char const * const err_msg){
	puts(err_msg);
	std::abort();
}

[[noreturn]] void _Assert_func(char const * const err_msg, char const * const func ,char const * const end){
	printf("%s%s%s\n", err_msg, func, end);
	std::abort();
}
