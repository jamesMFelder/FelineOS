#include <limits.h>
#include <stdbool.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stddef.h>

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

	char intStrBuf[256]={0};
	char *bufPtr;

	int written = 0;

	bool alt_form=false;
	//bool padded=false;
	//bool left_justified=false;
	bool always_signed=false;

	size_t min_width=0;
	//size_t min_precision;
	__attribute__((unused)) size_t width=0;
	//size_t precision=0;

	char specifier='\0';
	char length='\0';

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

		//Prepair the buffer
		memset(intStrBuf, '\0', 256);
		bufPtr=intStrBuf;

		//Get flags
		while(true){
				switch(*++format){
					case '#':
						alt_form=true;
						continue;
					case '0':
						//padded=true;
						continue;
					case '-':
						//left_justified=true;
						continue;
					case '+':
						always_signed=true;
						continue;
				}
				//This isn't the switch's default case because we can't break
				//out of the loop from inside the switch (always make sure each
				//switch case ends with `continue;`)
				break;
		}
		//We overshoot by one;
		format--;

		//Get the explicit width (if any)
		while(isdigit(*++format)){
				min_width*=10;
				min_width+=*format;
		}
		//We overshoot by one;
		format--;

		//Get the explicit precision (if any)
		/*if(*++format=='.'){
				while(isdigit(*++format)){
						min_precision*=10;
						min_precision+=*format;
				}
		}
		//We overshoot by one;
		format--;*/

		//Get the length specifier
		switch(*++format){
			//Short
			case 'h':
			//hh: short short
				if(*(format+1)=='h'){
						format++;
						length='H';
				}
				break;
			//long
			case 'l':
			//ll: long long
				if(*(format+1)=='l'){
						format++;
						length='q';
				}
				break;
			//long double
			case 'L':
			//(u)intmax_t
			case 'j':
			//(s)size_t
			case 'z':
			//ptrdiff_t
			case 't':
				length=*format;
				break;
			//Decrement if it is anything else
			default:
				format--;
				length='\0';
				break;
		}
		//Get the conversion specifier
		specifier=*++format;
		switch(specifier){
			//Character (just display it);
			case 'c':
				*bufPtr++=(char)va_arg(parameters, int);
				*bufPtr++='\0';
				break;
			//String (just display it);
			case 's':
			{
				char *str=va_arg(parameters, char*);
				strcpy(bufPtr, str);
				bufPtr+=strlen(bufPtr);
				*bufPtr--='\0'; //extra safety
				break;
			}
			case 'd':
			case 'i':
				switch(length){
					//char
					case 'H':
					{
						char num=(char)va_arg(parameters, unsigned int);
						if(num<0){
							*bufPtr++='-';
						} else if(always_signed){
							*bufPtr++='+';
						}
						ctostr(abs(num), bufPtr);
						break;
					}
					//short
					case 'h':
					{

						short num=(short)va_arg(parameters, signed int);
						if(num<0){
							*bufPtr++='-';
						} else if(always_signed){
							*bufPtr++='+';
						}
						stostr(abs(num), bufPtr);
						break;
					}
					//int
					case '\0':
					{
						int num=(int)va_arg(parameters, unsigned int);
						if(num<0){
							*bufPtr++='-';
						} else if(always_signed){
							*bufPtr++='+';
						}
						itostr(abs(num), bufPtr);
						break;
					}
					//long
					case 'l':
					{
						long num=(long)va_arg(parameters, unsigned long);
						if(num<0){
							*bufPtr++='-';
						} else if(always_signed){
							*bufPtr++='+';
						}
						ltostr(abs(num), bufPtr);
						break;
					}
					//long long
					case 'q':
					{
						long long num=(long long)va_arg(parameters, unsigned long long);
						if(num<0){
							*bufPtr++='-';
						} else if(always_signed){
							*bufPtr++='+';
						}
						lltostr(abs(num), bufPtr);
						break;
					}
					//size_t
					case 'z':
					{
						size_t num=(size_t)va_arg(parameters, size_t);
						if(always_signed){
							*bufPtr++='+';
						}
						ztostr(abs(num), bufPtr);
						break;
					}
					//ptrdiff_t
					case 't':
					{
						ptrdiff_t num=(ptrdiff_t)va_arg(parameters, ptrdiff_t);
						if(num<0){
							*bufPtr++='-';
						} else if(always_signed){
							*bufPtr++='+';
						}
						ttostr(abs(num), bufPtr);
						break;
					}
				}
				width=strlen(bufPtr);
				bufPtr+=width;
				//precision=0;
				break;
			case 'o':
				//Prefix with 0 if we need to
				if(alt_form){
					*bufPtr++='0';
				}
				switch(length){
					//char
					case 'H':
					{
						char num=(char)va_arg(parameters, unsigned int);
						octostr(num, bufPtr);
						break;
					}
					//short
					case 'h':
					{
						short num=(short)va_arg(parameters, signed int);
						ostostr(num, bufPtr);
						break;
					}
					//int
					case '\0':
					{
						int num=(int)va_arg(parameters, unsigned int);
						otostr(num, bufPtr);
						break;
					}
					//long
					case 'l':
					{
						long num=(long)va_arg(parameters, unsigned long);
						oltostr(num, bufPtr);
						break;
					}
					//long long
					case 'q':
					{
						long long num=(long long)va_arg(parameters, unsigned long long);
						olltostr(num, bufPtr);
						break;
					}
					//size_t
					case 'z':
					{
						size_t num=(size_t)va_arg(parameters, size_t);
						oztostr(num, bufPtr);
						break;
					}
					//ptrdiff_t
					case 't':
					{
						ptrdiff_t num=(ptrdiff_t)va_arg(parameters, ptrdiff_t);
						ottostr(num, bufPtr);
						break;
					}
				}
				width=strlen(bufPtr);
				bufPtr+=width;
				//precision=0;
				break;
			case 'u':
				switch(length){
					//char
					case 'H':
					{
						char num=(char)va_arg(parameters, unsigned int);
						if(always_signed){
							*bufPtr++='+';
						}
						ctostr(num, bufPtr);
						break;
					}
					//short
					case 'h':
					{
						short num=(short)va_arg(parameters, signed int);
						if(always_signed){
							*bufPtr++='+';
						}
						stostr(num, bufPtr);
						break;
					}
					//int
					case '\0':
					{
						int num=(int)va_arg(parameters, unsigned int);
						if(always_signed){
							*bufPtr++='+';
						}
						itostr(num, bufPtr);
						break;
					}
					//long
					case 'l':
					{
						long num=(long)va_arg(parameters, unsigned long);
						if(always_signed){
							*bufPtr++='+';
						}
						ltostr(num, bufPtr);
						break;
					}
					//long long
					case 'q':
					{
						long long num=(long long)va_arg(parameters, unsigned long long);
						if(always_signed){
							*bufPtr++='+';
						}
						lltostr(num, bufPtr);
						break;
					}
					//size_t
					case 'z':
					{
						size_t num=(size_t)va_arg(parameters, size_t);
						if(always_signed){
							*bufPtr++='+';
						}
						ztostr(num, bufPtr);
						break;
					}
					//ptrdiff_t
					case 't':
					{
						ptrdiff_t num=(ptrdiff_t)va_arg(parameters, ptrdiff_t);
						if(always_signed){
							*bufPtr++='+';
						}
						ttostr(num, bufPtr);
						break;
					}
				}
				width=strlen(bufPtr);
				bufPtr+=width;
				//precision=0;
				break;
			case 'x':
				//Prefix with 0x if we need to
				if(alt_form){
					*bufPtr++='0';
					*bufPtr++='x';
				}
				switch(length){
					//char
					case 'H':
					{
						char num=(char)va_arg(parameters, unsigned int);
						xctostr(num, bufPtr);
						break;
					}
					//short
					case 'h':
					{
						short num=(short)va_arg(parameters, signed int);
						xstostr(num, bufPtr);
						break;
					}
					//int
					case '\0':
					{
						int num=(int)va_arg(parameters, unsigned int);
						xtostr(num, bufPtr);
						break;
					}
					//long
					case 'l':
					{
						long num=(long)va_arg(parameters, unsigned long);
						xltostr(num, bufPtr);
						break;
					}
					//long long
					case 'q':
					{
						long long num=(long long)va_arg(parameters, unsigned long long);
						xlltostr(num, bufPtr);
						break;
					}
					//size_t
					case 'z':
					{
						size_t num=(size_t)va_arg(parameters, size_t);
						xztostr(num, bufPtr);
						break;
					}
					//ptrdiff_t
					case 't':
					{
						ptrdiff_t num=(ptrdiff_t)va_arg(parameters, ptrdiff_t);
						xttostr(num, bufPtr);
						break;
					}
				}
				width=strlen(bufPtr);
				bufPtr+=width;
				//precision=0;
				break;
			case 'X':
				//Prefix with 0X if we need to
				if(alt_form){
					*bufPtr++='0';
					*bufPtr++='X';
				}
				switch(length){
					//char
					case 'H':
					{
						char num=(char)va_arg(parameters, unsigned int);
						Xctostr(num, bufPtr);
						break;
					}
					//short
					case 'h':
					{
						short num=(short)va_arg(parameters, signed int);
						Xstostr(num, bufPtr);
						break;
					}
					//int
					case '\0':
					{
						int num=(int)va_arg(parameters, unsigned int);
						Xtostr(num, bufPtr);
						break;
					}
					//long
					case 'l':
					{
						long num=(long)va_arg(parameters, unsigned long);
						Xltostr(num, bufPtr);
						break;
					}
					//long long
					case 'q':
					{
						long long num=(long long)va_arg(parameters, unsigned long long);
						Xlltostr(num, bufPtr);
						break;
					}
					//size_t
					case 'z':
					{
						size_t num=(size_t)va_arg(parameters, size_t);
						Xttostr(num, bufPtr);
						break;
					}
					//ptrdiff_t
					case 't':
					{
						ptrdiff_t num=(ptrdiff_t)va_arg(parameters, ptrdiff_t);
						Xttostr(num, bufPtr);
						break;
					}
				}
				width=strlen(bufPtr);
				bufPtr+=width;
				//precision=0;
				break;
			case 'p':
			{
				unsigned long num=(unsigned long)va_arg(parameters, unsigned long);
				if(alt_form){
					xltostr(num, bufPtr);
				} else{
					*bufPtr++='0';
					*bufPtr++='x';
					xltostr(num, bufPtr);
				}
				width=strlen(bufPtr);
				bufPtr+=width;
				//precision=0;
				break;
			}
			//If we don't support it yet
			default:
				//Rewind until we reach the percent
				while(*format!='%'){
					format--;
				}
				//Show the percent
				putchar('%');
				written++;
				//Don't repeat this loop
				format++;
				continue;
		}

		//Padd/justify the number
/*		if(width<min_width){
			if(left_justified){
				char *buff_end=intStrBuf+strlen(intStrBuf);
				for(size_t i=0; i<(min_width-width); i++){
					*(buff_end+i)=' ';
				}
				*(buff_end+(min_width-width))='\0';
			}
			else{
				for(size_t i=0; i<(min_width-width); i++){
					putchar(padded ? '0' : ' ');
				}
			}
		}*/

		//Write the number
		if(maxrem<strlen(intStrBuf)){
			//TODO: set errono to EOVERFLOW
			return -1;
		}
		puts_no_nl(intStrBuf);
		written+=strlen(intStrBuf);
		format++;
	}
	return written;
}
