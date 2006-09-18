# Makefile for linuxtv.org dvb-apps/util/scan

objects  = atsc_psip_section.o \
           diseqc.o            \
           dump-vdr.o          \
           dump-zap.o          \
           lnb.o               \
           scan.o              \
           section.o

binaries = scan

inst_bin = $(binaries)

removing = atsc_psip_section.c atsc_psip_section.h

CPPFLAGS += -I../../include -DDATADIR=\"$(prefix)/share\"

.PHONY: all

all: $(binaries)

$(binaries): $(objects)

$(objects): atsc_psip_section.c atsc_psip_section.h

atsc_psip_section.c atsc_psip_section.h:
	perl section_generate.pl atsc_psip_section.pl

include ../../Make.rules

install::
	@echo installing scan files
	@mkdir -p $(prefix)/share/dvb/atsc
	@mkdir -p $(prefix)/share/dvb/dvb-c
	@mkdir -p $(prefix)/share/dvb/dvb-s
	@mkdir -p $(prefix)/share/dvb/dvb-t
	@install -m 664 atsc/* $(prefix)/share/dvb/atsc
	@install -m 664 dvb-c/* $(prefix)/share/dvb/dvb-c
	@install -m 664 dvb-s/* $(prefix)/share/dvb/dvb-s
	@install -m 664 dvb-t/* $(prefix)/share/dvb/dvb-t
