#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "../../kernel/drivers/include/drivers/serial.h"

/*//Writes length bytes or until putchar returns EOF
static bool print(const char* data, size_t length) {
	const unsigned char* bytes = (const unsigned char*) data;
	for (size_t i = 0; i < length; i++)
		if (putchar(bytes[i]) == EOF)
			return false;
	return true;
}*/

//Minimal wrapper around vprintf to reduce code duplication
__attribute__ ((format (printf, 1, 2))) int printf(const char *restrict format, ...){
	int retval;
	va_list data;
	va_start(data, format);
	retval=vprintf(format, data);
	va_end(data);
	return retval;
}

int vprintf(const char* restrict format, va_list parameters){

	int written = 0;

	while (*format != '\0') {
		size_t maxrem = INT_MAX - written;

		if (format[0] != '%' || format[1] == '%') {
			if (format[0] == '%')
				format++;
			size_t amount = 1;
			while (format[amount] && format[amount] != '%')
				amount++;
			if (maxrem < amount) {
				// TODO: Set errno to EOVERFLOW.
				return -1;
			}
			for(size_t count=0; count<amount; count++){
				putchar(format[count]);
			}
			format += amount;
			written += amount;
			continue;
		}

		format++;
		switch(*format){
			case 'c':
			{
				char c = (char) va_arg(parameters, int); //char promotes to int
				if (!maxrem) {
					// TODO: Set errno to EOVERFLOW.
					return -1;
				}
				if(!putchar(c)){
					return -1;
				}
				written++;
				break;
			}
			case 's':
			{
				const char *str = va_arg(parameters, const char*);
				size_t len=strlen(str);
				if (maxrem<len) {
					// TODO: Set errno to EOVERFLOW.
					return -1;
				}
				if(puts_no_nl(str)==EOF){
					return -1;
				}
				written+=len;
				break;
			}
			case 'd':
			case 'l':
			case 'z':
			case 'x':
			case 'X':
			{
				const int num = (int) va_arg(parameters, const int);
				char str[17];
				switch(*format){
					case 'd':
						itostr(num, str);
						break;
					case 'l':
					case 'z':
						switch(*(++format)){
								case 'd':
								case 'u':
									lltostr(num, str);
									break;
								case 'x':
									xlltostr(num, str);
									break;
								case 'X':
									Xlltostr(num, str);
									break;
								default:
								format--;
								goto invl_format_spec;
						}
						break;
					case 'x':
						xtostr(num, str);
						break;
					case 'X':
						Xtostr(num, str);
						break;
				}
				size_t len = strlen(str);
				if (maxrem < len) {
					// TODO: Set errno to EOVERFLOW.
					return -1;
				}
				if (puts_no_nl(str))
					return -1;
				written += len;
				break;
			}
			case 'p':
			{
				const void *num = va_arg(parameters, const void*);
				char str[19]="0x";
				Xlltostr((size_t)&num, str+2);
				size_t len=strlen(str);
				if (maxrem < len){
						// TODO: Set errno to EOVERFLOW.
						return -1;
				}
				if(puts_no_nl(str))
						return -1;
				written+=len;
				break;
			}
			default:
			{
				//goto here if the format is invalid, set format to the character after the percent
				invl_format_spec:
				if (maxrem < 2) {
					// TODO: Set errno to EOVERFLOW.
					return -1;
				}
				if (putchar('%'))
					return -1;
				if (putchar(*format))
					return -1;
				written += 2;
				format += 2;
			}
		}
		format++;
	}
	return written;
}
