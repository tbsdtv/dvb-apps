CC = gcc
CFLAGS = -g -O2 -W -Wall -o $(ARCH)
CPPFLAGS = -MD -I../../lib/libdvbsi -I../../lib/libdvben50221
LDFLAGS = -g -O2 -L ../../lib/libdvbsi -ldvbsi -L ../../lib/libdvben50221 -ldvben50221
RM = rm -f

OBJS = dst_test.o
TARGETS = dst_test

all: $(OBJS) $(TARGETS)

dst_test:
	$(CC) $(CPPFLAGS) $(CFLAGS) dst_test dst_test.o $(LD_FLAGS)

clean:
	$(RM) $(TARGETS) *.o dst_test

-include $(wildcard *.d) dummy
