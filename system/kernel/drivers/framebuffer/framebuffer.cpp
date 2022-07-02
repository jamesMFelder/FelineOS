/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2021 James McNaughton Felder */

#include <drivers/framebuffer.h>
#include <cassert>
#include <cstdlib>
#include <kernel/log.h>
#include <kernel/paging.h>

int framebuffer::init(pixel_bgr_t *addr, uint16_t width, uint16_t height,
		uint16_t pitch, uint8_t bpp){
	/* TODO: verify anything? */
	fb.addr=addr;
	fb.width=width;
	fb.height=height;
	fb.pitch=pitch;
	/* TODO: we should support other formats */
	assert(bpp==32);
	fb.bpp=bpp;
	/* Map as many pages as the framebuffer is */
	return map_range(addr, fb.pitch*fb.height*sizeof(pixel_bgr_t), reinterpret_cast<void**>(&fb.addr), 0);
}

int framebuffer::wrapUp(bool clearScreen){
	if(clearScreen){
		putRect(0, 0, fb.width, fb.height, {0, 0, 0});
	}
	map_results unmap=unmap_range(fb.addr, fb.pitch*fb.height*sizeof(pixel_bgr_t), 0);
	if(unmap!=map_success){
		kerrorf("Error %d unmapping the framebuffer.", unmap);
		return 0;
	}
	return -1;
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
	/* Check everything is okay */
	unsigned int inval=checkParms(x, y, p);
	if(inval!=0){
		/* If we aren't setup */
		if((inval&0b1) != 0){
			return -1;
		}
		/* If the padding is bad (this shouldn't be possible, because this is a private function) */
		if((inval&0b10) != 0){
			return -2;
		}
		/* If we are off-screen */
		if((inval&0b100) != 0){
			return -3;
		}
	}
	*(fb.addr+y*fb.pitch+x)=p;
	return 0;
}

int framebuffer::putRect_bgr(uint16_t x, uint16_t y, uint16_t width, uint16_t height, pixel_bgr_t p){
	/* Check everything is okay */
	unsigned int inval=checkParms(x+width, y+height, p);
	if(inval!=0){
		/* If we aren't setup */
		if((inval&0b1) != 0){
			return -1;
		}
		/* If the padding is bad (this shouldn't be possible, because this is a private function) */
		if((inval&0b10) != 0){
			return -2;
		}
		/* If we are off-screen (TODO: truncate?) */
		if((inval&0b100) != 0){
			return -3;
		}
	}
	/* Start in the upper left */
	pixel_bgr_t *point=fb.addr+y*(fb.pitch/sizeof(pixel_bgr_t))+x;
	/* For each row */
	for(uint16_t rows=0; rows<height; rows++){
		for(uint16_t col=0; col<width; col++){
			point[col]=p;
		}
		/* Go down a row */
		point+=fb.pitch/sizeof(pixel_bgr_t);
	}
	return 0;
}

unsigned int framebuffer::checkParms(uint16_t maxRight, uint16_t maxDown, pixel_bgr_t p){
	/* No errors to start */
	unsigned int errors=0;
	/* If we aren't setup */
	if(fb.addr==nullptr){
		errors&=0b1;
	}
	/* If it's off screen */
	if(maxRight>=fb.width || maxDown>=fb.height){
		errors&=0b100;
	}
	/* If the reserved bits are non-zero */
	/* Apparently in at least one setup it switches that pixel to palette mode */
	/* Disable the compiler warning */
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	if(p.fb_padding!=0){
#pragma GCC diagnostic pop
		errors&=0b10;
	}
	return errors;
}
