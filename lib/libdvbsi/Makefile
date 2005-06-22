CC=gcc
CFLAGS= -g -Wall -O2 $(ARCH)
CPPFLAGS = -MD -I../../include -I../lib -I. $(DEFS)
RM=rm

OBJS = filter.o pat.o pmt.o si.o descriptor.o channels.o
all: $(OBJS) libs

$(OBJS): %.o : %.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

clean:
	$(RM) *.o *.d *.a

libs:
	ar rcs libdvbsi.a filter.o pat.o pmt.o si.o descriptor.o channels.o

-include $(wildcard *.d) dummy
