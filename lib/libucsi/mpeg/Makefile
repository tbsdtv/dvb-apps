
CC = gcc
CFLAGS = -Wall -Wpointer-arith -g -fPIC
CPPFLAGS = -MD -I. -I.. $(DEFS)
LDFLAGS = -r

OBJ = cat_section.o \
      pat_section.o \
      pmt_section.o \
      tsdt_section.o \
      odsmt_section.o

INC = 	audio_stream_descriptor.h \
	ca_descriptor.h \
	cat_section.h \
	content_labelling_descriptor.h \
	copyright_descriptor.h \
	data_stream_alignment_descriptor.h \
	descriptor.h \
	external_es_id_descriptor.h \
	fmc_descriptor.h \
	fmxbuffer_size_descriptor.h \
	hierarchy_descriptor.h \
	ibp_descriptor.h \
	iod_descriptor.h \
	iso_639_language_descriptor.h \
	maximum_bitrate_descriptor.h \
	metadata_descriptor.h \
	metadata_pointer_descriptor.h \
	metadata_std_descriptor.h \
	mpeg4_audio_descriptor.h \
	mpeg4_video_descriptor.h \
	multiplex_buffer_descriptor.h \
	multiplex_buffer_utilization_descriptor.h \
	muxcode_descriptor.h \
	odsmt_section.h \
	pat_section.h \
	pmt_section.h \
	private_data_indicator_descriptor.h \
	registration_descriptor.h \
	section.h \
	sl_descriptor.h \
	smoothing_buffer_descriptor.h \
	std_descriptor.h \
	system_clock_descriptor.h \
	target_background_grid_descriptor.h \
	tsdt_section.h \
	types.h \
	video_stream_descriptor.h \
	video_window_descriptor.h

TARGET = libmpeg.a

.PHONY: all

all: $(TARGET)

$(TARGET): $(OBJ)
	$(LD) $(LDFLAGS) $(OBJ) -o $(TARGET)

install: $(LIB) $(INC)
	mkdir -p $(INSTDIR)/include/mpeg
	cp -f $(INC) $(INSTDIR)/include/mpeg

clean:
	rm -f *.d *.o $(TARGET) test *~ *#

-include $(wildcard *.d) dummy

