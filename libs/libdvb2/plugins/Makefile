
CC = gcc
LD = gcc

CFLAGS = -Wall -g -fPIC
CPPFLAGS = -MD -I../../../include -I. -I.. -I../../libsi2 $(DEFS)
LDFLAGS = -lc -L../ -ldvb2 -L../ci -lci2 -shared
PLUGINS = ci_linklayer.so ci_highlevel.so

INSTDIR = /usr/local

.PHONY: all

all: $(PLUGINS)

%.so: %.o
	$(LD) $(LDFLAGS) $< -o $@

install: $(PLUGINS)
	mkdir -p $(INSTDIR)/lib/dvb
	cp -f $(PLUGINS) $(INSTDIR)/lib/dvb

clean:
	rm -f *.d *.o *.so

-include $(wildcard *.d) dummy

