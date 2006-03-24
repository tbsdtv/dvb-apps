
OBJ =	test-transport.o

CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC -I.. -I../../libdvbapi -I../../libdvbcfg -I../../libucsi

all: test-transport test-session test-app

test-transport: test-transport.o
	$(CC) -o test-transport -static test-transport.o -L../ -ldvben50221 -L../../libucsi -lucsi -L../../libdvbapi -ldvbapi -lpthread

test-session: test-session.o
	$(CC) -o test-session -static test-session.o -L../ -ldvben50221 -L../../libucsi -lucsi -L../../libdvbapi -ldvbapi -lpthread

test-app: test-app.o
	$(CC) -o test-app -static test-app.o -L../ -ldvben50221 -L../../libucsi -lucsi -L../../libdvbapi -ldvbapi -lpthread

clean:
	rm -f *.d *.o *.a *.so *~ *# test-transport test-session test-app

-include $(wildcard *.d) dummy

