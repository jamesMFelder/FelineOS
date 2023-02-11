/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

/* Only guard the function definition because the macro can be un/redefined by including this header file again. */
#ifndef _ASSERT_H
#define _ASSERT_H 1

#include <bits/c_compat.h>

/* Print error message and call abort() */
C_LINKAGE [[noreturn]] void _Assert(char const * const err_msg);
/* Print error message and call abort() */
/* Extra parameters because __func__ can't be concatenated with a string literal. */
C_LINKAGE [[noreturn]] void _Assert_func(char const * const, char const * const, char const * const);

/* Turn __LINE__ (a macro number) into a string */
#define __symbol2value(x) #x
#define __symbol2string(x) __symbol2value(x)

#endif /* _ASSERT_H */

/* Undefine it to prevent redefinition warnings */
#undef assert

#ifdef NDEBUG
/* Define it to a syntactically valid nothing */
#define assert(ignored) ((void)0)
#else /* NDEBUG */

/* C>=C99 or C++>=C++11 means we have __func__ */
#if __STDC_VERSION__ >= 199901L || __cplusplus >= 201103L

#define assert(expr) ( (expr) ? (void)0 : \
	_Assert_func("Assertion " #expr " failed in function ", __func__,\
		" at " __FILE__ ":" __symbol2string(__LINE__) ".") )

#else /* __STDC_VERSION__ >= 199901L || __cplusplus >=201108L */

#define assert(expr) ( (expr) ? (void)0 : \
	_Assert("Assertion " #expr " failed at " __FILE__ ":" __symbol2string(__LINE__) ".") )

#endif /* __STDC_VERSION__ >= 199901L || __cplusplus >=201108L (else) */

#endif /* NDEBUG (else) */
