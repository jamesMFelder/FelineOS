// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _KERN_ASM_H
#define _KERN_ASM_H 1

#include <stdint.h>

static inline void outb(uint16_t port, uint8_t val){
	asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}

static inline void outw(uint16_t port, uint16_t val){
	asm volatile ( "outw %0, %1" : : "a"(val), "Nd"(port) );
}

static inline void outl(uint16_t port, uint32_t val){
	asm volatile ( "outl %0, %1" : : "a"(val), "Nd"(port) );
}

static inline uint8_t inb(uint16_t port){
	uint8_t ret;
	asm volatile ( "inb %1, %0"
	: "=a"(ret)
	: "Nd"(port) );
	return ret;
}

static inline uint16_t inw(uint16_t port){
	uint8_t ret;
	asm volatile ( "inb %1, %0"
	: "=a"(ret)
	: "Nd"(port) );
	return ret;
}

static inline uint32_t inl(uint16_t port){
	uint8_t ret;
	asm volatile ( "inb %1, %0"
	: "=a"(ret)
	: "Nd"(port) );
	return ret;
}

static inline void io_wait(void){
	/* Port 0x80 is used for 'checkpoints' during POST. */
	/* The Linux kernel seems to think it is free for use :-/ */
	outb(0x80, 0);
}

#endif ///_KERN_ASM_H
