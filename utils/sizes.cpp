/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */

/* Print the sizes of various things */

#include <iostream>
/* Output in the format of `"name": "size"eolc` */
/* eolc is either null (default) or a comma ',' if --json is the first argument */
#define PRINT_SIZE(size) std::cout << '"' << #size << "\": \"" << sizeof(size) << '"' << eolc << '\n';
/* Just a bit quicker and looks nicer with all the PRINT_SIZE() macros */
#define NL std::cout << std::endl

/* Include various headers with different types */
#include <cstddef>
#include <cstdbool>
#include <cmath>
#include <ctime>
#include <cstring>
#include <cstdint>

int main(int argc, char **argv){
	char eolc='\0';
	/* If we have an argument */
	if(argc>1){
		/* strcmp returns 0 when they are the same and 0 is false */
		if(!strcmp(argv[1], "--json")){
			/* If we are using json end each line with a comma, and start of the top-level object */
			eolc=',';
			std::cout << "{\n";
		}
		else if(!strcmp(argv[1], "--help")){
			/* Honestly, I think this help completly covers all of our features */
			std::cout << argv[0] << ": print the sizes of various types.\n";
			std::cout << "\t--json: output the results in json\n";
			std::cout << "\t--help: show this help.\n";
			return 0;
		}
	}
	/* Start with fundemental integral types */
	/* Using default signedness because it doesn't change the size */
	PRINT_SIZE(char);
	PRINT_SIZE(int);
	PRINT_SIZE(short int);
	PRINT_SIZE(long int);
	PRINT_SIZE(long long int);
	NL;
	/* Sanity check */
	/* Use unsigned because who needs a fixed-width signed number for an OS? */
	PRINT_SIZE(uint8_t);
	PRINT_SIZE(uint16_t);
	PRINT_SIZE(uint32_t);
	PRINT_SIZE(uint64_t);
	NL;
	/* More standard than fixed-width types, but I think 99% redundant today. */
	PRINT_SIZE(uint_least8_t);
	PRINT_SIZE(uint_least16_t);
	PRINT_SIZE(uint_least32_t);
	PRINT_SIZE(uint_least64_t);
	NL;
	/* Probably better to use (in theory) when you need speedy 2^n-1 values, in practice I need to check. */
	PRINT_SIZE(uint_fast8_t);
	PRINT_SIZE(uint_fast16_t);
	PRINT_SIZE(uint_fast32_t);
	PRINT_SIZE(uint_fast64_t);
	NL;
	/* Floating types and complex numbers. */
	PRINT_SIZE(float);
	PRINT_SIZE(float_t);
	PRINT_SIZE(float _Complex);
	PRINT_SIZE(double);
	PRINT_SIZE(double_t);
	PRINT_SIZE(double _Complex);
	PRINT_SIZE(long double);
	PRINT_SIZE(long double _Complex);
	NL;
	/* Misc */
	PRINT_SIZE(bool);
	PRINT_SIZE(size_t);
	PRINT_SIZE(intmax_t);
	PRINT_SIZE(uintmax_t);
	NL;
	/* Why do I care about this? */
	PRINT_SIZE(FILE);
	NL;
	/* Time keeping */
	PRINT_SIZE(time_t);
	PRINT_SIZE(timer_t);
	NL;
	/* Pointer as int */
	PRINT_SIZE(intptr_t);
	PRINT_SIZE(uintptr_t);
	/* Pointer (don't use PRINT_SIZE() because the last object in valid json doesn't end with a comma) */
	std::cout << "\"void*\": \"" << sizeof(void*) << "\"\n";
	/* If we are using json, finish the outer object */
	if(argc>1){if(!strcmp(argv[1], "--json")){std::cout << "}\n";}}
	return 0;
}
