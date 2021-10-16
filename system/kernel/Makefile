# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
DEFAULT_HOST!=../../default-host.sh
HOST?=DEFAULT_HOST
HOSTARCH!=../../target-triplet-to-arch.sh $(HOST)

ARCHDIR=arch/$(HOSTARCH)
DRIVERDIR=drivers

CFLAGS?=-O2 -g
CPPFLAGS?=-fno-exceptions -fno-rtti
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
BOOTDIR?=$(EXEC_PREFIX)/boot
INCLUDEDIR?=$(PREFIX)/include

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CFLAGS) $(CPPFLAGS) -D__is_kernel -Iinclude -Idrivers/include
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -Wl,-\( -lVGA_text -lk -Wl,-\) -lgcc

include $(ARCHDIR)/make.config
include $(DRIVERDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS) $(KERNEL_DRIVER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS) $(KERNEL_DRIVER_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS) $(KERNEL_DRIVER_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS) $(KERNEL_DRIVER_LIBS)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
$(KERNEL_DRIVER_OBJS) \
kernel/kernel.o \
kernel/log.o \
kernel/cpp.o \

OBJS=\
$(ARCHDIR)/boot/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/boot/crtn.o \

LINK_LIST=\
$(LDFLAGS) \
$(ARCHDIR)/boot/crti.o \
$(ARCHDIR)/crtbegin.o \
$(KERNEL_OBJS) \
$(LIBS) \
$(ARCHDIR)/crtend.o \
$(ARCHDIR)/boot/crtn.o \

.PHONY: all clean install install-headers install-kernel
.SUFFIXES: .o .c .S

all: FelineOS.kernel

FelineOS.kernel: $(OBJS) $(ARCHDIR)/linker.ld
	$(CC) -T $(ARCHDIR)/linker.ld -o $@ $(CFLAGS) $(LINK_LIST)
	grub-file --is-x86-multiboot FelineOS.kernel

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

.cpp.o:
	$(CPP) -MD -c $< -o $@ -std=gnu++11 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.s.o:
	$(NASM) $(NASM_ARGS) $< -o $@

clean:
	rm -f FelineOS.kernel
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.
	cp -R --preserve=timestamps drivers/include/. $(DESTDIR)$(INCLUDEDIR)/.

install-kernel: FelineOS.kernel
	mkdir -p $(DESTDIR)$(BOOTDIR)
	cp FelineOS.kernel $(DESTDIR)$(BOOTDIR)

-include $(OBJS:.o=.d)
