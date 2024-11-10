/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */
#ifndef FELINE_KERNEL_EXCEPTIONS_H
#define FELINE_KERNEL_EXCEPTIONS_H 1

#include <feline/kstring.h>

#ifdef LIBFELINE_ONLY

#include <exception>

class FelineError : std::exception {
	public:
		FelineError(KStringView message) : msg(message) {}
		char const *what() const noexcept { return msg.data(); }

	private:
		KStringView msg;
};

[[noreturn]] void inline report_fatal_error(KStringView const &err) {
	throw FelineError(err);
}

#else

#include <feline/logger.h>

[[noreturn]] void inline report_fatal_error(KStringView const &err) {
	kCritical() << err;
	std::abort();
}

#endif

#endif /* FELINE_KERNEL_EXCEPTIONS_H */
