// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_CPUID_H
#define _KERN_CPUID_H 1

#include <cpuid.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

#ifdef __cplusplus
extern "C"{
#endif
//Returns true if cpuid is int cpuid_supported
//Always call before any other calls as it sets an internal variable allowing exiting before calling any invalid instructions.
bool cpuid_supported(void);

//Returns number of supported cpuid inputs
int cpuid_max();

//Return the vendor of the cpu in vendor
//Overwrites all 13 bytes with the string
//Returns the max EAX value supported
int cpuid_vendor(char vendor[13]);
#ifdef __cplusplus
}
#endif

#endif //_KERN_CPUID_H