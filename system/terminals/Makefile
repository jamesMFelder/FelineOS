# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
DEFAULT_HOST!=../../default-host.sh
HOST?=DEFAULT_HOST
HOSTARCH!=../../target-triplet-to-arch.sh $(HOST)

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
CPPFLAGS:=$(CFLAGS) $(CPPFLAGS) -Iinclude
LDFLAGS:=$(LDFLAGS)
LIBS:=$(LIBS) -nostdlib -lk -lgcc

VGA_TEXT_OBJS=\
common/terminal.o \
vga/vga_text.o

.PHONY: all clean install install-headers
.SUFFIXES: .o .c .S

all: $(BINARIES)

libVGA_text.a: $(VGA_TEXT_OBJS)
	$(AR) rcs $@ $(VGA_TEXT_OBJS)

.c.o:
	$(CC) -MD -c $< -o $@ -std=gnu11 $(CFLAGS)

.cpp.o:
	$(CPP) -MD -c $< -o $@ -std=gnu++11 $(CFLAGS) $(CPPFLAGS)

.S.o:
	$(CC) -MD -c $< -o $@ $(CFLAGS) $(CPPFLAGS)

.s.o:
	$(NASM) $(NASM_ARGS) $< -o $@

clean:
	rm -f libVGA_text.a
	rm -f $(OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)

-include $(OBJS:.o=.d)
