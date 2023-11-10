/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */

#include <terminals/terminal.h>
#include <cstring>
#include <cctype>

unsigned char terminal::x=0, terminal::y=0;

terminal::terminal()
{
	reset();
}

/* TODO: clear screen at end? */
terminal::~terminal(){
}

/* Clear the screen */
/* Inneficiently put a space in every spot. */
/* Subclass with a decent implimentation please. */
void terminal::clear(){
	unsigned char max_x, max_y;
	getMaxPosition(&max_x, &max_y);
	for(unsigned int i=0; i<max_x*max_y; i++){
		putchar(' ');
	}
}

/* Reset everything to sane values (+clear the screen) */
void terminal::reset(){
	move(0, 0);
	clear();
}

/* Move the cursor */
int terminal::move(unsigned char const newX, unsigned char const newY){
	if(x>maxX || y>maxY){
		return -1;
	}
	x=newX;
	y=newY;
	return 0;
}

/* Get the current position */
void terminal::getPosition(unsigned char *newX, unsigned char *newY){
	*newX=x;
	*newY=y;
}

/* Get the maximum position */
void terminal::getMaxPosition(unsigned char *X, unsigned char *Y){
	*X=maxX;
	*Y=maxY;
}

/* Put a string */
int terminal::puts(char const * const s){
	char const *curptr=s;
	while(*curptr){
		putchar(*curptr);
		curptr++;
	}
	/* TODO: propogate errors */
	return 0;
}
