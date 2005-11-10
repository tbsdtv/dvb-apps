
OBJS =  dvbnet.o 

INCLUDES = dvbnet.h

CFLAGS ?= -fPIC -O2 -g -Wall -Wstrict-prototypes -Wshadow -Wpointer-arith
CPPFLAGS = -I. -I../../include
LDFLAGS = -shared -lc

STATICLIB = libdvbnet.a
SHAREDLIB = libdvbnet.so
RANLIB = ranlib
STRIP = strip
INSTDIR=/usr/local

all: $(STATICLIB) $(SHAREDLIB)

$(STATICLIB): $(OBJS)
	$(AR) rc $(STATICLIB) $(OBJS)
	$(RANLIB) $(STATICLIB)

$(SHAREDLIB): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(SHAREDLIB)

install: $(STATICLIB) $(SHAREDLIB) $(INCLUDES)
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/include
	cp -f $(STATICLIB) $(INSTDIR)/lib
	cp -f $(SHAREDLIB) $(INSTDIR)/lib
	$(STRIP) $(INSTDIR)/lib/$(SHAREDLIB)
	cp -f $(INCLUDES) $(INSTDIR)/include

clean:
	rm -f *~ *# *.o *.a *.so

.PHONY: clean all
