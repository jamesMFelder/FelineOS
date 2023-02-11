/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

/* Quickly check that we are using a c++ compiler */
/* TODO: check a macro for a filename to use in the error? */
#ifndef __cplusplus
#error "You included a c++ header file in a c program." \
"Did you mean to use g++ or clang++ instead of gcc or clang?"
#endif
