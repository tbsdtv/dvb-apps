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

#define MAX_DESCRIPTOR		20

// tag = 0x02
struct video_stream_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned multiple_frame_rate_flag: 1;
	unsigned frame_rate_code: 4;
	unsigned mpeg_1_only_flag: 1;
	unsigned constrained_parameter_flag: 1;
	unsigned still_picture_flag: 1;

	unsigned profile_and_level_indication: 8;
	unsigned chroma_format: 2;
	unsigned frame_rate_extension_flag: 1;
	unsigned reserved: 5;
};

// tag = 0x03
struct audio_stream_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned free_format_flag: 1;
	unsigned id: 1;
	unsigned layer: 2;
	unsigned variable_rate_audio_indicator: 1;
	unsigned reserved: 3;
};

// tag = 0x04
struct hierarchy_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned reserved_1: 4;
	unsigned hierarchy_type: 4;
	unsigned reserved_2: 2;
	unsigned hierarchy_layer_index: 6;
	unsigned reserved_3: 2;
	unsigned hierarchy_embedded_layer_index: 6;
	unsigned reserved_4: 2;
	unsigned hierarchy_channel: 6;
};

// tag = 0x05
struct registration_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned format_identifier: 32;

	uint8_t *p_additional_id_info;
};

// tag = 0x06
struct data_stream_alignment_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned alignment_type: 8;
};

// tag = 0x07
struct target_background_grid_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned horizontal_size: 14;
	unsigned vertical_size: 14;
	unsigned aspect_ratio_information: 4;
};

// tag = 0x08
struct video_window_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned horizontal_offset: 14;
	unsigned vertical_offset: 14;
	unsigned window_priority: 4;
};

// tag = 0x09
struct ca_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned ca_system_id: 16;
	unsigned reserved: 3;
	unsigned ca_pid: 13;

	uint8_t *p_private_data_byte;
};

// tag = 0x0a
struct iso_639_language_code {
	unsigned iso_639_language_code: 24;
	unsigned audio_type: 8;
};

struct iso_639_language_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;

	struct iso_639_language_code *p_iso_639_language_code;
};

// tag = 0x0b
struct system_clock_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned external_clock_reference_indicator: 1;
	unsigned reserved_1: 1;
	unsigned clock_accuracy_integer: 6;
	unsigned clock_accuracy_exponent: 3;
	unsigned reserved_2: 5;
};

// tag = 0x0c
struct multiplex_buffer_utilization_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned bound_valid_flag: 1;
	unsigned ltw_offset_lower_bound: 15;
	unsigned reserved: 1;
	unsigned ltw_offset_upper_bound: 14;
};

// tag = 0x0d
struct copyright_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned copyright_identifier: 32;

	uint8_t *p_additional_copyright_info;
};

// tag = 0x0e
struct maximum_bitrate_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned reserved: 2;
	unsigned maximum_bitrate: 22;
};

// tag = 0x0f
struct private_data_indicator_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned private_data_indicator: 32;
};

// tag = 0x10
struct smoothing_buffer_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned reserved_1: 2;
	unsigned sb_leak_rate: 22;
	unsigned reserved_2: 2;
	unsigned sb_size: 22;
};

// tag = 0x11
struct std_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned reserved: 7;
	unsigned leak_valid_flag: 1;
};

// tag = 0x12
struct ibp_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned closed_gop_flag: 1;
	unsigned identical_gop_flag: 1;
	unsigned max_gop_length: 14;
};

// tag = 0x1b
struct mpeg_4_video_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned mpeg_4_visual_profile_and_level: 8;
};

// tag = 0x1c
struct mpeg_4_audio_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 16;
	unsigned mpeg_4_audio_profile_and_level: 8;
};

// tag = 0x1d
struct initial_object_descriptor {
/*	described in ISO/IEC 14496-1	*/
};

struct iod_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned scope_of_iod_label: 8;
	unsigned iod_label: 8;
	struct initial_object_descriptor *p_initial_object_descriptor;
};

// tag = 0x1e
struct sl_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned es_id: 16;
};

// tag = 0x1f
struct flex_mux {
	unsigned es_id: 16;
	unsigned flex_mux_channel: 8;
};

struct fmc_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	struct flex_mux *p_flex_mux;
};

// tag = 0x20
struct external_es_id_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned external_es_id: 16;
};

// tag = 0x21
struct muxcode_table_entry {
/*	described in ISO/IEC 14496-1	*/
};

struct muxcode_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	struct muxcode_table_entry *p_muxcode_table_entry;
};

// tag = 0x22
struct default_flexmux_buffer_descriptor {
/*	defined in ISO/IEC 14496-1	*/
};

struct fmxbuffer_size_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	struct default_flexmux_buffer_descriptor *p_default_flexmux_buffer_descriptor;
};

// tag = 0x23
struct multiplex_buffer_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned mb_buffer_size: 24;
	unsigned tb_leak_rate: 24;
};

/*	Some new descriptors (EN 300 468)	*/

// tag = 0x40
struct network_name_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	uint8_t *p_network_name;
};

// tag = 0x41
struct service_list {
	unsigned service_id: 16;
	unsigned service_type: 8;
};

struct service_list_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;

	struct service_list *p_service_list;
};

// tag = 0x42
struct stuffing_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	uint8_t *p_stuffing_bytes;
};

// tag = 0x43
struct satellite_delivery_system {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned frequency: 32;
	unsigned orbital_position: 16;
	unsigned west_east_flag: 1;
	unsigned polarization: 2;
	unsigned modulation: 5;
	unsigned symbol_rate: 28;
	unsigned fec_inner: 4;
};

// tag = 0x44
struct cable_delivery_system {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned frequency: 32;
	unsigned reserved: 12;
	unsigned fec_outer: 4;
	unsigned modulation: 8;
	unsigned symbol_rate: 28;
	unsigned fec_inner: 4;
};

// tag = 0x47
struct bouquet_name_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	uint8_t *p_bouquet_name;
};

// tag = 0x48
struct service_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned service_type: 8;

	unsigned service_provider_name_length: 8;
	uint8_t *p_service_provider_name;

	unsigned service_name_length: 8;
	uint8_t *p_service_name;
};

// tag = 0x49
struct country_loop {
	unsigned country_code: 24;
};

struct country_availability_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned country_availability_flag: 1;
	unsigned reserved: 7;
	struct country_loop *p_country_loop;
};

// tag = 0x4a
struct linkage_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned transport_stream_id: 16;
	unsigned original_network_id: 16;
	unsigned service_id: 16;
	unsigned linkage_type: 8;
	uint8_t *p_private_data;
};

// tag = 0x4b
struct nvod_reference {
	unsigned transport_stream_id: 16;
	unsigned original_network_id: 16;
	unsigned service_id: 16;
};

struct nvod_reference_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	struct nvod_reference *p_nvod_reference;
};

// tag = 0x4c
struct time_shifted_service_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned reference_service_id: 16;
};

// tag = 0x4d
struct short_event_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned iso_639_language_code: 24;

	unsigned event_name_length: 8;
	uint8_t *p_event_name;

	unsigned text_length: 8;
	uint8_t *p_text_char;
};

// tag = 0x4e
struct items {
	unsigned item_description_length: 8;
	uint8_t *p_item_description;

	unsigned item_length: 8;
	uint8_t *p_item_char;
};

struct extended_event_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned descriptor_number: 4;
	unsigned last_descriptor_number: 4;
	unsigned iso_639_language_code: 24;

	unsigned length_of_items: 8;
	struct items *p_items;

	unsigned text_length: 8;
	uint8_t *p_text_char;
};

// tag = 0x4f
struct time_shifted_event_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned reference_service_id: 16;
	unsigned reference_event_id: 16;
};

// tag = 0x50
struct component_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned reserved: 4;
	unsigned stream_content: 4;
	unsigned component_type: 8;
	unsigned component_tag: 8;
	unsigned iso_639_language_code: 24;
	uint8_t *p_text_char;
};

// tag = 0x51
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

struct mosaic_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned mosaic_entry_point: 1;
	unsigned number_of_horiz_elementary_cells: 3;
	unsigned reserved: 1;
	unsigned number_of_vert_elementary_cells: 3;

	struct mosaic_info *p_mosaic_info;
};

// tag = 0x52
struct stream_identifier_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned component_tag: 8;
};

// tag = 0x53
struct ca_identifier_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	uint16_t *p_ca_identifier;
};

// tag = 0x54
struct content {
	unsigned content_nibble_level_1: 4;
	unsigned content_nibble_level_2: 4;
	unsigned user_nibble_1: 4;
	unsigned user_nibble_2: 4;
};

struct content_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;

	struct content *p_content;
};

// tag = 0x55
struct parental_rating {
	unsigned country_code: 24;
	unsigned rating: 8;
};

struct parental_rating_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	struct parental_rating *p_parental_rating;
};

// tag = 0x56
struct teletext {
	unsigned iso_639_language_code: 24;
	unsigned teletext_type: 5;
	unsigned teletext_magazine_number: 3;
	unsigned teletext_page_number: 8;
};

struct teletext_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;

	struct teletext *p_teletext;
};

// tag = 0x57
struct telephone_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
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
};

// tag = 0x58
struct local_time_offset {
	unsigned country_code: 24;
	unsigned country_region_id: 6;
	unsigned reserved: 1;
	unsigned local_time_offset_polarity: 1;
	unsigned local_time_offset: 16;
	unsigned long long time_of_change;
	unsigned next_time_offset: 16;
};

struct local_time_offset_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	struct local_time_offset *p_local_time_offset;
};

// tag = 0x59
struct subtitle {
	unsigned iso_639_language_code: 24;
	unsigned subtitling_type: 8;
	unsigned composition_page_id: 16;
	unsigned ancillary_page_id: 16;
};

struct subtitling_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;

	struct subtitle *p_subtitle;
};

// tag = 0x5a
struct terrestrial_delivery_system {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
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
};

// tag = 0x5b
struct multilingual_network_name {
	unsigned iso_639_language_code: 24;
	unsigned network_name_length: 8;
	uint8_t *p_char;
};

struct multilingual_network_name_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	struct multilingual_network_name *p_multilingual_network_name;
};

// tag = 0x5c
struct multilingual_bouquet_name {
	unsigned iso_639_language_code: 24;
	unsigned bouquet_name_length: 8;
	uint8_t *p_char;
};

struct multilingual_bouquet_name_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	struct multilingual_bouquet_name *p_multilingual_bouquet_name;
};

// tag = 0x5d
struct multilingual_service_name {
	unsigned iso_639_language_code: 24;

	unsigned service_provider_name_length: 8;
	uint8_t *p_service_provider_name;

	unsigned service_name_length: 8;
	uint8_t *p_service_name;
};

struct multilingual_service_name_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	struct multilingual_service_name *p_multilingual_service_name;
};

// tag = 0x5e
struct multilingual_component {
	unsigned iso_639_language_code: 24;
	unsigned text_description_length: 8;
	uint8_t *p_text_char;
};

struct multilingual_component_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned component_tag: 8;
	struct multilingual_component *p_mulyilingual_component;
};

// tag = 0x5f
struct private_data_specifier_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned private_data_specifier: 32;
};

// tag = 0x60
struct service_move_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned new_original_network_id: 16;
	unsigned new_transport_stream_id: 16;
	unsigned new_service_id: 16;
};

// tag = 0x61
struct short_smoothing_buffer_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned sb_size: 2;
	unsigned sb_leak_rate: 6;
	uint8_t *p_dvb_reserved;
};


// tag = 0x62
struct frequency_list_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned reserved:6;
	unsigned coding_type: 2;
	uint32_t *p_centre_frequency;
};

// tag = 0x64
struct data_broadcast_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned data_broadcast_id: 16;
	unsigned component_tag: 8;

	unsigned selector_length: 8;
	uint8_t *p_selector_byte;
	unsigned iso_639_language_code: 24;

	unsigned text_length: 8;
	uint8_t *p_text_char;
};

// tag = 0x66
struct data_broadcast_id_descriptor {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
	unsigned data_broadcast_id: 16;
};

struct descriptor_info {
	unsigned descriptor_tag: 8;
	unsigned descriptor_length: 8;
};


uint8_t parse_descriptor_info(struct descriptor_info *p_descr_info, uint8_t *buf, uint16_t pos);
uint16_t parse_ca_descriptor(struct ca_descriptor *p_descriptor, uint8_t *buf, uint16_t pos);
void *allocate_descriptor_storage(struct descriptor_info *p_descriptor_info);
uint16_t parse_descriptor(void *p_descr, struct descriptor_info *p_descriptor_info, uint8_t *buf, uint16_t pos);
//void deallocate_descriptor_storage(void *p_descriptor);

#endif
