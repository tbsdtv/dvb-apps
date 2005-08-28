
OBJ =	demux.o \
	device.o \
	dvb.o \
	event.o \
	frontend.o \
	notifier.o \
	plugin.o \
	program.o \
	stream.o \
	thread.o

INC =	common.h \
	demux.h \
	device.h \
	dvb.h \
	event.h \
	frontend.h \
	internal.h \
	list.h \
	notifier.h \
	plugin.h \
	program.h \
	stream.h \
	thread.h

CC = gcc
LD = gcc

CFLAGS = -Wall -Wformat -Wpointer-arith -g -fPIC
CPPFLAGS = -MD -I../../include -I. -I../libsi2 $(DEFS)
LDFLAGS = -shared -lc -lpthread -lrt -ldl

TARGET = libdvb2.so
INSTDIR = /usr/local
DVBLIBS = -Lci -L../libsi2 -lci2 -lsi2
TESTLIBS = -L. -L../libsi2 -ldvb2 -lsi2

.PHONY: all ci plugins

all: symlink $(TARGET) ci plugins test

symlink:
	rm -f dvb
	ln -s . dvb

$(TARGET): $(OBJ) ci
	$(LD) $(LDFLAGS) $(OBJ) $(DVBLIBS) -o $(TARGET)

plugins:
	$(MAKE) -C plugins

ci:
	$(MAKE) -C ci

install: $(LIB) $(INC)
	$(MAKE) -C plugins install INSTDIR=$(INSTDIR)
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/include/dvb
	cp -f $(TARGET) $(INSTDIR)/lib
	cp -f $(INC) $(INSTDIR)/include/dvb

test: test.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TESTLIBS) test.c -o test

clean:
	$(MAKE) -C plugins clean
	$(MAKE) -C ci clean
	rm -f *.d *.o $(TARGET) test

-include $(wildcard *.d) dummy

