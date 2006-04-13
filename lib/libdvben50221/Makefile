CC=gcc

#DEBUG= -DDEBUG_RXDATA=1 -DDEBUG_TXDATA=1 -DLOG_LEVEL=5
DEBUG=-DLOG_LEVEL=1

CPPFLAGS += -fPIC -O2 -g -Wall -W -MD $(DEBUG) -I../../lib

OBJS = 	asn_1.o \
		en50221_transport.o \
		en50221_session.o \
		en50221_app_utils.o \
		en50221_app_rm.o \
		en50221_app_ai.o \
		en50221_app_dvb.o \
		en50221_app_datetime.o \
		en50221_app_lowspeed.o \
		en50221_app_auth.o \
		en50221_app_teletext.o \
		en50221_app_smartcard.o \
		en50221_app_epg.o \
		en50221_app_ca.o \
		en50221_app_mmi.o

INC =   asn_1.h \
		en50221_app_ai.h \
		en50221_app_auth.h \
		en50221_app_ca.h \
		en50221_app_datetime.h \
		en50221_app_dvb.h \
		en50221_app_epg.h \
		en50221_app_lowspeed.h \
		en50221_app_mmi.h \
		en50221_app_rm.h \
		en50221_app_smartcard.h \
		en50221_app_tags.h \
		en50221_app_teletext.h \
		en50221_app_utils.h \
		en50221_errno.h \
		en50221_session.h \
		en50221_transport.h

LDFLAGS = -shared
RANLIB = ranlib

STATICLIB = libdvben50221.a
SHAREDLIB = libdvben50221.so

all: $(STATICLIB) $(SHAREDLIB)

test: $(STATICLIB) $(SHAREDLIB)
	$(MAKE) -C test

$(SHAREDLIB): $(OBJS)
	$(LD) $(LDFLAGS) -o $(SHAREDLIB) $(OBJS)

$(STATICLIB): $(OBJS)
	$(AR) rc $(STATICLIB) $(OBJS)
	$(RANLIB) $(STATICLIB)

clean:
	rm -f *.so *.a *.o *.d *~ *# dvben50221
	$(MAKE) -C test clean

install: $(SHAREDLIB) $(STATICLIB) $(INC)
	mkdir -p $(INSTDIR)/lib
	mkdir -p $(INSTDIR)/include/dvben50221
	cp -f $(STATICLIB) $(INSTDIR)/lib
	cp -f $(SHAREDLIB) $(INSTDIR)/lib
	strip $(INSTDIR)/lib/$(SHAREDLIB)
	cp -f $(INC) $(INSTDIR)/include/dvben50221

-include $(wildcard *.d) dummy
