#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <kernel/tty.h>

#include "vga.h"

static const size_t VGA_WIDTH = 80;
static const size_t VGA_HEIGHT = 25;
static uint16_t* const VGA_MEMORY = (uint16_t*) 0xB8000;

static size_t terminal_row;
static size_t terminal_column;
static uint8_t terminal_color;
static uint16_t* terminal_buffer;

void terminal_initialize(void) {
	terminal_row = 0;
	terminal_column = 0;
	terminal_color = vga_entry_color(VGA_COLOR_LIGHT_GREY, VGA_COLOR_BLACK);
	terminal_buffer = VGA_MEMORY;
	for (size_t y = 0; y < VGA_HEIGHT; y++) {
		for (size_t x = 0; x < VGA_WIDTH; x++) {
			const size_t index = y * VGA_WIDTH + x;
			terminal_buffer[index] = vga_entry(' ', terminal_color);
		}
	}
}

void terminal_setcolor(uint8_t color) {
	terminal_color = color;
}

void terminal_putentryat(unsigned char c, uint8_t color, size_t x, size_t y) {
	const size_t index = y * VGA_WIDTH + x;
	terminal_buffer[index] = vga_entry(c, color);
}

//Scrolls the terminal 1 line down
//TODO: scroll multiple lines? (would be much faster if needed)
void terminal_scroll(){
	size_t x, y;
	//Shift evertthing a line down
	for(y=0;y<VGA_HEIGHT-1;y++){
		for(x=0;x<VGA_WIDTH;x++){
			terminal_buffer[y*VGA_WIDTH+x]=terminal_buffer[(y+1)*VGA_WIDTH+x];
		}
	}
	//Add a new line of spaces
	for(x=0;x<VGA_WIDTH;x++){
		terminal_putentryat(' ', terminal_color, x, VGA_HEIGHT-1);
	}
}

//Add a new line, scroll if needed
void terminal_newline(){
	//Increment the line and check if it goes off the screen
	//TODO: handle previous overflow? (see the one for terminal_scroll)
	if(++terminal_row>=VGA_HEIGHT){
		//If it does scroll
		terminal_scroll();
	}
}

void terminal_putchar(char c) {
	unsigned char uc = c;
	//newlines and tabs
	//if it is a newline, put ourselves first char of next line
	if(uc=='\n'){
		terminal_column=0;
		terminal_newline();
		return;
	}
	//If it is a carridge return, put ourselves first char of this line
	else if(uc=='\r'){
		terminal_column=0;
		return;
	}
	//If it is a tab, put ourselves TABSTOP chars foreward unless we hit
	//the edge of the screen, in which case just go to the next line.
	else if(uc=='\t'){
		do{
			if(++terminal_column==VGA_WIDTH){
				terminal_column=0;
				terminal_newline();
				return;
			}
		}while(terminal_column%TABSTOP!=0);
		return;
	}
	terminal_putentryat(uc, terminal_color, terminal_column, terminal_row);
	if (++terminal_column == VGA_WIDTH) {
		terminal_column = 0;
		terminal_newline();
	}
}

void terminal_write(const char* data, size_t size) {
	for (size_t i = 0; i < size; i++)
		terminal_putchar(data[i]);
}

void terminal_writestring(const char* data) {
	terminal_write(data, strlen(data));
}