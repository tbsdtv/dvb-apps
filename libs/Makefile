
.PHONY: libdvb2 libsi2

all: libdvb2 libsi2

libsi2:
	$(MAKE) -C libsi2 $(MAKECMDGOALS)

libdvb2: libsi2
	$(MAKE) -C libdvb2 $(MAKECMDGOALS)

%::
	$(MAKE) -C libsi2 $(MAKECMDGOALS)
	$(MAKE) -C libdvb2 $(MAKECMDGOALS)
