#ifndef _STDIO_H
#define _STDIO_H 1

#include <sys/cdefs.h>
#include <stdarg.h>

#define EOF (-1)

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

__attribute__ ((format (printf, 1, 2)))
int printf(const char* __restrict, ...);
int vprintf(const char* __restrict, va_list);
int putchar(int);
int puts(const char*);
//Not standard, but I want something to use in printf for %s
int puts_no_nl(const char*);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_STDIO_H
