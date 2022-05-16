// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef FELINE_BOOL_COMPAT_H
#define FELINE_BOOL_COMPAT_H 1

// Since so many functions were created pre-bool, they need to return an int
// This makes it clearer in the implimentation which is true or false
#define INT_FALSE 0
// WARNING: true is any non-zero value, so use INT_TRUE with care
//  ONLY ASSIGN FROM IT, NEVER COMPARE TO IT
#define INT_TRUE 1

// Convenience macros to hide the fact that we haven't shouldn't use INT_TRUE
// Usage:
//  if (isdigit(c) IS_TRUE)
#define IS_FALSE == INT_FALSE
#define IS_TRUE != INT_FALSE

#endif // FELINE_BOOL_COMPAT_H
