
OBJ =	crc32.o \
	descriptor.o \
	section.o \

INC = 	common.h \
	crc32.h \
	descriptor.h \
	section.h \

CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC
CPPFLAGS = -MD -I. $(DEFS)
LDFLAGS = -r -Lmpeg -Ldvb -Latsc -lmpeg -ldvb # -latsc

TARGET = libsi2.a
INSTDIR= /usr/local

.PHONY: symlink all mpeg dvb atsc

all: symlink $(TARGET)

symlink:
	@rm -f si
	@ln -s . si

$(TARGET): $(OBJ) mpeg dvb # atsc
	$(LD) $(LDFLAGS) $(OBJ) -o $(TARGET)

install: $(LIB) $(INC)
	#$(MAKE) INSTDIR=$(INSTDIR) -C atsc install
	$(MAKE) INSTDIR=$(INSTDIR) -C dvb install
	$(MAKE) INSTDIR=$(INSTDIR) -C mpeg install
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/include/dvb/si
	cp -f $(TARGET) $(INSTDIR)/lib
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
	rm -f *.d *.o $(TARGET) test

-include $(wildcard *.d) dummy

