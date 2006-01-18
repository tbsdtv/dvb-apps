# Makefile for linuxtv.org dvb-apps

VERSION := 1.1.0
PACKAGE := linuxtv-dvb-apps-$(VERSION)
CVSROOT := $(shell test -r CVS/Root && cat CVS/Root)
RELEASE_TAG := LINUXTV-DVB-$(subst .,_,$(subst -,_,$(VERSION)))

all:

release dist:
	rm -rf release-tmp $(PACKAGE).tar.gz
	mkdir release-tmp
	( cd release-tmp; cvs -d$(CVSROOT) export -r$(RELEASE_TAG) -d$(PACKAGE) dvb-apps )
	find release-tmp -name .cvsignore | xargs rm -v
	( cd release-tmp; tar cjf ../$(PACKAGE).tar.bz2 $(PACKAGE) )
	rm -rf release-tmp
	@echo
	@echo --------------------------------------------------------------------------------
	@echo
	@echo "dist package: ./$(PACKAGE).tar.bz2"
	@echo
	@echo --------------------------------------------------------------------------------
	@echo

%::
	$(MAKE) -C lib/libdvbsi $(MAKECMDGOALS)
	$(MAKE) -C lib/libdvben50221 $(MAKECMDGOALS)
	$(MAKE) -C util $(MAKECMDGOALS)
	$(MAKE) -C test $(MAKECMDGOALS)
	$(MAKE) -C apps $(MAKECMDGOALS)
