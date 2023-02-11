/* SPDX-License-Identifier: MIT */
/* Copyright (c) 2023 James McNaughton Felder */
#ifndef _KERN_DRIVER_FB_H
#define _KERN_DRIVER_FB_H 1

#include <stdint.h>

typedef struct pixel{
	uint8_t red;
	uint8_t green;
	uint8_t blue;
} pixel_t;

typedef struct pixel_bgr{
	uint8_t blue;
	uint8_t green;
	uint8_t red;
	/* For 32-bit color framebuffers */
	[[deprecated("Can cause the pixel to be interpreted differently.")]] uint8_t fb_padding=0;
} pixel_bgr_t;

inline pixel_t bgr2rgb(pixel_bgr_t bgr){
	return {bgr.red, bgr.green, bgr.blue};
}

inline pixel_bgr_t rgb2bgr(pixel_t rgb){
	return {rgb.blue, rgb.green, rgb.red};
}

typedef struct fbInfo{
	pixel_bgr_t *addr; /* The actual framebuffer; */
	uint16_t width; /* pixels per line */
	uint16_t height; /* line on the screen; */
	uint16_t pitch; /* bytes per line (account for extra padding) */
	uint8_t bpp; /* bits per pixel (32(good), 24(bad), 16(old)) */
} fbInfo_t;

class framebuffer{
	public:
		/* Actually setup the framebuffer (TODO: merge with the constructor) */
		/* map_range must be functional, but paging doesn't need to be on */
		int init(pixel_bgr_t *addr, uint16_t width, uint16_t height,
				uint16_t pitch, uint8_t bpp);
		/* Put a pixel */
		int putPixel(uint16_t x, uint16_t y, pixel_t p);
		/* Put a rectangle (much faster than looped calls to putPixel) */
		int putRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, pixel_t p);
		/* Get the maximum dimensions for the screen */
		int getMax(uint16_t *x, uint16_t *y);
		/* Clean up (TODO: merge with destructor) */
		int wrapUp(bool clearScreen);
	private:
		fbInfo_t fb={nullptr, 0, 0, 0, 0};
		int putPixel_bgr(uint16_t x, uint16_t y, pixel_bgr_t p);
		int putRect_bgr(uint16_t x, uint16_t y, uint16_t width, uint16_t height, pixel_bgr_t p);
		/* Returns: */
		/*	00 if everything is good */
		/*	bit 1 is set if the framebuffer is not setup */
		/*	bit 2 is set if the padding for the pixel is non-zero (optional, support or zero) */
		/*	bit 3 is set if the size would put it off-screen (optional, truncate) */
		unsigned int checkParms(uint16_t maxRight, uint16_t maxDown, pixel_bgr_t p);
};

#endif /* _KERN_DRIVER_FB_H */
