/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

.global enable_paging

//r0: ttbr0
//r1: ttbr1
//r2: ttbc
enable_paging:
	mcr p15, 0, r0, c2, c0, 0
	mcr p15, 0, r1, c2, c0, 1
	mcr p15, 0, r2, c2, c0, 2
	bx lr
