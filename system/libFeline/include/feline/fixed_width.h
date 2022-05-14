// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _FELINE_FIXED_WIDTH_H
#define _FELINE_FIXED_WIDTH_H 1

#include <feline/cpp_only.h>

#include <cstdint>

constexpr inline uint8_t operator"" _uint8_t(unsigned long long num){return static_cast<uint8_t>(num);}
constexpr inline int8_t operator"" _int8_t(unsigned long long num){return static_cast<int8_t>(num);}

constexpr inline uint16_t operator"" _uint16_t(unsigned long long num){return static_cast<uint16_t>(num);}
constexpr inline int16_t operator"" _int16_t(unsigned long long num){return static_cast<int16_t>(num);}

constexpr inline uint32_t operator"" _uint32_t(unsigned long long num){return static_cast<uint32_t>(num);}
constexpr inline int32_t operator"" _int32_t(unsigned long long num){return static_cast<int32_t>(num);}

constexpr inline uint64_t operator"" _uint64_t(unsigned long long num){return static_cast<uint64_t>(num);}
constexpr inline int64_t operator"" _int64_t(unsigned long long num){return static_cast<int64_t>(num);}

constexpr inline intptr_t operator"" _intptr_t(unsigned long long num){return static_cast<intptr_t>(num);}
constexpr inline uintptr_t operator"" _uintptr_t(unsigned long long num){return static_cast<uintptr_t>(num);}

constexpr inline unsigned long long operator"" _KiB(unsigned long long KiB){return KiB*1024ull;}
constexpr inline unsigned long long operator"" _MiB(unsigned long long MiB){return MiB*1024ull*1024ull*1024ull;}
constexpr inline unsigned long long operator"" _GiB(unsigned long long GiB){return GiB*1024ull*1024ull*1024ull;}

#endif // _FELINE_FIXED_WIDTH_H
