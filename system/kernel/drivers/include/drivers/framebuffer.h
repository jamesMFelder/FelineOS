// SPDX-License-Identifier: MIT
// Copyright (c) 2021 James McNaughton Felder
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
	//For 32-bit color framebuffers
	[[deprecated("Can cause the pixel to be interpreted differently.")]] uint8_t fb_padding=0;
} pixel_bgr_t;

inline pixel_t bgr2rgb(pixel_bgr_t bgr){
	return {bgr.red, bgr.green, bgr.blue};
}

inline pixel_bgr_t rgb2bgr(pixel_t rgb){
	return {rgb.blue, rgb.green, rgb.red};
}

typedef struct fbInfo{
	pixel_bgr_t *addr; //The actual framebuffer;
	uint16_t width; //pixels per line
	uint16_t height; //line on the screen;
	uint16_t pitch; //bytes per line (account for extra padding)
	uint8_t bpp; //bits per pixel (32(good), 24(bad), 16(old))
} fbInfo_t;

#ifdef __cplusplus
class framebuffer{
	public:
		framebuffer();
		int init(pixel_bgr_t *addr, uint16_t width, uint16_t height,
				uint16_t pitch, uint8_t bpp);
		int putPixel(uint16_t x, uint16_t y, pixel_t p);
		int putRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, pixel_t p);
		int getMax(uint16_t *x, uint16_t *y);
		~framebuffer();
	private:
		fbInfo_t fb={0, 0, 0, 0, 0};
		bool ready=false;
		int putPixel_bgr(uint16_t x, uint16_t y, pixel_bgr_t p);
		int putRect_bgr(uint16_t x, uint16_t y, uint16_t width, uint16_t height, pixel_bgr_t p);
		int checkParms(uint16_t maxRight, uint16_t maxDown, pixel_bgr_t p);
};
#endif // __cplusplus

#endif // _KERN_DRIVER_FB_H
