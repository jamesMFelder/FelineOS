/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

[[noreturn]] inline void halt() {
	while(true){}
	__builtin_unreachable();
}
