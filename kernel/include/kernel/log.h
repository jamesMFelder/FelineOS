#ifndef _KERNEL_LOG_H
#define _KERNEL_LOG_H

#include <kernel/tty.h>
#include <stdio.h>
#include <stdarg.h>

#define FELINE_CRIT 0
#define FELINE_ERR 1
#define FELINE_WARN 2
#define FELINE_LOG 3

//Basic logging functions
//TODO: should I make these functions if we can optomize having no % stuff
#define kerror(data) kerrorf("%s", data)
#define kwarn(data) kwarnf("%s", data)
#define klog(data) klogf("%s", data)
//ONLY USE FOR WHEN THE OS IS ABOUT TO CRASH
//TODO: Restrict access to kernel?
#define kcritical(data) kcriticalf("%s", data)

//printf()-style logging functions
void klogf(const char *format, ...);
void kwarnf(const char *format, ...);
void kerrorf(const char *format, ...);
//ONLY USE FOR WHEN THE OS IS ABOUT TO CRASH
//TODO: Restrict access to kernel?
void kcriticalf(const char *format, ...);

#endif //_KERNEL_LOG_H
