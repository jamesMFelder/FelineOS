/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */
#ifndef FELINE_KERNEL_EXCEPTIONS_H
#define FELINE_KERNEL_EXCEPTIONS_H 1

#include <feline/kstring.h>

class GenericKernelError {
	public:
		GenericKernelError(KStringView message) : msg(message) {}
		KStringView what() const { return msg; }

	private:
		KStringView msg;
};

#endif /* FELINE_KERNEL_EXCEPTIONS_H */
