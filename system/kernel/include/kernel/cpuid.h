/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_CPUID_H
#define _KERN_CPUID_H 1

#include <cpuid.h>

/* Returns true if cpuid is int cpuid_supported */
/* Always call before any other calls as it sets an internal variable allowing
 * exiting before calling any invalid instructions. */
bool cpuid_supported(void);

/* Returns number of supported cpuid inputs */
unsigned int cpuid_max();

/* Return the vendor of the cpu in vendor */
/* Overwrites all 13 bytes with the string */
/* Returns the max EAX value supported */
unsigned int cpuid_vendor(char vendor[13]);

#endif /* _KERN_CPUID_H */
