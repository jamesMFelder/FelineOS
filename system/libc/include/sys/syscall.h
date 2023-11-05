/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_SYSCALL_H
#define _KERN_SYSCALL_H 1

#include <bits/c_compat.h>
#include <stddef.h>

#ifdef __is_kernel
#include <kernel/user_ptr.h>
#define USER_PTR(p) UserPtr<p>
#else // __is_kernel
#define USER_PTR(p) p*
#endif // __is_kernel (else)

enum syscall_number {
	syscall_noop,
	syscall_open,
	syscall_read,
	syscall_write,
	syscall_close,
};

// Returns false if the syscall is not supported by the kernel (args and result are untouched)
C_LINKAGE bool raw_syscall(syscall_number which, void* args, void* result);

typedef int fd_type;

#define SYSCALL_ENUMERATE(_S) \
	_S( \
			noop, \
			, \
			, \
			\
	)\
	_S( \
			open, \
			USER_PTR(char const) path; int flags; , \
			open_any , \
			fd_type fd; \
	)\
	_S( \
			close, \
			fd_type fd; , \
			close_any , \
			; \
	)\
	_S( \
			read, \
			fd_type fd; USER_PTR(void) buffer; size_t len; , \
			read_any , \
			size_t amount_read; \
	)\
	_S( \
			write, \
			fd_type fd; USER_PTR(void const) buffer; size_t len; , \
			write_any , \
			size_t amount_written; \
	)\

#define _S(name, args, errors, result) \
	struct __syscall_ ## name ## _args { \
		args \
	}; \
	enum __syscall_ ## name ## _errors { \
		name ## _none, \
		errors \
	}; \
	struct __syscall_ ## name ## _result { \
		__syscall_ ## name ## _errors error; \
		result \
	}; \
	inline bool __syscall_ ## name(__syscall_ ## name ## _args &args_param, __syscall_ ## name ## _result &result_param) { \
		return raw_syscall(syscall_ ## name, &args_param, &result_param); \
	}

SYSCALL_ENUMERATE(_S)
#undef _S

#endif /* _KERN_SYSCALL_H */
