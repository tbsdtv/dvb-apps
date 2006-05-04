# Makefile for linuxtv.org dvb-apps/lib/libucsi

CPPFLAGS += -I..

includes = crc32.h            \
           descriptor.h       \
           endianops.h        \
           section.h          \
           section_buf.h      \
           transport_packet.h \
           types.h

objects  = crc32.o            \
           section_buf.o      \
           transport_packet.o

sharedlib = libucsi.so
staticlib = libucsi.a

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

all: $(sharedlib) $(staticlib)

include atsc/Makefile
include dvb/Makefile
include mpeg/Makefile

.PHONY: $(sub-install)

$(sharedlib): $(objects)

$(staticlib): $(objects)

clean::
	rm -f $(objects) $(sharedlib) $(staticlib)

install:	$(sub-install)
	mkdir -p $(prefix)/include/libucsi
	cp $(includes) $(prefix)/include/libucsi
	mkdir -p $(prefix)/lib
	cp $(sharedlib) $(staticlib) $(prefix)/lib

$(sub-install):
	$(MAKE) -C $@ sub-install-$@

include ../../Make.rules
