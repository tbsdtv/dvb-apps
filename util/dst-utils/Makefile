CC = gcc
CFLAGS ?= -g -O2 -W -Wall
CFLAGS += $(ARCH)
CPPFLAGS += -MD -I../../lib/libdvben50221 -I../../include
LDFLAGS = -g -O2 -L ../../lib/libdvben50221 -ldvben50221
RM = rm -f

OBJS = dst_test.o
TARGETS = dst_test

all: $(OBJS) $(TARGETS)

dst_test: $(OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o dst_test dst_test.o $(LD_FLAGS)

clean:
	$(RM) $(TARGETS) *.o *.d dst_test

install:

-include $(wildcard *.d) dummy
