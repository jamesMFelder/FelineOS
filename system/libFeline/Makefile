# SPDX-License-Identifier: MIT
# Copyright (c) 2021 James McNaughton Felder
DEFAULT_HOST!=../../default-host.sh
HOST?=$(DEFAULT_HOST)
HOSTARCH!=../../target-triplet-to-arch.sh $(HOST)

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

ARCHDIR=arch/$(HOSTARCH)

include $(ARCHDIR)/make.config

CFLAGS:=$(CFLAGS) $(ARCH_CFLAGS)
CPPFLAGS:=$(CPPFLAGS) $(ARCH_CPPFLAGS)
LIBFELINE_CFLAGS:=$(LIBFELINE_CFLAGS) $(LIB_FELINE_ARCH_CFLAGS)
LIBFELINE_CPPFLAGS:=$(LIBFELINE_CPPFLAGS) $(LIB_FELINE_ARCH_CPPFLAGS)

FREEOBJS=\
$(ARCH_FREEOBJS) \
src/string/itostr.o \
src/locking/spinlock.o

HOSTEDOBJS=\
$(ARCH_HOSTEDOBJS) \

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
	$(CC) -MD -o $@ -std=gnu17 $(CFLAGS) -c $<

.cpp.o:
	$(CPP) -MD -o $@ -std=gnu++17 $(CFLAGS) $(CPPFLAGS) -c $<

.c.S:
	$(CC) -MD -o $@ $(CFLAGS) -c $<

.cpp.S:
	$(CPP) -MD -o $@ $(CFLAGS) $(CPPFLAGS) -c $<

.c.libfeline.o:
	$(CC) -MD -o $@ -std=gnu17 $(LIBFELINE_CFLAGS) -c $<

.cpp.libfeline.o:
	$(CPP) -MD -o $@ -std=gnu++17 $(LIBFELINE_CFLAGS) $(LIBFELINE_CPPFLAGS) -c $<

.S.libfeline.o:
	$(CC) -MD -o $@ $(LIBFELINE_CFLAGS) $(LIBFELINE_CPPFLAGS) -c $<

clean:
	rm -f $(BINARIES) *.a
	rm -f $(OBJS) $(LIBFELINE_OBJS) *.o */*.o */*/*.o
	rm -f $(OBJS:.o=.d) $(LIBFELINE_OBJS:.o=.d) *.d */*.d */*/*.d

install: install-headers install-libs

install-headers:
	mkdir -p $(DESTDIR)$(INCLUDEDIR)
	cp -R --preserve=timestamps include/. $(DESTDIR)$(INCLUDEDIR)/.

install-libs: $(BINARIES)
	mkdir -p $(DESTDIR)$(LIBDIR)
	cp $(BINARIES) $(DESTDIR)$(LIBDIR)

-include $(OBJS:.o=.d)
-include $(LIBFELINE_OBJS:.o=.d)
