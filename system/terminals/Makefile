# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
DEFAULT_HOST!=../../default-host.sh
TARGET_HOST?=DEFAULT_HOST
HOSTARCH!=../../target-triplet-to-arch.sh $(TARGET_HOST)

CFLAGS?=-O2 -g
CPPFLAGS?=-fno-exceptions -fno-rtti
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXECPREFIX?=$(PREFIX)
INCLUDEDIR?=$(PREFIX)/include
LIBDIR=$(EXECPREFIX)/lib
BINARIES=libVGA_text.a

CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CFLAGS) $(CPPFLAGS)
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk -lgcc

VGA_TEXT_OBJS=\
common/terminal.o \
vga/vga_text.o \

.PHONY: all clean install install-headers
.SUFFIXES: .o .c .S

all: $(BINARIES)

libVGA_text.a: $(VGA_TEXT_OBJS)
	$(AR) rcs $@ $(VGA_TEXT_OBJS)

.c.o:
	$(CC) -MD -o $@ -std=gnu17 $(CFLAGS) -c $(abspath $<)

.cpp.o:
	$(CPP) -MD -o $@ -std=gnu++17 $(CPPFLAGS) -c $(abspath $<)

.S.o:
	$(CC) -MD -o $@ $(CPPFLAGS) -c $(abspath $<)

clean:
	$(RM) $(BINARIES)
	$(RM) $(OBJS) *.o */*.o */*/*.o
	$(RM) $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)

-include $(OBJS:.o=.d)
