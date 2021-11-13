// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _TERMINAL_VGA_FB_H
#define _TERMINAL_VGA_FB_H

#include <stddef.h>
#include <stdint.h>
#include <terminals/terminal.h>

//Tabstops (VGA_TEXT_WIDTH%TABSTOP must equal zero or '\t' breaks)
#define TABSTOP 4

//Color
struct color{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
}

#ifdef __cplusplus
class vga_fb_term:public terminal{
	public:
		//constructor and destructor
		vga_fb_term();
		~vga_fb_term();

		//utility functions
		void clear();
		void reset();
		void scroll();

		static void getMaxPosition(unsigned char &maxX, unsigned char &maxY);

		//Put them at the current cursor position with current color
		//	updates the cursor position
		int putchar(unsigned char const c);

		//Sets the color
		int setfg(uint8_t const fg);
		int setbg(uint8_t const bg);

	private:
		uint8_t terminal::maxX=0;
		uint8_t terminal::maxY=0;

		//Color
		uint8_t color;
};
#else //__cplusplus
#error "This is a c++ header file, try compiling your program with g++ or clang++ instead of gcc or clang"
#endif //__cplusplus (else)

#endif // _TERMINAL_VGA_FB_H
