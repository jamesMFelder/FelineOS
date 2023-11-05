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
	__syscall_read(args, res);

	//We didn't write anything (either failure, or len=0), so check errors
	switch (res.error) {
		case read_none:
			return res.amount_read;
			break;

			//Please, never have this happen!
		case read_invalid_syscall:
#ifdef __is_libk
			kWarning("libc/") << "read() system call returned ENOSYS!";
			//FIXME: should we just crash in userspace?
#endif // __is_libk
			errno=ENOSYS;
			break;

			//Correct errors shall be added
		case read_any:
			errno=ENOTSUP;
			break;
	}
	return -1;
}
