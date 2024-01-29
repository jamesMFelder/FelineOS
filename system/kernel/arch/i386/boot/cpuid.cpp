/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <kernel/cpuid.h>

static bool cpuid_is_supported = false;

bool cpuid_supported() {
	if (__get_cpuid_max(0, nullptr) == 0) {
		cpuid_is_supported = false;
		return false;
	}
	cpuid_is_supported = true;
	return true;
}

unsigned int cpuid_max() {
	if (!cpuid_is_supported) {
		return 0;
	}
	return __get_cpuid_max(0x80000000, nullptr);
}

unsigned int cpuid_vendor(unsigned char regs[13]) {
	if (!cpuid_is_supported) {
		memset(regs, 0, 13);
		return 0;
	}
	unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
	__get_cpuid(0, &eax, &ebx, &ecx, &edx);
	regs[0] = (ebx & 255);
	regs[1] = ((ebx & (255 << 8)) >> 8);
	regs[2] = ((ebx & (255 << 16)) >> 16);
	regs[3] = ((ebx & (255ull << 24)) >> 24);
	regs[4] = (edx & 255);
	regs[5] = ((edx & (255 << 8)) >> 8);
	regs[6] = ((edx & (255 << 16)) >> 16);
	regs[7] = ((edx & (255ull << 24)) >> 24);
	regs[8] = (ecx & 255);
	regs[9] = ((ecx & (255 << 8)) >> 8);
	regs[10] = ((ecx & (255 << 16)) >> 16);
	regs[11] = ((ecx & (255ull << 24)) >> 24);
	regs[12] = '\0';
	return eax;
}
