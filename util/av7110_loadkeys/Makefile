CC = gcc
CFLAGS = -g -Wall -O2 $(ARCH)
CPPFLAGS = $(DEFS)
DEFS = '-DDATADIR="$(datadir)"'
bindir  = /usr/local/bin
datadir = /usr/local/share

all: av7110_loadkeys evtest

av7110_loadkeys: av7110_loadkeys.o

evtest: evtest.o

av7110_loadkeys.o: av7110_loadkeys.c input_keynames.h

evtest.o: evtest.c input_keynames.h


input_keynames.h: /usr/include/linux/input.h input_fake.h Makefile
	@echo 'generate $@...'
	@echo '#ifndef __INPUT_KEYNAMES_H__' > $@
	@echo '#define __INPUT_KEYNAMES_H__' >> $@
	@echo '' >> $@
	@echo '#include <linux/input.h>' >> $@
	@echo '' >> $@
	@echo '#if !defined(KEY_OK)' >> $@
	@echo '#include "input_fake.h"' >> $@
	@echo '#endif' >> $@
	@echo '' >> $@
	@echo '' >> $@
	@echo 'struct input_key_name {' >> $@
	@echo '        const char *name;' >> $@
	@echo '        int         key;' >> $@
	@echo '};' >> $@
	@echo '' >> $@
	@echo '' >> $@
	@echo 'static struct input_key_name key_name [] = {' >> $@
	@for x in `cat /usr/include/linux/input.h input_fake.h | \
	          egrep '#define[ \t]+KEY_' | grep -v KEY_MAX | \
		  cut -f 1 | cut -f 2 -d ' ' | sort -u` ; do \
		echo "        { \"`echo $$x | cut -b 5-`\", $$x }," >> $@ \
		; \
	done
	@echo '};' >> $@
	@echo '' >> $@
	@echo 'static struct input_key_name btn_name [] = {' >> $@
	@for x in `cat /usr/include/linux/input.h input_fake.h | \
	          egrep '#define[ \t]+BTN_' | \
		  cut -f 1 | cut -f 2 -d ' ' | sort -u` ; do \
		echo "        { \"`echo $$x | cut -b 5-`\", $$x }," >> $@ \
		; \
	done
	@echo '};' >> $@
	@echo '' >> $@
	@echo '#endif  /* __INPUT_KEYNAMES_H */' >> $@
	@echo '' >> $@


clean:
	$(RM) core* *.o input_keynames.h av7110_loadkeys evtest

install: install-bin install-data
install-bin: av7110_loadkeys
	install -m 755 av7110_loadkeys $(DESTDIR)$(bindir)
install-data:
	mkdir -p $(DESTDIR)$(datadir)/dvb/av7110_loadkeys
	cp *.rc5 *.rcmm $(DESTDIR)$(datadir)/dvb/av7110_loadkeys/
