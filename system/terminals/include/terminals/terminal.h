// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
#ifndef _TERMINAL_H
#define _TERMINAL_H 1

#include <stddef.h>
#include <stdint.h>

typedef struct term_color{
	unsigned char r, g, b;
} term_color_t;

constexpr term_color_t term_black={0, 0, 0};
constexpr term_color_t term_white={255, 255, 255};

#ifdef __cplusplus
class terminal{
	public:
		//constructor and destructor
		terminal();
		~terminal();

		//utility functions
		virtual void clear();
		virtual void reset();
		virtual void scroll()=0;

		//positioning the cursor
		//move returns an error (specific one TBD) on invalid coordinates
		static int move(unsigned char const newX, unsigned char const newY);
		static void getPosition(unsigned char *newX, unsigned char *newY);
		static void getMaxPosition(unsigned char *X, unsigned char *Y);

		//Put them at the current cursor position with current color
		//	updates the cursor position
		virtual int putchar(unsigned char const c)=0;
		virtual int puts(char const * const s);

		//Sets the color (noop for monochrome terminal)
		virtual int setfg(term_color_t const fg);
		virtual int setbg(term_color_t const bg);

	protected:
		//Location
		static unsigned char x, y;
		static unsigned char maxX, maxY;
};
#else //__cplusplus
#error "This is a c++ header file, try compiling your program with g++ or clang++ instead of gcc or clang"
#endif //__cplusplus (else)

#endif // _TERMINAL_H