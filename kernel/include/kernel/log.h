#ifndef _KERNEL_LOG_H
#define _KERNEL_LOG_H

#include <kernel/tty.h>

void klog(const char *data);
void kwarn(const char *data);
void kerror(const char *data);

#endif //_KERNEL_LOG_H
