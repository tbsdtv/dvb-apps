
CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC
CPPFLAGS = -MD -I. -I.. $(DEFS)
LDFLAGS = -r

OBJ = bat_section.o \
      eit_section.o \
      dit_section.o \
      int_section.o \
      nit_section.o \
      rst_section.o \
      sdt_section.o \
      sit_section.o \
      st_section.o \
      tdt_section.o \
      tot_section.o \
      types.o

INC = 	ac3_descriptor.h \
	adaptation_field_data_descriptor.h \
	ait_application_descriptor.h \
	ait_application_icons_descriptor.h \
	ait_application_name_descriptor.h \
	ait_external_application_authorisation_descriptor.h \
	ancillary_data_descriptor.h \
	announcement_support_descriptor.h \
	application_signalling_descriptor.h \
	bat_section.h \
	bouquet_name_descriptor.h \
	cable_delivery_descriptor.h \
	ca_identifier_descriptor.h \
	cell_frequency_link_descriptor.h \
	cell_list_descriptor.h \
	component_descriptor.h \
	content_descriptor.h \
	content_identifier_descriptor.h \
	country_availability_descriptor.h \
	data_broadcast_descriptor.h \
	data_broadcast_id_descriptor.h \
	default_authority_descriptor.h \
	descriptor.h \
	dit_section.h \
	dsng_descriptor.h \
	eit_section.h \
	extended_event_descriptor.h \
	frequency_list_descriptor.h \
	int_section.h \
	linkage_descriptor.h \
	local_time_offset_descriptor.h \
	mhp_data_broadcast_id_descriptor.h \
	mosaic_descriptor.h \
	multilingual_bouquet_name_descriptor.h \
	multilingual_component_descriptor.h \
	multilingual_network_name_descriptor.h \
	multilingual_service_name_descriptor.h \
	network_name_descriptor.h \
	nit_section.h \
	nvod_reference_descriptor.h \
	parental_rating_descriptor.h \
	partial_transport_stream_descriptor.h \
	pdc_descriptor.h \
	private_data_specifier_descriptor.h \
	related_content_descriptor.h \
	rnt_rar_over_dvb_stream_descriptor.h \
	rnt_rar_over_ip_descriptor.h \
	rnt_rnt_scan_descriptor.h \
	rst_section.h \
	satellite_delivery_descriptor.h \
	scrambling_descriptor.h \
	sdt_section.h \
	section.h \
	service_availability_descriptor.h \
	service_descriptor.h \
	service_identifier_descriptor.h \
	service_list_descriptor.h \
	service_move_descriptor.h \
	short_event_descriptor.h \
	short_smoothing_buffer_descriptor.h \
	sit_section.h \
	stream_identifier_descriptor.h \
	st_section.h \
	stuffing_descriptor.h \
	subtitling_descriptor.h \
	tdt_section.h \
	telephone_descriptor.h \
	teletext_descriptor.h \
	terrestrial_delivery_descriptor.h \
	time_shifted_event_descriptor.h \
	time_shifted_service_descriptor.h \
	tot_section.h \
	transport_stream_descriptor.h \
	tva_id_descriptor.h \
	types.h \
	vbi_data_descriptor.h \
	vbi_teletext_descriptor.h

TARGET = libdvb.a

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $(TARGET)

install: $(LIB) $(INC)
	mkdir -p $(INSTDIR)/include/dvb
	cp -f $(INC) $(INSTDIR)/include/dvb

clean:
	rm -f *.d *.o $(TARGET) test *~ *#

-include $(wildcard *.d) dummy

