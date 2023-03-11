# SPDX-License-Identifier: MIT
# Copyright (c) 2023 James McNaughton Felder
DEFAULT_HOST!=../../default-host.sh
TARGET_HOST?=DEFAULT_HOST
HOSTARCH!=../../target-triplet-to-arch.sh $(TARGET_HOST)

CFLAGS?=-O2 -g
CPPFLAGS?=-fno-exceptions -fno-rtti
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
INCLUDEDIR?=$(PREFIX)/include
LIBDIR?=$(EXEC_PREFIX)/lib

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra -D__is_libc
CPPFLAGS:=$(CPPFLAGS)
LIBK_CFLAGS:=$(CFLAGS) -D__is_libk
LIBK_CPPFLAGS:=$(CPPFLAGS)

ARCHDIR=arch/$(HOSTARCH)

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(ARCH_CFLAGS)
CPPFLAGS:=$(CFLAGS) $(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBK_CFLAGS:=$(LIBK_CFLAGS) $(KERNEL_ARCH_CFLAGS)
LIBK_CPPFLAGS:=$(LIBK_CFLAGS) $(LIBK_CPPFLAGS) $(KERNEL_ARCH_CPPFLAGS)

FREEOBJS=\
$(ARCH_FREEOBJS) \
assert/assert.o \
ctype/isdigit.o \
ctype/iscntrl.o \
icxxabi/icxxabi.o \
stdio/printf.o \
stdio/putchar.o \
stdio/puts.o \
stdio/io_common.o \
stdlib/abort.o \
stdlib/abs.o \
string/memcmp.o \
string/memcpy.o \
string/memmove.o \
string/memset.o \
string/strlen.o \
string/strcat.o \
string/strcpy.o \
string/strcmp.o \
ssp/ssp.o

HOSTEDOBJS=\
$(ARCH_HOSTEDOBJS) \

OBJS=\
$(FREEOBJS) \
$(HOSTEDOBJS) \

LIBK_OBJS=$(FREEOBJS:.o=.libk.o)

#BINARIES=libc.a libk.a # Not ready for libc yet.
BINARIES=libk.a

.PHONY: all clean install install-headers install-libs
.SUFFIXES: .o .libk.o .c .S

all: $(BINARIES)

libc.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

libk.a: $(LIBK_OBJS)
	$(AR) rcs $@ $(LIBK_OBJS)

.c.o:
	$(CC) -MD -o $@ -std=gnu17 $(CFLAGS) -c $(abspath $<)

.cpp.o:
	$(CPP) -MD -o $@ -std=gnu++17 $(CPPFLAGS) -c $(abspath $<)

.c.S:
	$(CC) -MD -o $@ $(CFLAGS) -c $(abspath $<)

.cpp.S:
	$(CPP) -MD -o $@ $(CPPFLAGS) -c $(abspath $<)

.c.libk.o:
	$(CC) -MD -o $@ -std=gnu17 $(LIBK_CFLAGS) -c $(abspath $<)

.cpp.libk.o:
	$(CPP) -MD -o $@ -std=gnu++17 $(LIBK_CPPFLAGS) -c $(abspath $<)

.S.libk.o:
	$(CC) -MD -o $@  $(LIBK_CPPFLAGS) -c $(abspath $<)

clean:
	$(RM) $(BINARIES) *.a
	$(RM) $(OBJS) $(LIBK_OBJS) *.o */*.o */*/*.o
	$(RM) $(OBJS:.o=.d) $(LIBK_OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)

-include $(OBJS:.o=.d)
-include $(LIBK_OBJS:.o=.d)
