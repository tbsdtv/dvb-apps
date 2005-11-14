
OBJ =	testucsi.o

CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC -I.. -I../../libdvbdemux -I../../libdvbcfg -I../../libdvbfe

testucsi: testucsi.o
	$(CC) -static -o testucsi testucsi.o -L../ -lucsi -L../../libdvbdemux -ldvbdemux -L../../libdvbcfg -ldvbcfg -L../../libdvbfe -ldvbfe

clean:
	rm -f *.d *.o *.a *.so *~ *# testucsi

-include $(wildcard *.d) dummy

