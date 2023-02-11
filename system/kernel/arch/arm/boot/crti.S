/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
.section .init
.global _init
_init:
	push {r4, lr}
	/* gcc will nicely put the contents of crtbegin.o's .init section here. */

.section .fini
.global _fini
_fini:
	push {r4, lr}
	/* gcc will nicely put the contents of crtbegin.o's .fini section here. */
