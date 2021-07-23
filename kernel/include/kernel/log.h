#ifndef __KERNEL_LOG_H
#define __KERNEL_LOG_H

#include <kernel/tty.h>

void klog(const char *data);
void kwarn(const char *data);
void kerror(const char *data);

#endif //__KERNEL_LOG_H
