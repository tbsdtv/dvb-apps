
.PHONY: libdvbapi libdvbcfg libucsi libdvben50221 libdvbsi

all: libdvbapi libdvbcfg libucsi libdvben50221 libdvbsi

libdvbcfg:
	$(MAKE) -C libdvbcfg $(MAKECMDGOALS)

libdvbapi:
	$(MAKE) -C libdvbapi $(MAKECMDGOALS)

libdvben50221:
	$(MAKE) -C libdvben50221 $(MAKECMDGOALS)

libdvbsi:
	$(MAKE) -C libdvbsi $(MAKECMDGOALS)

libucsi:
	$(MAKE) -C libucsi $(MAKECMDGOALS)

%::
	$(MAKE) -C libdvbcfg $(MAKECMDGOALS)
	$(MAKE) -C libdvbapi $(MAKECMDGOALS)
	$(MAKE) -C libdvben50221 $(MAKECMDGOALS)
	$(MAKE) -C libdvbsi $(MAKECMDGOALS)
	$(MAKE) -C libucsi $(MAKECMDGOALS)
