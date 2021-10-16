// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder

#include <terminals/vga/vga_text.h>
#include <drivers/serial.h>
#include <cstring>
#include <cctype>

//Pointer to the video memory
vga_text_char *vga_text_term::vga_hardware_mem=(vga_text_char*)(void*)0xB8000;

vga_text_term::vga_text_term()
{
	reset();
	writestr_serial("vga_text_term::vga_text_term() called\n");
}

//TODO: clear screen at end?
vga_text_term::~vga_text_term(){
	writestr_serial("vga_text_term::~vga_text_term() called\n");
}

//Clear the screen
void vga_text_term::clear(){
	std::memset(vga_hardware_mem, ((color&0xf0)|(color>>4)), VGA_TEXT_WIDTH*VGA_TEXT_HEIGHT*sizeof(vga_text_char));
}

//Reset everything to sane values (+clear the screen)
void vga_text_term::reset(){
	setfg(VGA_COLOR_WHITE);
	setbg(VGA_COLOR_BLACK);
	clear();
}

//Scroll down a line
void vga_text_term::scroll(){
	for(unsigned char line=0; line<VGA_TEXT_HEIGHT-1; line++){
		std::memcpy(vga_hardware_mem+line*VGA_TEXT_WIDTH, vga_hardware_mem+(line+1)*VGA_TEXT_WIDTH, VGA_TEXT_WIDTH*sizeof(vga_text_char));
	}
	std::memset(vga_hardware_mem+(VGA_TEXT_HEIGHT-1)*VGA_TEXT_WIDTH, ((color&0xf0)|(color>>4)), VGA_TEXT_WIDTH*sizeof(vga_text_char));
}

//Get the maximum positions
void vga_text_term::getMaxPosition(unsigned char &maxX, unsigned char &maxY){
	maxX=VGA_TEXT_WIDTH;
	maxY=VGA_TEXT_HEIGHT;
}

//Add a character
int vga_text_term::putchar(unsigned char const c){
	//Check if it needs special handling
	if(std::iscntrl(c)){
		switch(c){
			//For a newline
			case '\n':
				//Reset x and increment y
				x=0; y++;
				//If we are off the screen scroll
				//Assumes y is valid before calling this
				if(y>=VGA_TEXT_HEIGHT){
					scroll();
					y--;
				}
				return 0;
			//Carridge return
			case '\r':
				//Go to the start of the line
				x=0;
				return 0;
			//Tab
			case '\t':
				//I think this gets all cases
				//	normal: go until a tab stop
				//	at tab stop: goes to the next (increments before check)
				//	approaching end of line: leaves you just before the end of the line
				//	at end of line: stays one before the end of the line
				//source https://vt100.net/docs/vt05-rm/chapter3.html#S3.5.6
				x+=TABSTOP-(x%TABSTOP);
				if(x>=VGA_TEXT_WIDTH){
					x=VGA_TEXT_WIDTH-1;
				}
				return 0;
			//Backspace
			//TODO: should it blank the character?
			case '\b':
				//Start of a line
				if(x==0){
					//Start of the screen
					if(y==0){
						//Do nothing
						return 0;
					}
					//Go to the last character of the previous line
					y--;
					x=VGA_TEXT_WIDTH-1;
					//Blank it
					*(vga_hardware_mem+y*VGA_TEXT_WIDTH+x)=vga_entry(' ', color);
					return 0;
				}
				//Blank the previous character and put x there
				*(vga_hardware_mem+y*VGA_TEXT_WIDTH+(x-=1))=vga_entry(' ', color);
				return 0;
			//Delete
			case 127:
				*(vga_hardware_mem+y*VGA_TEXT_WIDTH+x)=vga_entry(' ', color);
				return 0;
			//Escape
			case 27:
				return 0;
		}
	}
	//Normal characters
	*(vga_hardware_mem+y*VGA_TEXT_WIDTH+x)=vga_entry(c, color);
	x++;
	if(x>=VGA_TEXT_WIDTH){
		x=0;
		y++;
		if(y>=VGA_TEXT_HEIGHT){
			scroll();
			y--;
		}
	}
	return 0;
}

//Set the foreground
int vga_text_term::setfg(uint8_t const fg){
	//If it is too big, ignore it
	if(fg>0xf){
		return -1;
	}
	//Create a temporary to make this preemptible
	uint8_t temp_color=color;
	//Mask out the foreground
	temp_color&=0xf0;
	//Add in the new one
	temp_color|=fg;
	//Restore to the original
	color=temp_color;
	return 0;
}

//Set the background
int vga_text_term::setbg(uint8_t const bg){
	//If it is too big, ignore it
	if(bg>0xf){
		return -1;
	}
	//Create a temporary to make this preemptible
	uint8_t temp_color=color;
	//Mask out the background
	temp_color&=0x0f;
	//Add in the new one
	temp_color|=(bg<<4);
	//Restore to the original
	color=temp_color;
	return 0;
}
