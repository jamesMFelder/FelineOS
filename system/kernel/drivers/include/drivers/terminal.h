/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2024 James McNaughton Felder */
#ifndef _KERN_DRIVER_TERMINAL_H
#define _KERN_DRIVER_TERMINAL_H 1

#include <cstdint>
#include <kernel/phys_addr.h>

/* Width and height */
#define VGA_TEXT_HEIGHT 25
#define VGA_TEXT_WIDTH 80

/* Tabstops (VGA_TEXT_WIDTH%TABSTOP must equal zero or '\t' breaks) */
#define TABSTOP 4

static_assert(VGA_TEXT_WIDTH % TABSTOP == 0,
              "VGA_TEXT_WIDTH is not a multiple of TABSTOP: tab character "
              "handling will be broken");

/* The size of a VGA character+attributes */
typedef uint16_t vga_text_char;

/* VGA text mode colors */
/* The full color is `bg<<4 | fg` */
enum vga_color {
	VGA_COLOR_BLACK = 0,
	VGA_COLOR_BLUE = 1,
	VGA_COLOR_GREEN = 2,
	VGA_COLOR_CYAN = 3,
	VGA_COLOR_RED = 4,
	VGA_COLOR_MAGENTA = 5,
	VGA_COLOR_BROWN = 6,
	VGA_COLOR_LIGHT_GREY = 7,
	VGA_COLOR_DARK_GREY = 8,
	VGA_COLOR_LIGHT_BLUE = 9,
	VGA_COLOR_LIGHT_GREEN = 10,
	VGA_COLOR_LIGHT_CYAN = 11,
	VGA_COLOR_LIGHT_RED = 12,
	VGA_COLOR_LIGHT_MAGENTA = 13,
	VGA_COLOR_LIGHT_BROWN = 14,
	VGA_COLOR_WHITE = 15,
};

static inline uint8_t vga_color(enum vga_color fg, enum vga_color bg) {
	return fg | bg << 4;
}

static inline vga_text_char vga_entry(unsigned char uc, uint8_t color) {
	return (vga_text_char)uc | (vga_text_char)color << 8;
}

class vga_text_term {
	public:
		/* constructor and destructor */
		vga_text_term();
		~vga_text_term();

		/* Initialize */
		int init(PhysAddr<vga_text_char> addr);

		/* utility functions */
		void clear();
		void reset();
		void scroll();

		static void getMaxPosition(unsigned char &maxX, unsigned char &maxY);

		/* Put them at the current cursor position with current color */
		/* Updates the cursor position */
		int putchar(char const c);
		int putstr(char const *c);

		/* Sets the color */
		int setfg(uint8_t const fg);
		int setbg(uint8_t const bg);

	private:
		/* Pointer to the video memory */
		vga_text_char *vga_hardware_mem;

		/* Current cursor location */
		uint8_t x, y;

		/* Color */
		uint8_t color;
};

#endif /* _KERN_DRIVER_TERMINAL_H */
