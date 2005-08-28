
OBJS =  ca.o \
	ci.o \
	internal.o \
	application.o \
	session.o \
	transport.o

INCLUDES = 	ca.h \
		ci.h \
		ci_internal.h \
		ci_application.h \
		ci_session.h \
		ci_transport.h

CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC
CPPFLAGS = -MD -I../../../include -I. -I.. -I../../libsi2 $(DEFS)
LDFLAGS = -r

TARGET = libci2.a
INSTDIR= /usr/local
DVBLIBS=-lc -Wl,-rpath . -Wl,-rpath si2 -L. -Lsi2 -lsi2 -ldvb2

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)

install: $(LIB) $(INCLUDES)
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/include
	cp -f $(LIB) $(INSTDIR)/lib
	cp -f $(INCLUDES) $(INSTDIR)/include

clean:
	rm -f *.d *.o $(TARGET)

-include $(wildcard *.d) dummy

