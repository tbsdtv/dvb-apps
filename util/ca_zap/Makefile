CC = gcc
CFLAGS ?= -g -O2 -W -Wall
CFLAGS += $(ARCH)
CPPFLAGS += -MD \
	    -I../../lib/libdvben50221 \
	    -I../../lib/libdvbapi \
	    -I../../lib/libucsi \
	    -I../../lib/libdvbcfg

LDFLAGS =   -g -O2 \
	    -L ../../lib/libdvben50221 -ldvben50221 \
	    -L ../../lib/libdvbapi -ldvbapi \
	    -L ../../lib/libucsi -lucsi \
	    -L ../../lib/libdvbcfg -ldvbcfg

RM = rm -f

OBJS = ca_zap.o
TARGETS =ca_zap

all: $(OBJS) $(TARGETS)

ca_zap: $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o ca_zap ca_zap.o $(LDFLAGS)

clean:
	$(RM) $(TARGETS) *.o *.d ca_zap *~ *#

install:

-include $(wildcard *.d) dummy
