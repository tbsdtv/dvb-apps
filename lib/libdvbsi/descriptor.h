/*
	A SI parser implementation for libdvb
	an implementation for the High Level Common Interface

	Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as
	published by the Free Software Foundation; either version 2.1 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#ifndef __DESCRIPTOR_H__
#define __DESCRIPTOR_H__

#include <stdlib.h>
#include <stdint.h>
#include "pmt.h"

struct iso_639_language_code {
	unsigned iso_639_language_code: 24;
	unsigned audio_type: 8;
};

struct flex_mux {
	unsigned es_id: 16;
	unsigned flex_mux_channel: 8;
};

struct muxcode_table_entry {
/*	described in ISO/IEC 14496-1	*/
};

struct service_list {
	unsigned service_id: 16;
	unsigned service_type: 8;
};

struct country_loop {
	unsigned country_code: 24;
};

struct nvod_reference {
	unsigned transport_stream_id: 16;
	unsigned original_network_id: 16;
	unsigned service_id: 16;
};

struct items {
	unsigned item_description_length: 8;
	uint8_t *p_item_description;

	unsigned item_length: 8;
	uint8_t *p_item_char;
};

struct elementary_cell_field {
	unsigned reserved: 2;
	unsigned elementary_cell_id: 6;
};

struct mosaic_info {
	unsigned logical_cell_id: 6;
	unsigned reserved: 7;
	unsigned logical_cell_presentation_info: 3;

	unsigned elementary_cell_field_length: 8;
	struct elementary_cell_field *p_elementary_cell_field;

	unsigned cell_linkage_info: 8;
	unsigned bouquet_id: 16;

	unsigned original_network_id: 16;
	unsigned transport_stream_id: 16;
	unsigned service_id: 16;
	unsigned event_id: 16;

};

struct content {
	unsigned content_nibble_level_1: 4;
	unsigned content_nibble_level_2: 4;
	unsigned user_nibble_1: 4;
	unsigned user_nibble_2: 4;
};

struct parental_rating {
	unsigned country_code: 24;
	unsigned rating: 8;
};

struct teletext {
	unsigned iso_639_language_code: 24;
	unsigned teletext_type: 5;
	unsigned teletext_magazine_number: 3;
	unsigned teletext_page_number: 8;
};

struct local_time_offset {
	unsigned country_code: 24;
	unsigned country_region_id: 6;
	unsigned reserved: 1;
	unsigned local_time_offset_polarity: 1;
	unsigned local_time_offset: 16;
	unsigned long long time_of_change;
	unsigned next_time_offset: 16;
};

struct subtitle {
	unsigned iso_639_language_code: 24;
	unsigned subtitling_type: 8;
	unsigned composition_page_id: 16;
	unsigned ancillary_page_id: 16;
};

struct multilingual_network_name {
	unsigned iso_639_language_code: 24;
	unsigned network_name_length: 8;
	uint8_t *p_char;
};

struct multilingual_bouquet_name {
	unsigned iso_639_language_code: 24;
	unsigned bouquet_name_length: 8;
	uint8_t *p_char;
};

struct multilingual_service_name {
	unsigned iso_639_language_code: 24;

	unsigned service_provider_name_length: 8;
	uint8_t *p_service_provider_name;

	unsigned service_name_length: 8;
	uint8_t *p_service_name;
};

struct multilingual_component {
	unsigned iso_639_language_code: 24;
	unsigned text_description_length: 8;
	uint8_t *p_text_char;
};

struct descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	union {
		// tag = 0x02
		struct {
	unsigned multiple_frame_rate_flag: 1;
	unsigned frame_rate_code: 4;
	unsigned mpeg_1_only_flag: 1;
	unsigned constrained_parameter_flag: 1;
	unsigned still_picture_flag: 1;
	unsigned profile_and_level_indication: 8;
	unsigned chroma_format: 2;
	unsigned frame_rate_extension_flag: 1;
	unsigned reserved: 5;
		} video_stream;
		// tag = 0x03
		struct {
	unsigned free_format_flag: 1;
	unsigned id: 1;
	unsigned layer: 2;
	unsigned variable_rate_audio_indicator: 1;
	unsigned reserved: 3;
		} audio_stream;
		// tag = 0x04
		struct {
	unsigned reserved_1: 4;
	unsigned hierarchy_type: 4;
	unsigned reserved_2: 2;
	unsigned hierarchy_layer_index: 6;
	unsigned reserved_3: 2;
	unsigned hierarchy_embedded_layer_index: 6;
	unsigned reserved_4: 2;
	unsigned hierarchy_channel: 6;
		} hierarchy;
		// tag = 0x05
		struct {
	unsigned format_identifier: 32;
	uint8_t *p_additional_id_info;
		} registration;
		// tag = 0x06
		struct {
	unsigned alignment_type: 8;
		} data_stream_alignment;
		// tag = 0x07
		struct {
	unsigned horizontal_size: 14;
	unsigned vertical_size: 14;
	unsigned aspect_ratio_information: 4;
		} target_background_grid;
		// tag = 0x08
		struct {
	unsigned horizontal_offset: 14;
	unsigned vertical_offset: 14;
	unsigned window_priority: 4;
		} video_window;
		// tag = 0x09
		struct {
	unsigned ca_system_id: 16;
	unsigned reserved: 3;
	unsigned ca_pid: 13;
	uint8_t *p_private_data_byte;
		} ca;
		// tag = 0x0a
		struct {
	struct iso_639_language_code *p_iso_639_language_code;
		} iso_639_language;
		// tag = 0x0b
		struct {
	unsigned external_clock_reference_indicator: 1;
	unsigned reserved_1: 1;
	unsigned clock_accuracy_integer: 6;
	unsigned clock_accuracy_exponent: 3;
	unsigned reserved_2: 5;
		} system_clock;
		// tag = 0x0c
		struct {
	unsigned bound_valid_flag: 1;
	unsigned ltw_offset_lower_bound: 15;
	unsigned reserved: 1;
	unsigned ltw_offset_upper_bound: 14;
		} multiplex_buffer_utilization;
		// tag = 0x0d
		struct {
	unsigned copyright_identifier: 32;
	uint8_t *p_additional_copyright_info;
		} copyright;
		// tag = 0x0e
		struct {
	unsigned reserved: 2;
	unsigned maximum_bitrate: 22;
		} maximum_bitrate;
		// tag = 0x0f
		struct {
	unsigned private_data_indicator: 32;
		} private_data_indicator;
		// tag = 0x10
		struct {
	unsigned reserved_1: 2;
	unsigned sb_leak_rate: 22;
	unsigned reserved_2: 2;
	unsigned sb_size: 22;
		} smoothing_buffer;
		// tag = 0x11
		struct {
	unsigned reserved: 7;
	unsigned leak_valid_flag: 1;
		} std;
		// tag = 0x12
		struct {
	unsigned closed_gop_flag: 1;
	unsigned identical_gop_flag: 1;
	unsigned max_gop_length: 14;
		} ibp;
		// tag = 0x1b
		struct {
	unsigned mpeg_4_visual_profile_and_level: 8;
		} mpeg_4_video;
		// tag = 0x1c
		struct {
	unsigned mpeg_4_audio_profile_and_level: 8;
		} mpeg_4_audio;
		// tag = 0x1d
		struct {
	unsigned scope_of_iod_label: 8;
	unsigned iod_label: 8;
	struct initial_object_descriptor *p_initial_object_descriptor;
		} iod;
		// tag = 0x1e
		struct {
	unsigned es_id: 16;
		} sl;
		// tag = 0x1f
		struct {
	struct flex_mux *p_flex_mux;
		} fmc;
		// tag = 0x20
		struct {
	unsigned external_es_id: 16;
		} external_es_id;
		// tag = 0x21
		struct {
	struct muxcode_table_entry *p_muxcode_table_entry;
		} muxcode;
		// tag = 0x22
		struct {
	struct default_flexmux_buffer_descriptor *p_default_flexmux_buffer_descriptor;
		} fmxbuffer_size;
		// tag = 0x23
		struct {
	unsigned mb_buffer_size: 24;
			unsigned tb_leak_rate: 24;
		} multiplex_buffer;
		/*	Some new descriptors (EN 300 468)	*/
		// tag = 0x40
		struct {
			uint8_t *p_network_name;
		} network_name;
		// tag = 0x41
		struct {
	struct service_list *p_service_list;
		} service_list;
		// tag = 0x42
		struct {
	uint8_t *p_stuffing_bytes;
		} stuffing;
		// tag = 0x43
		struct {
	unsigned frequency: 32;
	unsigned orbital_position: 16;
	unsigned west_east_flag: 1;
	unsigned polarization: 2;
	unsigned modulation: 5;
	unsigned symbol_rate: 28;
	unsigned fec_inner: 4;
		} satellite_delivery_system;
		// tag = 0x44
		struct {
	unsigned frequency: 32;
	unsigned reserved: 12;
	unsigned fec_outer: 4;
	unsigned modulation: 8;
	unsigned symbol_rate: 28;
	unsigned fec_inner: 4;
		} cable_delivery_system;
		// tag = 0x47
		struct {
	uint8_t *p_bouquet_name;
		} bouquet_name;
		// tag = 0x48
		struct {
	unsigned service_type: 8;
	unsigned service_provider_name_length: 8;
	uint8_t *p_service_provider_name;
	unsigned service_name_length: 8;
	uint8_t *p_service_name;
		} service;
		// tag = 0x49
		struct {
	unsigned country_availability_flag: 1;
	unsigned reserved: 7;
	struct country_loop *p_country_loop;
		} country_availability;
		// tag = 0x4a
		struct {
	unsigned transport_stream_id: 16;
	unsigned original_network_id: 16;
	unsigned service_id: 16;
	unsigned linkage_type: 8;
	uint8_t *p_private_data;
		} linkage;
		// tag = 0x4b
		struct {
	struct nvod_reference *p_nvod_reference;
		} nvod_reference;
		// tag = 0x4c
		struct {
	unsigned reference_service_id: 16;
		} time_shifted_service;
		// tag = 0x4d
		struct {
	unsigned iso_639_language_code: 24;
	unsigned event_name_length: 8;
	uint8_t *p_event_name;
	unsigned text_length: 8;
	uint8_t *p_text_char;
		} short_event;
		// tag = 0x4e
		struct {
	unsigned descriptor_number: 4;
	unsigned last_descriptor_number: 4;
	unsigned iso_639_language_code: 24;
	unsigned length_of_items: 8;
	struct items *p_items;
	unsigned text_length: 8;
	uint8_t *p_text_char;
		} extended_event;
		// tag = 0x4f
		struct {
	unsigned reference_service_id: 16;
	unsigned reference_event_id: 16;
		} time_shifted_event;
		// tag = 0x50
		struct {
	unsigned reserved: 4;
	unsigned stream_content: 4;
	unsigned component_type: 8;
	unsigned component_tag: 8;
	unsigned iso_639_language_code: 24;
	uint8_t *p_text_char;
		} component;
		// tag = 0x51
		struct {
	unsigned mosaic_entry_point: 1;
	unsigned number_of_horiz_elementary_cells: 3;
	unsigned reserved: 1;
	unsigned number_of_vert_elementary_cells: 3;
	struct mosaic_info *p_mosaic_info;
		} mosaic;
		// tag = 0x52
		struct {
	unsigned component_tag: 8;
		} stream_identifier;
		// tag = 0x53
		struct {
	uint16_t *p_ca_identifier;
		} ca_identifier;
		// tag = 0x54
		struct {
	struct content *p_content;
		} content;
		// tag = 0x55
		struct {
	struct parental_rating *p_parental_rating;
		} parental_rating;
		// tag = 0x56
		struct {
	struct teletext *p_teletext;
		} teletext;
		// tag = 0x57
		struct {
	unsigned reserved_1: 2;
	unsigned foreign_availability: 1;
	unsigned connection_type: 5;
	unsigned reserved_2: 1;
	unsigned country_prefix_length: 2;
	unsigned international_area_code_length: 3;
	unsigned operator_code_length: 2;
	unsigned reserved_3: 1;
	unsigned national_area_code_length: 3;
	unsigned core_number_length: 4;
	uint8_t *p_country_prefix;
	uint8_t *p_international_area_code;
	uint8_t *p_operator_code;
	uint8_t *p_national_area_code;
	uint8_t *p_core_number;
		} telephone;
		// tag = 0x58
		struct {
	struct local_time_offset *p_local_time_offset;
		} local_time_offset;
		// tag = 0x59
		struct {
	struct subtitle *p_subtitle;
		} subtitling;
		// tag = 0x5a
		struct {
	unsigned centre_frequency: 32;
	unsigned bandwidth: 3;
	unsigned reserved_1: 5;
	unsigned constellation: 2;
	unsigned hierarchy_information: 3;
	unsigned code_rate_hp_stream: 3;
	unsigned code_rate_lp_stream: 3;
	unsigned guard_interval: 2;
	unsigned transmission_mode: 2;
	unsigned other_frequency_flag: 2;
	unsigned reserved_2: 32;
		} terrestrial_delivery_system;
		// tag = 0x5b
		struct {
	struct multilingual_network_name *p_multilingual_network_name;
		} multilingual_network_name;
		// tag = 0x5c
		struct {
	struct multilingual_bouquet_name *p_multilingual_bouquet_name;
		} multilingual_bouquet_name;
		// tag = 0x5d
		struct {
	struct multilingual_service_name *p_multilingual_service_name;
		} multilingual_service_name;
		// tag = 0x5e
		struct {
	unsigned component_tag: 8;
	struct multilingual_component *p_mulyilingual_component;
		} multilingual_component;
		// tag = 0x5f
		struct {
	unsigned private_data_specifier: 32;
		} private_data_specifier;
		// tag = 0x60
		struct {
	unsigned new_original_network_id: 16;
	unsigned new_transport_stream_id: 16;
	unsigned new_service_id: 16;
		} service_move;
		// tag = 0x61
		struct {
	unsigned sb_size: 2;
	unsigned sb_leak_rate: 6;
	uint8_t *p_dvb_reserved;
		} short_smoothing_buffer;
		// tag = 0x62
		struct {
	unsigned reserved:6;
	unsigned coding_type: 2;
	uint32_t *p_centre_frequency;
		} frequency_list;
		// tag = 0x64
		struct {
	unsigned data_broadcast_id: 16;
	unsigned component_tag: 8;
	unsigned selector_length: 8;
	uint8_t *p_selector_byte;
	unsigned iso_639_language_code: 24;
	unsigned text_length: 8;
	uint8_t *p_text_char;
		} data_broadcast;
		// tag = 0x66
		struct {
	unsigned data_broadcast_id: 16;
		} data_broadcast_id;
	};
};


uint8_t parse_descriptor_header(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos);
uint16_t parse_ca_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos);
//void *allocate_descriptor_storage(struct descriptor_info *p_descriptor_info);
uint16_t parse_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos);
//void deallocate_descriptor_storage(void *p_descriptor);

#endif
