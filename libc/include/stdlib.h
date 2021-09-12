#ifndef _STDLIB_H
#define _STDLIB_H 1

#include <sys/cdefs.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif //__cplusplus

__attribute__((__noreturn__))
void abort(void);

int abs(int j);
long labs(long j);
long long llabs(long long j);

#ifdef __cplusplus
}
#endif //__cplusplus

#endif // _STDLIB_H
