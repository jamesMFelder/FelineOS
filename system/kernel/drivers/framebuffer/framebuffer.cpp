// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder

#include <drivers/framebuffer.h>
#include <cassert>

framebuffer::framebuffer(){}
//TODO: clear screen on destruction?
framebuffer::~framebuffer(){}

int framebuffer::init(pixel_bgr_t *addr, uint16_t width, uint16_t height,
		uint16_t pitch, uint8_t bpp){
	//TODO: verify anything?
	fb.addr=addr;
	fb.width=width;
	fb.height=height;
	fb.pitch=pitch;
	assert(bpp==32);
	fb.bpp=bpp;
	return 0;
}

int framebuffer::getMax(uint16_t *x, uint16_t *y){
	*x=fb.width;
	*y=fb.height;
	return 0;
}

int framebuffer::putPixel(uint16_t x, uint16_t y, pixel_t p){
	return putPixel_bgr(x, y, rgb2bgr(p));
}

int framebuffer::putRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, pixel_t p){
	return putRect_bgr(x, y, width, height, rgb2bgr(p));
}

int framebuffer::putPixel_bgr(uint16_t x, uint16_t y, pixel_bgr_t p){
	//Check everything is okay
	int inval=checkParms(x, y, p);
	if(inval!=0){
		return inval;
	}
	*(fb.addr+y*fb.pitch+x)=p;
	return 0;
}

int framebuffer::putRect_bgr(uint16_t x, uint16_t y, uint16_t width, uint16_t height, pixel_bgr_t p){
	//Check everything is okay
	int inval=checkParms(x+width, y+height, p);
	if(inval!=0){
		return inval;
	}
	//Start in the upper left
	pixel_bgr_t *point=fb.addr+y*(fb.pitch/sizeof(pixel_bgr_t))+x;
	//For each row
	for(uint16_t rows=0; rows<height; rows++){
		for(uint16_t col=0; col<width; col++){
			point[col]=p;
		}
		//Go down a row
		point+=fb.pitch/sizeof(pixel_bgr_t);
	}
	return 0;
}

int framebuffer::checkParms(uint16_t maxRight, uint16_t maxDown, pixel_bgr_t p){
	//If we aren't setup
	if(fb.addr==nullptr){
		return -1;
	}
	//If it's off screen
	if(maxRight>=fb.width || maxDown>=fb.height){
		return -1;
	}
	//If the reserved bits are non-zero
	//Apparently in at least one setup it switches that pixel to palette mode
	//Disable the compiler warning
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	if(p.fb_padding!=0){
#pragma GCC diagnostic pop
		return -1;
	}
	return 0;
}
