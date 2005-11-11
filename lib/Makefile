
.PHONY: libdvbcfg libdvbdemux libdvben50221 libdvbfe libdvbnet libdvbsi libucsi

all: libdvbcfg libdvbdemux libdvben50221 libdvbfe libdvbnet libdvbsi libucsi

libdvbcfg:
	$(MAKE) -C libdvbcfg $(MAKECMDGOALS)

libdvbdemux:
	$(MAKE) -C libdvbdemux $(MAKECMDGOALS)

libdvben50221:
	$(MAKE) -C libdvben50221 $(MAKECMDGOALS)

libdvbfe:
	$(MAKE) -C libdvbfe $(MAKECMDGOALS)

libdvbnet:
	$(MAKE) -C libdvbnet $(MAKECMDGOALS)

libdvbsi:
	$(MAKE) -C libdvbsi $(MAKECMDGOALS)

libucsi:
	$(MAKE) -C libucsi $(MAKECMDGOALS)

%::
	$(MAKE) -C libdvbcfg $(MAKECMDGOALS)
	$(MAKE) -C libdvbdemux $(MAKECMDGOALS)
	$(MAKE) -C libdvben50221 $(MAKECMDGOALS)
	$(MAKE) -C libdvbfe $(MAKECMDGOALS)
	$(MAKE) -C libdvbnet $(MAKECMDGOALS)
	$(MAKE) -C libdvbsi $(MAKECMDGOALS)
	$(MAKE) -C libucsi $(MAKECMDGOALS)
