# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
DEFAULT_HOST!=../../default-host.sh
TARGET_HOST?=$(DEFAULT_HOST)
HOSTARCH!=../../target-triplet-to-arch.sh $(TARGET_HOST)

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
CPPFLAGS:=$(CFLAGS) $(CPPFLAGS) -D__is_kernel
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
	$(CPP) -T $(abspath $(ARCHDIR)/linker.ld) -o $@ $(CFLAGS) $(LINK_LIST)
ifeq ($(HOSTARCH), i386)
	grub2-file --is-x86-multiboot FelineOS.kernel
endif

$(ARCHDIR)/linker.ld: $(ARCHDIR)/linker.ldpp
	$(CPP) $(CFLAGS) $(CPPFLAGS) -CC -E -x c++ $(abspath $<) -o $@

$(ARCHDIR)/crtbegin.o $(ARCHDIR)/crtend.o:
	OBJ=`$(CC) $(CFLAGS) $(LDFLAGS) -print-file-name=$(@F)` && cp "$$OBJ" $@

.c.o:
	$(CC) -MD -o $@ -std=gnu17 $(CFLAGS) -c $(abspath $<)

.cpp.o:
	$(CPP) -MD -o $@ -std=gnu++17 $(CPPFLAGS) -c $(abspath $<)

.S.o:
	$(CC) -MD -o $@ $(CPPFLAGS) -c $(abspath $<)

clean:
	$(RM) FelineOS.kernel
	$(RM) $(ARCHDIR)/linker.ld
	$(RM) $(OBJS) *.o */*.o */*/*.o
	$(RM) $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-kernel

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.
	cp -R --preserve=timestamps drivers/include/. $(DESTDIR)$(INCLUDEDIR)/.

install-kernel: FelineOS.kernel
	mkdir -p $(DESTDIR)$(BOOTDIR)
ifeq ($(HOSTARCH), i386)
	cp FelineOS.kernel $(DESTDIR)$(BOOTDIR)
else ifeq ($(HOSTARCH), arm)
	objcopy -O binary FelineOS.kernel $(DESTDIR)$(BOOTDIR)/kernel.bin
	objcopy --only-keep-debug FelineOS.kernel $(DESTDIR)$(BOOTDIR)/kernel.sym
else
	@echo "Unknown architecture $(HOSTARCH)!"
	exit 1
endif

-include $(OBJS:.o=.d)
