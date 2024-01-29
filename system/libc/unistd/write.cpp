/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <errno.h>
#ifdef __is_libk
#include <kernel/log.h>
#endif // __is_libk
#include <sys/syscall.h>
#include <unistd.h>

ssize_t write(int fd, void const *buf, size_t len) {
	__syscall_write_result res;
	__syscall_write_args args = {.fd = fd, .buffer = buf, .len = len};
	bool is_valid = __syscall_write(args, res);

	// Please, never have this happen!
	if (!is_valid) {
		// FIXME: how do we report the error (if at all)?
		errno = ENOSYS;
		return -1;
	}

	// If it errored but we wrote something, return sucess anyways
	if (res.amount_written > 0) {
		return res.amount_written;
	}

	// We didn't write anything (either failure, or len=0), so check errors
	switch (res.error) {
	case write_none:
		return res.amount_written;
		break;

		// Correct errors shall be added
	case write_any:
		errno = ENOTSUP;
		break;
	}
	return -1;
}
