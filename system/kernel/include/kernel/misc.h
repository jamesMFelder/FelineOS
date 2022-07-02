/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */

#include <cstdint>

inline bool get_flag(uint32_t flags, unsigned int which){
	return flags & which;
}
