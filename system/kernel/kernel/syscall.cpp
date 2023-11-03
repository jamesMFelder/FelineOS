// SPDX-License-Identifier: MIT
// Copyright (c) 2023 James McNaughton Felder

#include <kernel/log.h>
#include <sys/syscall.h>

//FIXME: actually implement system calls
C_LINKAGE void syscall_handler(syscall_number which, UserPtr<void> args, UserPtr<void> result) {
	KString args_str, result_str;
	if (args) {
		args_str = hex(reinterpret_cast<uintptr_t>(args.get()));
	}
	else {
		args_str.append("std::nullptr"_kstr_vec);
	}
	if (result) {
		result_str = hex(reinterpret_cast<uintptr_t>(result.get()));
	}
	else {
		result_str.append("std::nullptr"_kstr_vec);
	}
	kLog("kernel/") << "Syscall " << dec(which) << " called with args=" << args_str << " and result=" << result_str;
}
