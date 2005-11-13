
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

INC = types.h \
      section.h \
      bat_section.h \
      dit_section.h \
      eit_section.h \
      int_section.h \
      nit_section.h \
      rst_section.h \
      sdt_section.h \
      sit_section.h \
      st_section.h \
      tdt_section.h \
      tot_section.h \
      descriptor.h \
      ac3_descriptor.h \
      ancillary_data_descriptor.h \
      announcement_support_descriptor.h \
      bouquet_name_descriptor.h \
      ca_identifier_descriptor.h \
      ca_system_descriptor.h \
      cable_delivery_descriptor.h \
      cell_frequency_link_descriptor.h \
      cell_list_descriptor.h \
      component_descriptor.h \
      content_descriptor.h \
      country_availability_descriptor.h \
      data_broadcast_descriptor.h \
      data_broadcast_id_descriptor.h \
      dsng_descriptor.h \
      extended_event_descriptor.h \
      frequency_list_descriptor.h \
      linkage_descriptor.h \
      local_time_offset_descriptor.h \
      mosaic_descriptor.h \
      multilingual_bouquet_name_descriptor.h \
      multilingual_component_descriptor.h \
      multilingual_network_name_descriptor.h \
      multilingual_service_name_descriptor.h \
      network_name_descriptor.h \
      nvod_reference_descriptor.h \
      parental_rating_descriptor.h \
      partial_transport_stream_descriptor.h \
      pdc_descriptor.h \
      private_data_specifier_descriptor.h \
      satellite_delivery_descriptor.h \
      service_descriptor.h \
      service_list_descriptor.h \
      service_move_descriptor.h \
      short_event_descriptor.h \
      short_smoothing_buffer_descriptor.h \
      stream_identifier_descriptor.h \
      stuffing_descriptor.h \
      subtitling_descriptor.h \
      telephone_descriptor.h \
      teletext_descriptor.h \
      terrestrial_delivery_descriptor.h \
      time_shifted_event_descriptor.h \
      time_shifted_service_descriptor.h \
      transport_stream_descriptor.h \
      vbi_data_descriptor.h \
      vbi_teletext_descriptor.h

TARGET = libdvb.a

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $(TARGET)

install: $(LIB) $(INC)
	mkdir -p $(INSTDIR)/include/ucsi/dvb
	cp -f $(INC) $(INSTDIR)/include/ucsi/dvb

clean:
	rm -f *.d *.o $(TARGET) test *~ *#

-include $(wildcard *.d) dummy

