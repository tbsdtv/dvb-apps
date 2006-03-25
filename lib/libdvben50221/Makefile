CC=gcc

#DEBUG= -DDEBUG_RXDATA=1 -DDEBUG_TXDATA=1 -DLOG_LEVEL=5
DEBUG=-DLOG_LEVEL=1

CPPFLAGS += -g -Wall -W -MD $(DEBUG) \
		 	-I. \
			-I../libucsi \
			-I../libdvbmisc \
			-I../libdvbapi

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
		en50221_app_mmi.o \

LDFLAGS = -shared
RANLIB = ranlib

STATICLIB = libdvben50221.a
SHAREDLIB = libdvben50221.so

all: symlink $(STATICLIB) $(SHAREDLIB)

symlink:
	@rm -f dvben50221
	@ln -s . dvben50221

test: symlink $(STATICLIB) $(SHAREDLIB)
	$(MAKE) -C test

$(SHAREDLIB): $(OBJS)
	$(LD) $(LDFLAGS) -o $(SHAREDLIB) $(OBJS)

$(STATICLIB): $(OBJS)
	$(AR) rc $(STATICLIB) $(OBJS)
	$(RANLIB) $(STATICLIB)

clean:
	rm -f *.so *.a *.o *.d *~ *# dvben50221
	$(MAKE) -C test clean

install:

-include $(wildcard *.d) dummy
