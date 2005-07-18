
.PHONY: libdvb2 libsi2 libdvbcfg

all: libdvb2 libsi2 libdvbcfg

libsi2:
	$(MAKE) -C libsi2 $(MAKECMDGOALS)

libdvb2: libsi2
	$(MAKE) -C libdvb2 $(MAKECMDGOALS)

libdvbcfg:
	$(MAKE) -C libdvbcfg $(MAKECMDGOALS)

%::
	$(MAKE) -C libsi2 $(MAKECMDGOALS)
	$(MAKE) -C libdvb2 $(MAKECMDGOALS)
	$(MAKE) -C libdvbcfg $(MAKECMDGOALS)
