OBJS =  dvbdemux.o	\
	dvbfe.o		\
	dvbnet.o	\
	diseqc.o	\
	dvbca.o

INCLUDES = dvbdemux.h dvbfe.h dvbnet.h diseqc.h dvbca.h

CFLAGS ?= -fPIC -O2 -g -Wall -Wstrict-prototypes -Wshadow -Wpointer-arith
CPPFLAGS = 	-I../../include \
		-I../../lib

LDFLAGS = -shared -lc

STATICLIB = libdvbapi.a
SHAREDLIB = libdvbapi.so
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
	mkdir -p $(INSTDIR)/include/dvbapi
	cp -f $(STATICLIB) $(INSTDIR)/lib
	cp -f $(SHAREDLIB) $(INSTDIR)/lib
	$(STRIP) $(INSTDIR)/lib/$(SHAREDLIB)
	cp -f $(INCLUDES) $(INSTDIR)/include/dvbapi

clean:
	rm -f *~ *# *.o *.a *.so dvbapi

.PHONY: clean all
