
OBJ =	crc32.o \
	section_buf.o \
	transport_packet.o

INC = 	endianops.h \
	crc32.h \
	descriptor.h \
	section.h \
	section_buf.h \
	transport_packet.h \
	convert.h

CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC
CPPFLAGS = -MD -I. $(DEFS)
LDFLAGS = -shared
RANLIB = ranlib

STATICLIB = libucsi.a
SHAREDLIB = libucsi.so
INSTDIR= /usr/local

.PHONY: symlink all mpeg dvb atsc test

all: symlink $(STATICLIB) $(SHAREDLIB) test

symlink:
	@rm -f ucsi
	@ln -s . ucsi
	
test: symlink $(SHAREDLIB) $(STATICLIB)
	$(MAKE) -C test

$(SHAREDLIB): $(OBJ) mpeg dvb # atsc
	$(LD) $(LDFLAGS) -o $(SHAREDLIB) $(OBJ) mpeg/libmpeg.a dvb/libdvb.a # atsc/libatsc.a

$(STATICLIB): $(OBJ) mpeg dvb # atsc
	$(AR) rc $(STATICLIB) $(OBJ) mpeg/*.o dvb/*.o # atsc/*.o
	$(RANLIB) $(STATICLIB)

install: $(SHAREDLIB) $(STATICLIB) $(INC)
	make -C mpeg install INSTDIR=$(INSTDIR)
	make -C dvb install INSTDIR=$(INSTDIR)
	make -C atsc install INSTDIR=$(INSTDIR)
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/include/ucsi
	cp -f $(STATICLIB) $(INSTDIR)/lib
	cp -f $(SHAREDLIB) $(INSTDIR)/lib
	strip $(INSTDIR)/lib/$(SHAREDLIB)
	cp -f $(INC) $(INSTDIR)/include/ucsi

mpeg:
	$(MAKE) -C mpeg

dvb:
	$(MAKE) -C dvb

atsc:
	$(MAKE) -C atsc

clean:
	$(MAKE) -C atsc clean
	$(MAKE) -C dvb clean
	$(MAKE) -C mpeg clean
	$(MAKE) -C test clean
	rm -f *.d *.o *.a *.so *~ *# ucsi

-include $(wildcard *.d) dummy

