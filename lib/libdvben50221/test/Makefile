
OBJ =	test-transport.o

CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC -I.. -I../../libdvbapi -I../../libdvbcfg

all: test-transport

test-transport: test-transport.o
	$(CC) -o test-transport test-transport.o -L../ -ldvben50221 -L../../libucsi -lucsi -L../../libdvbapi -ldvbapi -lpthread

clean:
	rm -f *.d *.o *.a *.so *~ *# test-transport

-include $(wildcard *.d) dummy

