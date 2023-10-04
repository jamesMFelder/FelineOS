# SPDX-License-Identifier: MIT
# Copyright (c) 2023 James McNaughton Felder
DEFAULT_HOST!=../../default-host.sh
TARGET_HOST?=$(DEFAULT_HOST)

CFLAGS?=-O2 -g
CPPFLAGS?=-fno-exceptions -fno-rtti
LDFLAGS?=
LIBS?=

DESTDIR?=
PREFIX?=/usr/local
EXEC_PREFIX?=$(PREFIX)
INCLUDEDIR?=$(PREFIX)/include
LIBDIR?=$(EXEC_PREFIX)/lib

BINARIES=libfeline.a


CFLAGS:=$(CFLAGS) -ffreestanding -Wall -Wextra
CPPFLAGS:=$(CFLAGS) $(CPPFLAGS)
LIBFELINE_CFLAGS:=$(CFLAGS) -D__is_libfeline
LIBFELINE_CPPFLAGS:=$(CPPFLAGS) -D__is_libfeline
LIBS:=$(LIBS) -nostdlib -lgcc

FREEOBJS=\
src/kstring/kstring.o \
src/string/itostr.o \
src/locking/spinlock.o

HOSTEDOBJS=\

OBJS=\
$(FREEOBJS) \
$(HOSTEDOBJS) \

LIBFELINE_OBJS=$(FREEOBJS:.o=.libfeline.o)

.PHONY: all clean install install-headers install-libs
.SUFFIXES: .o .libfeline.o .c .S

all: $(BINARIES)

libfeline.a: $(OBJS)
	$(AR) rcs $@ $(OBJS)

.c.o:
	$(CC) -MD -o $@ -std=gnu20 $(CFLAGS) -c $(abspath $<)

.cpp.o:
	$(CPP) -MD -o $@ -std=gnu++20 $(CFLAGS) $(CPPFLAGS) -c $(abspath $<)

.c.S:
	$(CC) -MD -o $@ $(CFLAGS) -c $(abspath $<)

.cpp.S:
	$(CPP) -MD -o $@ $(CFLAGS) $(CPPFLAGS) -c $(abspath $<)

.c.libfeline.o:
	$(CC) -MD -o $@ -std=gnu20 $(LIBFELINE_CFLAGS) -c $(abspath $<)

.cpp.libfeline.o:
	$(CPP) -MD -o $@ -std=gnu++20 $(LIBFELINE_CFLAGS) $(LIBFELINE_CPPFLAGS) -c $(abspath $<)

.S.libfeline.o:
	$(CC) -MD -o $@ $(LIBFELINE_CFLAGS) $(LIBFELINE_CPPFLAGS) -c $(abspath $<)

clean:
	$(RM) $(BINARIES) *.a
	$(RM) $(OBJS) $(LIBFELINE_OBJS) *.o */*.o */*/*.o
	$(RM) $(OBJS:.o=.d) $(LIBFELINE_OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)

-include $(OBJS:.o=.d)
-include $(LIBFELINE_OBJS:.o=.d)
