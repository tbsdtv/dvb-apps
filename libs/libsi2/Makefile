
OBJ =	crc32.o \
	descriptor.o \
	section.o \
	section_buf.o

INC = 	common.h \
	crc32.h \
	descriptor.h \
	section.h \
	section_buf.h

CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC
CPPFLAGS = -MD -I. $(DEFS)
LDFLAGS = -shared
RANLIB = ranlib

STATICLIB = libsi2.a
SHAREDLIB = libsi2.so
INSTDIR= /usr/local

.PHONY: symlink all mpeg dvb atsc

all: symlink $(STATICLIB) $(SHAREDLIB)

symlink:
	@rm -f si
	@ln -s . si

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
	mkdir -p $(INSTDIR)/include/dvb/si
	cp -f $(STATICLIB) $(INSTDIR)/lib
	cp -f $(SHAREDLIB) $(INSTDIR)/lib
	strip $(INSTDIR)/lib/$(SHAREDLIB)
	cp -f $(INC) $(INSTDIR)/include/dvb/si

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
	rm -f *.d *.o *.a *.so *~ *# si test

-include $(wildcard *.d) dummy

