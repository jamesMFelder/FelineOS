/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <errno.h>
#ifdef __is_libk
#include <kernel/log.h>
#endif // __is_libk
#include <unistd.h>
#include <sys/syscall.h>

ssize_t read(int fd, void *buf, size_t len) {
	__syscall_read_result res;
	__syscall_read_args args = {.fd=fd, .buffer=buf, .len=len};
	bool is_valid = __syscall_read(args, res);

	//Please, never have this happen!
	if (!is_valid) {
		//FIXME: how do we report the error (if at all)?
		errno = ENOSYS;
		return -1;
	}

	//We didn't write anything (either failure, or len=0), so check errors
	switch (res.error) {
		case read_none:
			return res.amount_read;
			break;

			//Correct errors shall be added
		case read_any:
			errno=ENOTSUP;
			break;
	}
	return -1;
}
