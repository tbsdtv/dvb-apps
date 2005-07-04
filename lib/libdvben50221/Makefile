CC=gcc
CFLAGS= -g -Wall -W -I ../libdvbsi $(ARCH)
CPPFLAGS = -MD -I../libdvbsi -I. -I../../include $(DEFS)
OBJS = en50221_hlci.o asn_1.o en50221_encode.o gimmick.o

all: $(OBJS) libs

$(OBJS): %.o : %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

libs: $(OBJS)
	ar rcs libdvben50221.a en50221_hlci.o asn_1.o en50221_encode.o gimmick.o

clean:
	rm -f *.a *.o *.d

install:

-include $(wildcard *.d) dummy
