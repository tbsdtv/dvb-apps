CFLAGS   = -g -O2 -Wall
CPPFLAGS = -MD -I../../include

dvbtraffic: dvbtraffic.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $< -o $@

clean:
	rm -f *.o *.d dvbtraffic
