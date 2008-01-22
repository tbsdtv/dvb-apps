# Makefile for linuxtv.org dvb-apps

.PHONY: all clean install update

all clean install:
	$(MAKE) -C lib $@
	$(MAKE) -C test $@
	$(MAKE) -C util $@

update:
	@echo "Pulling changes & updating from master repository"
	hg pull -u
