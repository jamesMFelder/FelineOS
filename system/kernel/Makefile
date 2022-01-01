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

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -isystem include -isystem drivers/include -isystem ../libc/include -isystem ../libFeline/include
CPPFLAGS:=$(CFLAGS) $(CPPFLAGS) -D__is_kernel -isystem ../libc/include/c++ -isystem ../libFeline/include/feline/c++
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -L$(SYSROOT)/usr/lib -Wl,-\( -lfeline -lk -Wl,-\) -lgcc

include kernel/make.config
include $(ARCHDIR)/make.config
include $(DRIVERDIR)/make.config

CFLAGS:=$(CFLAGS) $(KERNEL_ARCH_CFLAGS) $(KERNEL_DRIVER_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS) $(KERNEL_DRIVER_CPPFLAGS)
LDFLAGS:=$(LDFLAGS) $(KERNEL_ARCH_LDFLAGS) $(KERNEL_DRIVER_LDFLAGS)
LIBS:=$(LIBS) $(KERNEL_ARCH_LIBS) $(KERNEL_DRIVER_LIBS)

KERNEL_OBJS=\
$(KERNEL_ARCH_OBJS) \
$(KERNEL_DRIVER_OBJS) \
$(KERNEL_CORE_OBJS) \

OBJS=\
$(ARCHDIR)/boot/crti.o \
$(KERNEL_OBJS) \
$(ARCHDIR)/boot/crtn.o \

LINK_LIST=\
$(LDFLAGS) \
$(ARCHDIR)/boot/crti.o \
$(KERNEL_OBJS) \
$(LIBS) \
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
	$(CC) -MD -o $@ -std=gnu17 $(CFLAGS) -c $<

.cpp.o:
	$(CPP) -MD -o $@ -std=gnu++17 $(CPPFLAGS) -c $<

.S.o:
	$(CC) -MD -o $@ $(CPPFLAGS) -c $<

.s.o:
	$(NASM) $(NASM_ARGS) -o $@ $<

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
