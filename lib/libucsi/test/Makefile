
OBJ =	testucsi.o

CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC -I.. -I../../libdvbapi -I../../libdvbcfg

testucsi: testucsi.o
	$(CC) -static -o testucsi testucsi.o -L../ -lucsi -L../../libdvbapi -ldvbapi -L../../libdvbcfg -ldvbcfg

clean:
	rm -f *.d *.o *.a *.so *~ *# testucsi

-include $(wildcard *.d) dummy

