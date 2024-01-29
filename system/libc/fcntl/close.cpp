/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <errno.h>
#include <exception>
#ifdef __is_libk
#include <kernel/log.h>
#endif // __is_libk
#include <fcntl.h>
#include <sys/syscall.h>

int close(int fd) {
	__syscall_close_result res;
	__syscall_close_args args = {.fd = fd};
	bool valid_syscall = __syscall_close(args, res);

	// Please, never have this happen!
	if (!valid_syscall) {
		// FIXME: how do we report the error (if at all)?
		errno = ENOSYS;
		return -1;
	}

	switch (res.error) {
	case close_none:
		return 0;
		break;

		// Correct errors shall be added
	case close_any:
		errno = ENOTSUP;
		break;
	}
	return -1;
}
