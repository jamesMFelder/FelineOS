/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <cstdlib>

int abs(int j) { return j > 0 ? j : -j; }

long labs(long j) { return j > 0 ? j : -j; }

long long llabs(long long j) { return j > 0 ? j : -j; }

long int std::abs(long int n) { return labs(n); }

long long int std::abs(long long int n) { return llabs(n); }
