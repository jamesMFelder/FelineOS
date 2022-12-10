#ifndef _ICXXABI_H
#define _ICXXABI_H 1

#include <bits/c_compat.h>

#define ATEXIT_MAX_FUNCS 128

typedef unsigned int uarch_t;

struct atexit_func_entry_t
{
	/**
	 * Each member is at least 4 bytes large. Such that each entry is 12bytes.
	 *	* 128 * 12 = 1.5KB exact.
	 **/
	void (*destructor_func)(void *);
	void *obj_ptr;
	void *dso_handle;
};

C_LINKAGE int __cxa_atexit(void (*f)(void *), void *objptr, void *dso);
C_LINKAGE void __cxa_finalize(void *f);

#endif
