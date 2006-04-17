# Makefile for linuxtv.org dvb-apps

.PHONY: all clean install

all clean install:
	$(MAKE) -C lib $@
	$(MAKE) -C util $@
	$(MAKE) -C test $@
