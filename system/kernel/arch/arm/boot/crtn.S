/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */
.section .init
	/* gcc will nicely put the contents of crtend.o's .init section here. */
	bx lr

.section .fini
	/* gcc will nicely put the contents of crtend.o's .fini section here. */
	bx lr
