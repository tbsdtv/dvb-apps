CC = gcc
CFLAGS = -g -O2 -W -Wall -o $(ARCH)
CPPFLAGS = -MD -I../../lib/libdvbsi -I../../lib/libdvben50221
LDFLAGS = -g -O2 -L ../../lib/libdvbsi -ldvbsi -L ../../lib/libdvben50221 -ldvben50221
RM = rm -f

OBJS = ca_zap.o
TARGETS =ca_zap

all: $(OBJS) $(TARGETS)

ca_zap:
	$(CC) $(CPPFLAGS) $(CFLAGS) ca_zap ca_zap.o $(LDFLAGS)

clean:
	$(RM) $(TARGETS) *.o *.d ca_zap

-include $(wildcard *.d) dummy
