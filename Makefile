# Makefile for linuxtv.org dvb-apps

%::
#	$(MAKE) -C libdvb2 $(MAKECMDGOALS)
	$(MAKE) -C util $(MAKECMDGOALS)
	$(MAKE) -C test $(MAKECMDGOALS)

FORCE:
