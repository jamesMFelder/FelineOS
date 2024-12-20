/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#include <bits/io.h>
#include <cctype>
#include <climits>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <feline/bool_int.h>
#include <feline/str.h>

/* Minimal wrapper around vprintf to reduce code duplication */
__attribute__((format(printf, 1, 2))) int printf(const char *format, ...) {
	int retval;
	va_list data;
	va_start(data, format);
	retval = vprintf(format, data);
	va_end(data);
	return retval;
}

int vprintf(const char *format, va_list parameters) {

	char intStrBuf[256] = {0};
	char *bufPtr;

	int written = 0;

	size_t min_width = 0;
	/* size_t min_precision; */
	size_t width = 0;
	/* size_t precision=0; */

	char specifier = '\0';
	char length = '\0';

	while (*format != '\0') {
		int maxrem = INT_MAX - written;

		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			int amount = 1;
			while (format[amount] != '\0' && format[amount] != '%')
				amount++;
			if (maxrem < amount || amount < 0) {
				/* TODO: Set errno to EOVERFLOW. */
				return -1;
			}
			for (int count = 0; count < amount; count++) {
				putchar(format[count]);
			}
			format += amount;
			written += amount;
			continue;
		}

		/* Prepare the buffer */
		bufPtr = intStrBuf;

		/* Reset the flags */
		bool alt_form = false;
		bool padded = false;
		bool left_justified = false;
		bool always_signed = false;

		/* Get flags */
		while (true) {
			switch (*++format) {
			case '#':
				alt_form = true;
				continue;
			case '0':
				/* padded=true; */
				continue;
			case '-':
				/* left_justified=true; */
				continue;
			case '+':
				always_signed = true;
				continue;
			}
			/* This isn't the switch's default case because we can't break */
			/* out of the loop from inside the switch (always make sure each */
			/* switch case ends with `continue;`) */
			break;
		}
		/* We overshoot by one; */
		format--;

		/* Get the explicit width (if any) */
		while (isdigit(*++format) == INT_TRUE) {
			min_width *= 10;
			min_width += static_cast<size_t>(*format - '0');
		}
		/* We overshoot by one; */
		format--;

		/* Get the explicit precision (if any) */
		/*if(*++format=='.'){
		        while(isdigit(*++format)){
		                min_precision*=10;
		                min_precision+=*format;
		        }
		}*/
		/* We overshoot by one; */
		/* format--; */

		/* Get the length specifier */
		switch (*++format) {
		/* Short */
		case 'h':
			/* hh: short short */
			if (*(format + 1) == 'h') {
				format++;
				length = 'H';
			}
			break;
		/* long */
		case 'l':
			/* ll: long long */
			if (*(format + 1) == 'l') {
				format++;
				length = 'q';
			}
			break;
		/* long double */
		case 'L':
		/* (u)intmax_t */
		case 'j':
		/* (s)size_t */
		case 'z':
		/* ptrdiff_t */
		case 't':
			length = *format;
			break;
		/* Decrement if it is anything else */
		default:
			format--;
			length = '\0';
			break;
		}
		/* Get the conversion specifier */
		specifier = *++format;
		switch (specifier) {
		/* Character (just display it); */
		case 'c':
			*bufPtr++ = static_cast<char>(va_arg(parameters, int));
			break;
		/* String (just display it); */
		case 's': {
			char *str = va_arg(parameters, char *);
			strcpy(bufPtr, str);
			bufPtr += strlen(bufPtr);
			*bufPtr-- = '\0'; /* extra safety */
			break;
		}
		case 'd':
		case 'i':
			switch (length) {
			/* char */
			case 'H': {
				signed char num =
					static_cast<signed char>(va_arg(parameters, signed int));
				if (num < 0) {
					*bufPtr++ = '-';
				} else if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(std::abs(num), bufPtr);
				break;
			}
			/* short */
			case 'h': {

				short num = static_cast<short>(va_arg(parameters, signed int));
				if (num < 0) {
					*bufPtr++ = '-';
				} else if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(std::abs(num), bufPtr);
				break;
			}
			/* int */
			case '\0': {
				signed int num =
					static_cast<signed int>(va_arg(parameters, signed int));
				if (num < 0) {
					*bufPtr++ = '-';
				} else if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(std::abs(num), bufPtr);
				break;
			}
			/* long */
			case 'l': {
				signed long num =
					static_cast<signed long>(va_arg(parameters, signed long));
				if (num < 0) {
					*bufPtr++ = '-';
				} else if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(std::abs(num), bufPtr);
				break;
			}
			/* long long */
			case 'q': {
				signed long long num = static_cast<signed long long>(
					va_arg(parameters, signed long long));
				if (num < 0) {
					*bufPtr++ = '-';
				} else if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(std::abs(num), bufPtr);
				break;
			}
			/* size_t */
			case 'z': {
				size_t num = static_cast<size_t>(va_arg(parameters, size_t));
				if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(num, bufPtr);
				break;
			}
			/* ptrdiff_t */
			case 't': {
				ptrdiff_t num =
					static_cast<ptrdiff_t>(va_arg(parameters, ptrdiff_t));
				if (num < 0) {
					*bufPtr++ = '-';
				} else if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(std::abs(num), bufPtr);
				break;
			}
			/* intmax_t */
			case 'j': {
				intmax_t num =
					static_cast<intmax_t>(va_arg(parameters, intmax_t));
				if (num < 0) {
					*bufPtr++ = '-';
				} else if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(num, bufPtr);
				break;
			}
			}
			width = strlen(bufPtr);
			bufPtr += width;
			/* precision=0; */
			break;
		case 'o':
			/* Prefix with 0 if we need to */
			if (alt_form) {
				*bufPtr++ = '0';
			}
			switch (length) {
			/* char */
			case 'H': {
				unsigned char num = static_cast<unsigned char>(
					va_arg(parameters, unsigned int));
				otostr(num, bufPtr);
				break;
			}
			/* short */
			case 'h': {
				unsigned short num = static_cast<unsigned short>(
					va_arg(parameters, unsigned int));
				otostr(num, bufPtr);
				break;
			}
			/* int */
			case '\0': {
				unsigned int num =
					static_cast<unsigned int>(va_arg(parameters, unsigned int));
				otostr(num, bufPtr);
				break;
			}
			/* long */
			case 'l': {
				unsigned long num = static_cast<unsigned long>(
					va_arg(parameters, unsigned long));
				otostr(num, bufPtr);
				break;
			}
			/* long long */
			case 'q': {
				unsigned long long num = static_cast<unsigned long long>(
					va_arg(parameters, unsigned long long));
				otostr(num, bufPtr);
				break;
			}
			/* size_t */
			case 'z': {
				size_t num = static_cast<size_t>(va_arg(parameters, size_t));
				otostr(num, bufPtr);
				break;
			}
			/* ptrdiff_t */
			case 't': {
				ptrdiff_t num =
					static_cast<ptrdiff_t>(va_arg(parameters, ptrdiff_t));
				otostr(num, bufPtr);
				break;
			}
			/* uintmax_t */
			case 'j': {
				uintmax_t num =
					static_cast<uintmax_t>(va_arg(parameters, uintmax_t));
				otostr(num, bufPtr);
				break;
			}
			}
			width = strlen(bufPtr);
			bufPtr += width;
			/* precision=0; */
			break;
		case 'u':
			switch (length) {
			/* char */
			case 'H': {
				unsigned char num = static_cast<unsigned char>(
					va_arg(parameters, unsigned int));
				if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(num, bufPtr);
				break;
			}
			/* short */
			case 'h': {
				unsigned short num = static_cast<unsigned short>(
					va_arg(parameters, unsigned int));
				if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(num, bufPtr);
				break;
			}
			/* int */
			case '\0': {
				unsigned int num =
					static_cast<unsigned int>(va_arg(parameters, unsigned int));
				if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(num, bufPtr);
				break;
			}
			/* long */
			case 'l': {
				unsigned long num = static_cast<unsigned long>(
					va_arg(parameters, unsigned long));
				if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(num, bufPtr);
				break;
			}
			/* long long */
			case 'q': {
				unsigned long long num = static_cast<unsigned long long>(
					va_arg(parameters, unsigned long long));
				if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(num, bufPtr);
				break;
			}
			/* size_t */
			case 'z': {
				size_t num = static_cast<size_t>(va_arg(parameters, size_t));
				if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(num, bufPtr);
				break;
			}
			/* ptrdiff_t */
			case 't': {
				ptrdiff_t num =
					static_cast<ptrdiff_t>(va_arg(parameters, ptrdiff_t));
				if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(num, bufPtr);
				break;
			}
			/* uintmax_t */
			case 'j': {
				uintmax_t num =
					static_cast<uintmax_t>(va_arg(parameters, uintmax_t));
				if (always_signed) {
					*bufPtr++ = '+';
				}
				itostr(num, bufPtr);
				break;
			}
			}
			width = strlen(bufPtr);
			bufPtr += width;
			/* precision=0; */
			break;
		case 'x':
			/* Prefix with 0x if we need to */
			if (alt_form) {
				*bufPtr++ = '0';
				*bufPtr++ = 'x';
			}
			switch (length) {
			/* char */
			case 'H': {
				unsigned char num = static_cast<unsigned char>(
					va_arg(parameters, unsigned int));
				xtostr(num, bufPtr, false);
				break;
			}
			/* short */
			case 'h': {
				unsigned short num = static_cast<unsigned short>(
					va_arg(parameters, unsigned int));
				xtostr(num, bufPtr, false);
				break;
			}
			/* int */
			case '\0': {
				unsigned int num =
					static_cast<unsigned int>(va_arg(parameters, unsigned int));
				xtostr(num, bufPtr, false);
				break;
			}
			/* long */
			case 'l': {
				unsigned long num = static_cast<unsigned long>(
					va_arg(parameters, unsigned long));
				xtostr(num, bufPtr, false);
				break;
			}
			/* long long */
			case 'q': {
				unsigned long long num = static_cast<unsigned long long>(
					va_arg(parameters, unsigned long long));
				xtostr(num, bufPtr, false);
				break;
			}
			/* size_t */
			case 'z': {
				size_t num = static_cast<size_t>(va_arg(parameters, size_t));
				xtostr(num, bufPtr, false);
				break;
			}
			/* ptrdiff_t */
			case 't': {
				ptrdiff_t num =
					static_cast<ptrdiff_t>(va_arg(parameters, ptrdiff_t));
				xtostr(num, bufPtr, false);
				break;
			}
			/* uintmax_t */
			case 'j': {
				uintmax_t num =
					static_cast<uintmax_t>(va_arg(parameters, uintmax_t));
				xtostr(num, bufPtr);
				break;
			}
			}
			width = strlen(bufPtr);
			bufPtr += width;
			/* precision=0; */
			break;
		case 'X':
			/* Prefix with 0X if we need to */
			if (alt_form) {
				*bufPtr++ = '0';
				*bufPtr++ = 'X';
			}
			switch (length) {
			/* char */
			case 'H': {
				unsigned char num = static_cast<unsigned char>(
					va_arg(parameters, unsigned int));
				xtostr(num, bufPtr);
				break;
			}
			/* short */
			case 'h': {
				unsigned short num = static_cast<unsigned short>(
					va_arg(parameters, unsigned int));
				xtostr(num, bufPtr);
				break;
			}
			/* int */
			case '\0': {
				unsigned int num =
					static_cast<unsigned int>(va_arg(parameters, unsigned int));
				xtostr(num, bufPtr);
				break;
			}
			/* long */
			case 'l': {
				unsigned long num = static_cast<unsigned long>(
					va_arg(parameters, unsigned long));
				xtostr(num, bufPtr);
				break;
			}
			/* long long */
			case 'q': {
				unsigned long long num = static_cast<unsigned long long>(
					va_arg(parameters, unsigned long long));
				xtostr(num, bufPtr);
				break;
			}
			/* size_t */
			case 'z': {
				size_t num = static_cast<size_t>(va_arg(parameters, size_t));
				xtostr(num, bufPtr);
				break;
			}
			/* ptrdiff_t */
			case 't': {
				ptrdiff_t num =
					static_cast<ptrdiff_t>(va_arg(parameters, ptrdiff_t));
				xtostr(num, bufPtr);
				break;
			}
			/* uintmax_t */
			case 'j': {
				uintmax_t num =
					static_cast<uintmax_t>(va_arg(parameters, uintmax_t));
				xtostr(num, bufPtr);
				break;
			}
			}
			width = strlen(bufPtr);
			bufPtr += width;
			/* precision=0; */
			break;
		case 'b':
			/* Prefix with 0x if we need to */
			if (alt_form) {
				*bufPtr++ = '0';
				*bufPtr++ = 'b';
			}
			switch (length) {
			/* char */
			case 'H': {
				unsigned char num = static_cast<unsigned char>(
					va_arg(parameters, unsigned int));
				btostr(num, bufPtr);
				break;
			}
			/* short */
			case 'h': {
				unsigned short num = static_cast<unsigned short>(
					va_arg(parameters, unsigned int));
				btostr(num, bufPtr);
				break;
			}
			/* int */
			case '\0': {
				unsigned int num =
					static_cast<unsigned int>(va_arg(parameters, unsigned int));
				btostr(num, bufPtr);
				break;
			}
			/* long */
			case 'l': {
				unsigned long num = static_cast<unsigned long>(
					va_arg(parameters, unsigned long));
				btostr(num, bufPtr);
				break;
			}
			/* long long */
			case 'q': {
				unsigned long long num = static_cast<unsigned long long>(
					va_arg(parameters, unsigned long long));
				btostr(num, bufPtr);
				break;
			}
			/* size_t */
			case 'z': {
				size_t num = static_cast<size_t>(va_arg(parameters, size_t));
				btostr(num, bufPtr);
				break;
			}
			/* ptrdiff_t */
			case 't': {
				ptrdiff_t num =
					static_cast<ptrdiff_t>(va_arg(parameters, ptrdiff_t));
				btostr(num, bufPtr);
				break;
			}
			/* uintmax_t */
			case 'j': {
				uintmax_t num =
					static_cast<uintmax_t>(va_arg(parameters, uintmax_t));
				btostr(num, bufPtr);
				break;
			}
			}
			width = strlen(bufPtr);
			bufPtr += width;
			/* precision=0; */
			break;
		case 'p': {
			unsigned long num =
				static_cast<unsigned long>(va_arg(parameters, unsigned long));
			if (alt_form) {
				xtostr(num, bufPtr, false);
			} else {
				*bufPtr++ = '0';
				*bufPtr++ = 'x';
				xtostr(num, bufPtr, false);
			}
			width = strlen(bufPtr);
			bufPtr += width;
			/* precision=0; */
			break;
		}
		/* If we don't support it yet */
		default:
			/* Rewind until we reach the percent */
			while (*format != '%') {
				format--;
			}
			/* Show the percent */
			__internal_putchar('%');
			written++;
			/* Don't repeat this loop */
			format++;
			continue;
		}

		/* Padd/justify the number */
		if (width < min_width) {
			if (left_justified) {
				char *buff_end = intStrBuf + strlen(intStrBuf);
				for (size_t i = 0; i < (min_width - width); i++) {
					*(buff_end + i) = ' ';
				}
				*(buff_end + (min_width - width)) = '\0';
			} else {
				for (size_t i = 0; i < (min_width - width); i++) {
					__internal_putchar(padded ? '0' : ' ');
				}
			}
		}

		/* Write the number */
		if (static_cast<size_t>(maxrem) < strlen(intStrBuf)) {
			/* TODO: set errono to EOVERFLOW */
			return -1;
		}
		__internal_writeStr(intStrBuf);
		written += strlen(intStrBuf);
		format++;
	}
	return written;
}
