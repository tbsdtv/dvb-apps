
CFLAGS += -I../../../lib

test: dvbcfg_test.o ../libdvbcfg.a
	gcc -o dvbcfg_test dvbcfg_test.o ../libdvbcfg.a

clean:
	rm -f *~ *# *.o *.a dvbcfg_test

.PHONY: clean
