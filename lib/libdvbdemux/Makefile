
OBJS =  dvbdemux.o 

INCLUDES = dvbdemux.h

CFLAGS ?= -fPIC -O2 -g -Wall -Wstrict-prototypes -Wshadow -Wpointer-arith
CPPFLAGS = -I. -I../../include
LDFLAGS = -shared -lc

STATICLIB = libdvbdemux.a
SHAREDLIB = libdvbdemux.so
RANLIB = ranlib
STRIP = strip
INSTDIR=/usr/local

all: symlink $(STATICLIB) $(SHAREDLIB)

symlink:
	@rm -f dvbdemux
	@ln -s . dvbdemux
		
$(STATICLIB): $(OBJS)
	$(AR) rc $(STATICLIB) $(OBJS)
	$(RANLIB) $(STATICLIB)

$(SHAREDLIB): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(SHAREDLIB)

install: $(STATICLIB) $(SHAREDLIB) $(INCLUDES)
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/include/dvbdemux
	cp -f $(STATICLIB) $(INSTDIR)/lib
	cp -f $(SHAREDLIB) $(INSTDIR)/lib
	$(STRIP) $(INSTDIR)/lib/$(SHAREDLIB)
	cp -f $(INCLUDES) $(INSTDIR)/include/dvbdemux

clean:
	rm -f *~ *# *.o *.a *.so dvbdemux

.PHONY: clean all
