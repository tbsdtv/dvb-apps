
OBJ =	test-transport.o

CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC -I.. -I../../libdvbapi -I../../libdvbcfg

all: test-transport test-session

test-transport: test-transport.o
	$(CC) -o test-transport -static test-transport.o -L../ -ldvben50221 -L../../libucsi -lucsi -L../../libdvbapi -ldvbapi -lpthread

test-session: test-session.o
	$(CC) -o test-session -static test-session.o -L../ -ldvben50221 -L../../libucsi -lucsi -L../../libdvbapi -ldvbapi -lpthread

clean:
	rm -f *.d *.o *.a *.so *~ *# test-transport test-session

-include $(wildcard *.d) dummy

