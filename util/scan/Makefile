# Makefile for linuxtv.org dvb-apps/util/scan

CPPFLAGS += -I../../include
CFLAGS   ?= -g -O2 -Wall -W -Wshadow -Wpointer-arith -Wstrict-prototypes

objects  = atsc_psip_section.o \
           diseqc.o            \
           dump-vdr.o          \
           dump-zap.o          \
           lnb.o               \
           scan.o              \
           section.o

binaries = scan

ifeq ($(prefix),)
prefix = /usr
endif

.PHONY: all clean install

all:		$(binaries)

$(binaries):	$(objects)

$(objects):	atsc_psip_section.c atsc_psip_section.h

atsc_psip_section.c atsc_psip_section.h:
	perl section_generate.pl atsc_psip_section.pl

clean:
	rm -f atsc_psip_section.c atsc_psip_section.h $(objects) $(binaries)

install:
	mkdir -p $(prefix)/bin
	cp $(binaries) $(prefix)/bin
