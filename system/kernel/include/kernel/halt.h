/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef FELINE_HALT_H
#define FELINE_HALT_H 1

[[noreturn]] inline void halt() {
	while(true){}
	__builtin_unreachable();
}

#endif /* FELINE_HALT_H */
