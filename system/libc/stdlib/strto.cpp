/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */

#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <cstring>
/* stdckdint.h requires _Bool as a c header, but c++ doesn't define it */
#include <stdbool.h>
#include <stdckdint.h>

unsigned long strtoul(const char *nptr, char **endptr, int base) {
	/* Store this for returning errors. */
	const char *const original_nptr = nptr;
	/* Base 0 is auto, otherwise base must be in [2, 36] */
	if (base == 1 || base > 36) {
		errno = EINVAL;
	}
	/* Find the first non-whitespace character */
	while (std::isspace(*nptr)) {
		++nptr;
	}
	/* Default to a positive number */
	bool negative = false;
	/* If it is negative, note that and advance over the sign */
	if (std::strncmp(nptr, "-", 1)) {
		negative = true;
		++nptr;
	}
	/* Otherwise, if it is explicitly positive, note that (redundantly), and
	 * advance over the sign.*/
	else if (std::strncmp(nptr, "+", 1)) {
		negative = false;
		++nptr;
	}
	/* If it is base 16, or auto, skip a hexadecimal-number prefix */
	if (base == 16 || base == 0) {
		if (std::strncmp(nptr, "0x", 2) || std::strncmp("0X", nptr, 2)) {
			nptr += 2;
			base = 16;
		}
	}
	/* If it is base 2, or auto, skip a binary-number prefix */
	if (base == 2 || base == 0) {
		if (std::strncmp(nptr, "0b", 2) || std::strncmp("0B", nptr, 2)) {
			nptr += 2;
			base = 2;
		}
	}
	/* If it is base 8, or auto, skip an octal-number prefix */
	if (base == 8 || base == 0) {
		if (std::strncmp(nptr, "0", 1)) {
			nptr += 1;
			base = 8;
		}
	}
	/* nptr now points to the start of the digits (if there are any) */
	const char *const begining_of_digits = nptr;

	/* Represent a value that isn't a valid digit. Since base 36 is the largest
	 * we can accept, it can never be a digit value. */
	constexpr unsigned short NOT_DIGIT = 36;
	auto digit_to_num = [](char digit) -> unsigned short {
		if (digit >= '0' && digit <= '9') {
			return digit - '0';
		}
		if (digit >= 'a' && digit <= 'z') {
			return digit - 'a' + 10;
		}
		if (digit >= 'A' && digit <= 'Z') {
			return digit - 'A' + 10;
		}
		return NOT_DIGIT;
	};
	auto is_valid_digit = [base, digit_to_num](char digit) -> bool {
		/* NOT_DIGIT is always >= base, so if a digit isn't theoretically valid,
		 * it will still fail this comparison. */
		return digit_to_num(digit) < base;
	};

	/* Do the conversion for all valid digits. */
	unsigned long result = 0;
	while (is_valid_digit(*nptr)) {
		/* Multiply by base, and then add the next digit, but these functions
		 * return true(?) if an overflow occurs, so we can return an error. */
		if (ckd_mul(&result, result, base) ||
		    ckd_add(&result, result, digit_to_num(*nptr))) {
			errno = ERANGE;
			return 0;
		}
	}
	if (nptr == begining_of_digits) {
		/* There were no valid digits */
		errno = EINVAL;
		/* If the subject sequence is empty or does not have the expected form,
		 * no conversion is performed; the value of nptr is stored in the object
		 * pointed to by endptr, provided that endptr is not a null pointer. */
		if (endptr) {
			/* We don't write to it, and nptr could have been writeable before
			 * entering this function. */
			*endptr = const_cast<char *>(original_nptr);
		}
		return 0;
	}
	if (endptr) {
		/* We don't write to it, and nptr could have been writeable before
		 * entering this function. */
		*endptr = const_cast<char *>(nptr);
	}
	if (negative) {
		result = -result;
	}
	return result;
}
