/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <errno.h>
#include <exception>
#ifdef __is_libk
#include <kernel/log.h>
#endif // __is_libk
#include <fcntl.h>
#include <sys/syscall.h>

int open(char const *pathname, int flags) {
	__syscall_open_result res;
	__syscall_open_args args = {.path=pathname, .flags=flags};
	__syscall_open(args, res);

	switch (res.error) {
		case open_none:
			return res.fd;
			break;

			//Please, never have this happen!
		case open_invalid_syscall:
			//FIXME: how do we report the error (if at all)?
			errno=ENOSYS;
			break;

			//Correct errors shall be added
		case open_any:
			errno=ENOTSUP;
			break;
	}
	return -1;
}
