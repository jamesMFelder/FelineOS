// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#define PRINT_SIZE(size) i+=sizeof(size)
#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

int main(int argc, char **argv){
	size_t i=0;
	PRINT_SIZE(void*);
	PRINT_SIZE(char);
	PRINT_SIZE(int);
	PRINT_SIZE(short int);
	PRINT_SIZE(long int);
	PRINT_SIZE(long long int);
	PRINT_SIZE(uint8_t);
	PRINT_SIZE(uint16_t);
	PRINT_SIZE(uint32_t);
	PRINT_SIZE(uint64_t);
	PRINT_SIZE(float);
	PRINT_SIZE(float _Complex);
	PRINT_SIZE(double);
	PRINT_SIZE(double _Complex);
	PRINT_SIZE(long double);
	PRINT_SIZE(long double _Complex);
	PRINT_SIZE(bool);
	PRINT_SIZE(size_t);
	PRINT_SIZE(intmax_t);
	return i;
}
