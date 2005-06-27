CC=gcc
CFLAGS= -g -Wall -W -I ../libdvbsi $(ARCH)
CPPFLAGS = -MD -I../libdvbsi -I. $(DEFS)
OBJS = en50221_hlci.o asn_1.o en50221_encode.o gimmick.o
RM=rm

all: $(OBJS) libs

$(OBJS): %.o : %.c
	$(CC) -c $(CPPFLAGS) $(CFLAGS) $< -o $@

libs:
	ar rcs libdvben50221.a en50221_hlci.o asn_1.o en50221_encode.o gimmick.o

clean:
	$(RM) *.a *.o *.d

-include $(wildcard *.d) dummy
