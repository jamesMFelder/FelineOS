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
	bool valid_syscall = __syscall_open(args, res);

	//Please, never have this happen!
	if (!valid_syscall) {
		//FIXME: how do we report the error (if at all)?
		errno=ENOSYS;
		return -1;
	}
	switch (res.error) {
		case open_none:
			return res.fd;
			break;

			//Correct errors shall be added
		case open_any:
			errno=ENOTSUP;
			break;
	}
	return -1;
}
