# Makefile for linuxtv.org dvb-apps/lib/libucsi

CPPFLAGS += -I../../lib
CFLAGS   += -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes -fPIC
LDFLAGS  += -shared

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

.DEFAULT_GOAL = all

include atsc/Makefile
include dvb/Makefile
include mpeg/Makefile

.PHONY: all clean install $(sub-install)

all:		$(sharedlib) $(staticlib)

$(sharedlib):	$(objects)
	$(CC) $(LDFLAGS) -o $(sharedlib) $(objects)

$(staticlib):	$(objects)
	$(AR) rcs $(staticlib) $(objects)

clean:
	rm -f $(objects) $(sharedlib) $(staticlib)

install:	$(sub-install)
	mkdir -p $(prefix)/include/libucsi
	cp $(includes) $(prefix)/include/libucsi
	mkdir -p $(prefix)/lib
	cp $(sharedlib) $(staticlib) $(prefix)/lib

$(sub-install):
	$(MAKE) -C $@ sub-install-$@
