
OBJ =	testucsi.o

CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC -I.. -I../../libdvbdemux

testucsi: testucsi.o
	$(CC) -static -o testucsi testucsi.o -L../ -lucsi -L../../libdvbdemux -ldvbdemux

clean:
	rm -f *.d *.o *.a *.so *~ *# testucsi

-include $(wildcard *.d) dummy

