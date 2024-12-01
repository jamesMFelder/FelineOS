/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */
#include <cctype>
#include <feline/bool_int.h>

/* 7.4.1.10 The isspace function tests for any character that is a standard
 * white-space character or is one of a locale-specific set of characters for
 * which isalnum is false. */
int isspace(int c) {
	/* The standard white-space characters are the following: */
	switch (c) {
	/* space (’ ’), */
	case ' ':
	/* form feed (’\f’), */
	case '\f':
	/* new-line (’\n’), */
	case '\n':
	/* carriage return (’\r’), */
	case '\r':
	/* horizontal tab (’\t’), */
	case '\t':
	/* and vertical tab (’\v’). */
	case '\v':
		return INT_TRUE;
	/* In the "C" locale, isspace returns true only for the standard white-space
	 * characters.*/
	default:
		return INT_FALSE;
	}
}
