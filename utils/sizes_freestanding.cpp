/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

/* You can use a cross compiler, but make sure to grab the asm file so you can see these numbers
 Compile without optimizations or it might just be `ret $num`
 Use one of the following commands (depending on what compiler you have installed)
	clang++ -target i686-elf -ffreestanding sizes_freestanding.cpp -S -o sizes_freestanding.S
	i686-elf-g++ -ffreestanding sizes_freestanding.cpp -S -o sizes_freestanding.S
 Then compare sizes_freestanding.S to sizes_freestanding.cpp
 You're probably going to have to start counting addl instructions from the top or bottom (make sure not to count adding to %esp) */

/* Just a way to get the value of the size into an ASM file */
#define SHOW_SIZE_IN_ASM(size) i+=sizeof(size)
/* Some freestanding headers */
#include <stddef.h>
#include <stdint.h>

int main(int argc, char **argv){
	int i=0;
	/* TODO: is there any way to insert a comment using gcc or clang so you don't have to scroll through these files side by side? */
	/* Other idea: break into functions? */
	SHOW_SIZE_IN_ASM(void*);
	SHOW_SIZE_IN_ASM(char);
	SHOW_SIZE_IN_ASM(int);
	SHOW_SIZE_IN_ASM(short int);
	SHOW_SIZE_IN_ASM(long int);
	SHOW_SIZE_IN_ASM(long long int);
	SHOW_SIZE_IN_ASM(uint8_t);
	SHOW_SIZE_IN_ASM(uint16_t);
	SHOW_SIZE_IN_ASM(uint32_t);
	SHOW_SIZE_IN_ASM(uint64_t);
	SHOW_SIZE_IN_ASM(uint_least8_t);
	SHOW_SIZE_IN_ASM(uint_least16_t);
	SHOW_SIZE_IN_ASM(uint_least32_t);
	SHOW_SIZE_IN_ASM(uint_least64_t);
	SHOW_SIZE_IN_ASM(uint_fast8_t);
	SHOW_SIZE_IN_ASM(uint_fast16_t);
	SHOW_SIZE_IN_ASM(uint_fast32_t);
	SHOW_SIZE_IN_ASM(uint_fast64_t);
	SHOW_SIZE_IN_ASM(float);
	SHOW_SIZE_IN_ASM(float _Complex);
	SHOW_SIZE_IN_ASM(double);
	SHOW_SIZE_IN_ASM(double _Complex);
	SHOW_SIZE_IN_ASM(long double);
	SHOW_SIZE_IN_ASM(long double _Complex);
	SHOW_SIZE_IN_ASM(bool);
	SHOW_SIZE_IN_ASM(size_t);
	SHOW_SIZE_IN_ASM(intmax_t);
	SHOW_SIZE_IN_ASM(uintmax_t);
	SHOW_SIZE_IN_ASM(intptr_t);
	SHOW_SIZE_IN_ASM(uintptr_t);
	/* Make sure that we use the values somehow by returning them */
	return i;
}
