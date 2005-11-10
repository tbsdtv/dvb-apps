
CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC
CPPFLAGS = -MD -I. -I.. $(DEFS)
LDFLAGS = -r

OBJ = cat_section.o \
      pat_section.o \
      pmt_section.o \
      tsdt_section.o \
      odsmt_section.o

INC = section.h \
      cat_section.h \
      odsmt_section.h \
      pat_section.h \
      pmt_section.h \
      tsdt_section.h \
      descriptor.h \
      mpeg4_audio_descriptor.h \
      mpeg4_video_descriptor.h \
      audio_stream_descriptor.h \
      ca_descriptor.h \
      copyright_descriptor.h \
      data_stream_alignment_descriptor.h \
      external_es_id_descriptor.h \
      fmc_descriptor.h \
      fmxbuffer_size_descriptor.h \
      hierarchy_descriptor.h \
      ibp_descriptor.h \
      iod_descriptor.h \
      iso_639_language_descriptor.h \
      maximum_bitrate_descriptor.h \
      multiplex_buffer_descriptor.h \
      multiplex_buffer_utilization_descriptor.h \
      muxcode_descriptor.h \
      private_data_indicator_descriptor.h \
      registration_descriptor.h \
      sl_descriptor.h \
      smoothing_buffer_descriptor.h \
      std_descriptor.h \
      system_clock_descriptor.h \
      target_background_grid_descriptor.h \
      video_stream_descriptor.h \
      video_window_descriptor.h

TARGET = libmpeg.a

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $(TARGET)

install: $(LIB) $(INC)
	mkdir -p $(INSTDIR)/include/ucsi/mpeg
	cp -f $(INC) $(INSTDIR)/include/ucsi/mpeg

clean:
	rm -f *.d *.o $(TARGET) test *~ *#

-include $(wildcard *.d) dummy

