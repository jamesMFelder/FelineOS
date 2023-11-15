/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _INTTYPES_H
#define _INTTYPES_H 1

#define PRId "d"
#define PRIi "i"
#define PRIo "o"
#define PRIu "u"
#define PRIx "x"
#define PRIX "X"
#define PRIb "b"

#define PRI8 "hh"
#define PRI16 "h"

#if defined(__clang__) /* For some reason, clang in insistent that uint32_t is unsigned int instead of unsigned long */
#define PRI32
#else
#define PRI32 "l"
#endif

#define PRI64 "ll"
#if defined(__arm__) || defined(__clang__) /* Uintptr is somehow int. */
#define PRIPTR
#elif defined(__i386__) && defined(__GNUC__) /* Oddly enough, for gcc on i386, int and long 32-bits, but it will only take an int. */
#define PRIPTR PRI32
#else
#error "Can't find correct size for a pointer"
#endif

#define PRId8 PRI8 PRId
#define PRId16 PRI16 PRId
#define PRId32 PRI32 PRId
#define PRId64 PRI64 PRId
#define PRIdPTR PRIPTR PRId

#define PRIi8 PRI8 PRid
#define PRIi16 PRI16 PRid
#define PRIi32 PRI32 PRid
#define PRIi64 PRI64 PRid
#define PRIiPTR PRIPTR PRid

#define PRIo8 PRI8 PRIo
#define PRIo16 PRI16 PRIo
#define PRIo32 PRI32 PRIo
#define PRIo64 PRI64 PRIo
#define PRIoPTR PRIPTR PRIo

#define PRIu8 PRI8 PRIu
#define PRIu16 PRI16 PRIu
#define PRIu32 PRI32 PRIu
#define PRIu64 PRI64 PRIu
#define PRIuPTR PRIPTR PRIu

#define PRIx8 PRI8 PRIx
#define PRIx16 PRI16 PRIx
#define PRIx32 PRI32 PRIx
#define PRIx64 PRI64 PRIx
#define PRIxPTR PRIPTR PRIx

#define PRIX8 PRI8 PRIX
#define PRIX16 PRI16 PRIX
#define PRIX32 PRI32 PRIX
#define PRIX64 PRI64 PRIX
#define PRIXPTR PRIPTR PRIX

#define PRIb8 PRI8 PRIb
#define PRIb16 PRI16 PRIb
#define PRIb32 PRI32 PRIb
#define PRIb64 PRI64 PRIb

#endif /* _INTTYPES_H */
