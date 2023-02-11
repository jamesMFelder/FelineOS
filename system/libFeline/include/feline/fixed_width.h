/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _FELINE_FIXED_WIDTH_H
#define _FELINE_FIXED_WIDTH_H 1

#include <feline/cpp_only.h>

#include <cstdint>

constexpr inline uint8_t __attribute__((always_inline)) operator"" _uint8_t(unsigned long long num){return static_cast<uint8_t>(num);}
constexpr inline int8_t __attribute((always_inline)) operator"" _int8_t(unsigned long long num) noexcept {return static_cast<int8_t>(num);}

constexpr inline uint16_t __attribute((always_inline)) operator"" _uint16_t(unsigned long long num) noexcept {return static_cast<uint16_t>(num);}
constexpr inline int16_t __attribute((always_inline)) operator"" _int16_t(unsigned long long num) noexcept {return static_cast<int16_t>(num);}

constexpr inline uint32_t __attribute((always_inline)) operator"" _uint32_t(unsigned long long num) noexcept {return static_cast<uint32_t>(num);}
constexpr inline int32_t __attribute((always_inline)) operator"" _int32_t(unsigned long long num) noexcept {return static_cast<int32_t>(num);}

constexpr inline uint64_t __attribute((always_inline)) operator"" _uint64_t(unsigned long long num) noexcept {return static_cast<uint64_t>(num);}
constexpr inline int64_t __attribute((always_inline)) operator"" _int64_t(unsigned long long num) noexcept {return static_cast<int64_t>(num);}

constexpr inline intptr_t __attribute((always_inline)) operator"" _intptr_t(unsigned long long num) noexcept {return static_cast<intptr_t>(num);}
constexpr inline uintptr_t __attribute((always_inline)) operator"" _uintptr_t(unsigned long long num) noexcept {return static_cast<uintptr_t>(num);}

constexpr inline unsigned long long __attribute((always_inline)) operator"" _KiB(unsigned long long KiB) noexcept {return KiB*1024ull;}
constexpr inline unsigned long long __attribute((always_inline)) operator"" _MiB(unsigned long long MiB) noexcept {return MiB*1024ull*1024ull*1024ull;}
constexpr inline unsigned long long __attribute((always_inline)) operator"" _GiB(unsigned long long GiB) noexcept {return GiB*1024ull*1024ull*1024ull;}

#endif /* _FELINE_FIXED_WIDTH_H */
