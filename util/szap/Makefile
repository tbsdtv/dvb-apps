CC = gcc
CFLAGS = -Wall -g -O2
CPPFLAGS = -MD -I../../include -I../lib
LFLAGS = -Wall -g -O2
RM = rm -f

TARGETS = szap tzap czap femon
OBJS = szap.o tzap.o czap.o femon.o

all: $(OBJS) $(TARGETS)
	@echo
	@echo "--------------------------------------------------------------------------------"
	@echo " please copy an appropriate channels.conf-XXX channel list for DVB-S/C/T"
	@echo
	@echo "   to ~/.szap/channels.conf"
	@echo "      ~/.czap/channels.conf"
	@echo "      ~/.tzap/channels.conf"
	@echo
	@echo " and then call ./szap for DVB-S, ./czap for DVB-C or ./tzap for DVB-T"
	@echo "--------------------------------------------------------------------------------"
	@echo

szap: szap.o ../lib/lnb.o
	$(CC) $(LFLAGS) -o szap szap.o ../lib/lnb.o

.c.o:
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ -c $<

.o:
	$(CC) $(LFLAGS) -o $@ $<

clean:
	$(RM) $(TARGETS) core* *.o *.d .depend

-include $(wildcard *.d) dummy

