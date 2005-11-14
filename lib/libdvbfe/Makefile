
OBJS =  dvbfe.o 

INCLUDES = dvbfe.h

CFLAGS ?= -fPIC -O2 -g -Wall -Wstrict-prototypes -Wshadow -Wpointer-arith
CPPFLAGS = -I. -I../../include
LDFLAGS = -shared -lc

STATICLIB = libdvbfe.a
SHAREDLIB = libdvbfe.so
RANLIB = ranlib
STRIP = strip
INSTDIR=/usr/local

all: symlink $(STATICLIB) $(SHAREDLIB)

symlink:
	@rm -f dvbfe
	@ln -s . dvbfe

$(STATICLIB): $(OBJS)
	$(AR) rc $(STATICLIB) $(OBJS)
	$(RANLIB) $(STATICLIB)

$(SHAREDLIB): $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(SHAREDLIB)

install: $(STATICLIB) $(SHAREDLIB) $(INCLUDES)
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/include/dvbfe
	cp -f $(STATICLIB) $(INSTDIR)/lib
	cp -f $(SHAREDLIB) $(INSTDIR)/lib
	$(STRIP) $(INSTDIR)/lib/$(SHAREDLIB)
	cp -f $(INCLUDES) $(INSTDIR)/include/dvbfe

clean:
	rm -f *~ *# *.o *.a *.so dvbfe

.PHONY: clean all
