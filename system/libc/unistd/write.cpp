/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <errno.h>
#ifdef __is_libk
#include <kernel/log.h>
#endif // __is_libk
#include <unistd.h>
#include <sys/syscall.h>

ssize_t write(int fd, void const *buf, size_t len) {
	__syscall_write_result res;
	__syscall_write_args args = {.fd=fd, .buffer=buf, .len=len};
	__syscall_write(args, res);

	//If it errored but we wrote something, return sucess anyways
	if (res.amount_written > 0) {
		return res.amount_written;
	}

	//We didn't write anything (either failure, or len=0), so check errors
	switch (res.error) {
		case write_none:
			return res.amount_written;
			break;

			//Please, never have this happen!
		case write_invalid_syscall:
#ifdef __is_libk
			kWarning("libc/") << "write() system call returned ENOSYS!";
			//FIXME: should we just crash in userspace?
#endif // __is_libk
			errno=ENOSYS;
			break;

			//Correct errors shall be added
		case write_any:
			errno=ENOTSUP;
			break;
	}
	return -1;
}
