
CC = gcc
CFLAGS ?= -g -Wall -O2
CFLAGS +=$(ARCH)
CPPFLAGS += -MD -I../../include $(DEFS)
LFLAGS = -g -Wall
DEFS = '-DDATADIR="$(datadir)"'
bindir = /usr/local/bin
datadir = /usr/local/share

OBJS = diseqc.o dump-zap.o dump-vdr.o scan.o section.o atsc_psip_section.o lnb.o
SRCS = $(OBJS:.o=.c)

DYNSRC = atsc_psip_section.c atsc_psip_section.h

TARGET = dvbscan
TARGETS = $(TARGET) atscscan

$(TARGET): $(DYNSRC) $(OBJS)
	$(CC) $(LFLAGS) -o $(TARGET) $(OBJS)
	cp dvbscan atscscan

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.d $(TARGETS) $(DYNSRC)

install: install-bin install-data
install-bin:
	test "$(bindir)/" != "/"
	mkdir -p $(DESTDIR)$(bindir)/
	for i in $(TARGETS) ; do cp $$i $(DESTDIR)$(bindir)/ ; done
install-data:
	test "$(datadir)/" != "/"
	mkdir -p $(DESTDIR)$(datadir)/dvb
	for d in dvb-* atsc ; do test -d $$d || continue ; echo ..... $$d ..... \
	; mkdir -p $(DESTDIR)$(datadir)/dvb/scan/$$d/ \
	; cp `find $$d -type f|grep -v CVS` $(DESTDIR)$(datadir)/dvb/scan/$$d/ ; done

-include $(wildcard *.d) dummy

atsc_psip_section.h: atsc_psip_section.c

atsc_psip_section.c: atsc_psip_section.pl section_generate.pl
	perl section_generate.pl atsc_psip_section.pl
