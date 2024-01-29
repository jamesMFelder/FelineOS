// SPDX-License-Identifier: MIT
// Copyright (c) 2023 James McNaughton Felder

#include <kernel/log.h>
#include <sys/syscall.h>

// FIXME: terminate process instead of the kernel
#define GET_ARGS_RESULT(syscall)                                               \
	auto unchecked_args =                                                      \
		static_cast<UserPtr<__syscall_##syscall##_args>>(sc_args);             \
	auto unchecked_result =                                                    \
		static_cast<UserPtr<__syscall_##syscall##_result>>(sc_result);         \
	if (!unchecked_args || !unchecked_result) {                                \
		kWarning() << "Unsafe pointer to syscall arguments ("                  \
				   << unchecked_args.unsafe_raw_get() << ") or result ("       \
				   << unchecked_result.unsafe_raw_get() << ")!";               \
		std::abort();                                                          \
	}                                                                          \
	auto args [[maybe_unused]] = unchecked_args.get();                         \
	auto result [[maybe_unused]] = unchecked_result.get();

static void sc_open(UserPtr<void> sc_args, UserPtr<void> sc_result) {
	GET_ARGS_RESULT(open);
	result->error = __syscall_open_errors::open_any;
	result->fd = -1;
}

static void sc_read(UserPtr<void> sc_args, UserPtr<void> sc_result) {
	GET_ARGS_RESULT(read);
	result->error = __syscall_read_errors::read_any;
	result->amount_read = 0;
}

static void sc_write(UserPtr<void> sc_args, UserPtr<void> sc_result) {
	GET_ARGS_RESULT(write);
	result->error = __syscall_write_errors::write_any;
	result->amount_written = 0;
}

static void sc_close(UserPtr<void> sc_args, UserPtr<void> sc_result) {
	GET_ARGS_RESULT(close);
	result->error = __syscall_close_errors::close_any;
}

// FIXME: actually implement system calls
C_LINKAGE bool syscall_handler(syscall_number which,
                               UserPtr<__syscall_noop_args> args,
                               UserPtr<__syscall_noop_result> result) {
	void (*real_func)(UserPtr<void>, UserPtr<void>) = nullptr;
	switch (which) {
	case syscall_noop:
		// Should this be callable (currently not)?
		break;
	case syscall_open:
		real_func = sc_open;
		break;
	case syscall_read:
		real_func = sc_read;
		break;
	case syscall_write:
		real_func = sc_write;
		break;
	case syscall_close:
		real_func = sc_close;
		break;
	}
	if (!real_func) {
		kWarning() << "Invalid syscall " << dec(which)
				   << " called. returning false.";
		return false;
	}
	/* If the syscall would be called with (NULL, NULL), don't bother calling
	 * it, just return that the syscall exists (this allows checking if syscalls
	 * exist). */
	if (!args.unsafe_raw_get() && !result.unsafe_raw_get()) {
		return true;
	}
	real_func(args, result);
	return true;
}
