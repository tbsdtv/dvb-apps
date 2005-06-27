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

#include <stdio.h>
#include <stdint.h>
#include "descriptor.h"

uint8_t parse_descriptor_info(struct descriptor_info *p_descr_info, uint8_t *buf, uint16_t pos)
{
	p_descr_info->descriptor_tag = buf[pos + 0];
	p_descr_info->descriptor_length = buf[pos + 1];

	return 0;
}

uint16_t parse_multilingual_bouquet_name_descriptor
	(struct multilingual_bouquet_name_descriptor *p_descriptor, uint8_t * buf, uint16_t pos)
{
	uint8_t i, j;
	struct multilingual_bouquet_name *p_multilingual_bouquet_name = NULL;
	uint8_t *p_char = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	p_multilingual_bouquet_name = (struct multilingual_bouquet_name *)
		malloc(sizeof (struct multilingual_bouquet_name) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i += 4) {
		p_multilingual_bouquet_name[i].iso_639_language_code =
			(((buf[pos + 2 + i] << 8) | buf[pos + 3 + i]) << 8) | buf[pos + 4 + i];
		p_multilingual_bouquet_name[i].bouquet_name_length = buf[pos + 5 + i];

		p_char = (uint8_t *) malloc(sizeof (uint8_t) * (p_multilingual_bouquet_name[i].bouquet_name_length));
		for (j = 0; j < p_multilingual_bouquet_name[i].bouquet_name_length; j++)
			p_char[j] = buf[pos + 6 + i + j];
		i += j;
		p_multilingual_bouquet_name[i].p_char = p_char;
	}
	p_descriptor->p_multilingual_bouquet_name = p_multilingual_bouquet_name;
	pos += 7 + i;

	return pos;
}

void deallocate_multilingual_bouquet_name_descriptor(struct multilingual_bouquet_name_descriptor *p_descriptor)
{
	uint8_t i;

	if (p_descriptor->descriptor_length) {
		for (i = 0; i < p_descriptor->descriptor_length; i++) {
			if (p_descriptor->p_multilingual_bouquet_name[i].bouquet_name_length)
				free(p_descriptor->p_multilingual_bouquet_name[i].p_char);
		}
		free(p_descriptor->p_multilingual_bouquet_name);
	}
}

uint16_t parse_multilingual_network_name_descriptor
	(struct multilingual_network_name_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint8_t i, j;
	struct multilingual_network_name *p_multilingual_network_name = NULL;
	uint8_t *p_char = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	p_multilingual_network_name = (struct multilingual_network_name *)
		malloc(sizeof (struct multilingual_network_name) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i += 4) {
		p_multilingual_network_name[i].iso_639_language_code =
			(((buf[pos + 2 + i] << 8) | buf[pos + 3 + i]) << 8) | buf[pos + 4 + i];
		p_multilingual_network_name[i].network_name_length = buf[pos + 5 + i];

		p_char = (uint8_t *) malloc(sizeof (uint8_t) * p_multilingual_network_name[i].network_name_length);
		for (j = 0; j < p_multilingual_network_name[i].network_name_length; j++)
			p_char[j] = buf[pos + 6 + i + j];
		i += j;
		p_multilingual_network_name[i].p_char = p_char;
	}
	p_descriptor->p_multilingual_network_name = p_multilingual_network_name;
	pos += 7 + i;

	return pos;
}

void deallocate_multilingual_network_name_descriptor(struct multilingual_network_name_descriptor *p_descriptor)
{
	uint8_t i;

	if (p_descriptor->descriptor_length) {
		for (i = 0; i < p_descriptor->descriptor_length; i++) {
			if (p_descriptor->p_multilingual_network_name[i].network_name_length)
				free(p_descriptor->p_multilingual_network_name[i].p_char);
		}
		free(p_descriptor->p_multilingual_network_name);
	}
}


uint16_t parse_terrestrial_delivery_system
	(struct terrestrial_delivery_system *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->centre_frequency =
		(((((buf[pos + 2] << 8 ) | buf[pos + 3]) << 8) | buf[pos + 4]) << 8) | buf[pos + 5];
	p_descriptor->bandwidth = buf[pos + 6] >> 5;
	p_descriptor->reserved_1 = buf[pos + 6] & 0x1f;
	p_descriptor->constellation = buf[pos + 7] >> 6;
	p_descriptor->hierarchy_information = (buf[pos + 7] >> 3) & 0x07;
	p_descriptor->code_rate_hp_stream = buf[pos + 7] & 0x07;
	p_descriptor->code_rate_lp_stream = buf[pos + 8] >> 5;
	p_descriptor->guard_interval = (buf[pos + 8] >> 3) & 0x03;
	p_descriptor->transmission_mode = (buf[pos + 8] >> 1) & 0x03;
	p_descriptor->other_frequency_flag = buf[pos + 8] & 0x01;
	p_descriptor->reserved_2 =
		(((((buf[pos + 9] << 8) | buf[pos + 10]) << 8) | buf[pos + 11]) << 8) | buf[pos + 12];

	pos += 13;

	return pos;
}

void deallocate_terrestrial_delivery_system(struct terrestrial_delivery_system *p_descriptor)
{
	return;
}

uint16_t parse_subtitling_descriptor
	(struct subtitling_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct subtitle *p_subtitle = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	p_subtitle = (struct subtitle *)
		malloc(sizeof (struct subtitle) * (p_descriptor->descriptor_length / 64));
	for (i = 0; i < p_descriptor->descriptor_length; i += 64) {
		p_subtitle[i].iso_639_language_code =
			(((buf[pos + 2] << 8) | buf[pos + 3]) << 8) | buf[pos + 4];
		p_subtitle[i].subtitling_type = buf[pos + 5];
		p_subtitle[i].composition_page_id = (buf[pos + 6] << 8) | buf[pos + 7];
		p_subtitle[i].ancillary_page_id = (buf[pos + 8] << 8) | buf[pos + 9];
	}
	p_descriptor->p_subtitle = p_subtitle;
	pos += 10;

	return pos;
}

void deallocate_subtitling_descriptor(struct subtitling_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_subtitle);
}

uint16_t parse_local_time_offset_descriptor
	(struct local_time_offset_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct local_time_offset *p_local_time_offset = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	p_local_time_offset = (struct local_time_offset *)
		malloc(sizeof (struct local_time_offset) * (p_descriptor->descriptor_length / 13) );
	for (i = 0; i < p_descriptor->descriptor_length; i += 13) {
		p_local_time_offset[i].country_code =
			(((buf[pos + 2] << 8) | buf[pos + 3]) << 8) | buf[pos + 4];
		p_local_time_offset[i].country_region_id = buf[pos + 5] >> 2;
		p_local_time_offset[i].reserved = (buf[pos + 5] >> 1) & 0x01;
		p_local_time_offset[i].local_time_offset_polarity = buf[pos + 5] & 0x01;
		p_local_time_offset[i].local_time_offset = (buf[pos + 6] << 8) | buf[pos + 7];
		p_local_time_offset[i].time_of_change =
			(((((((buf[pos + 8] << 8) | buf[pos + 9]) << 8) | buf[pos + 10]) << 8) |
					buf[pos + 11]) << 8) | buf[pos + 12];
		p_local_time_offset[i].next_time_offset = (buf[pos + 13] << 8) | buf[pos + 14];
	}
	p_descriptor->p_local_time_offset = p_local_time_offset;
	pos += 15;

	return pos;
}

void deallocate_local_time_offset_descriptor(struct local_time_offset_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_local_time_offset);
}

uint16_t parse_telephone_descriptor
	(struct telephone_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_country_prefix = NULL;
	uint8_t *p_international_area_code = NULL;
	uint8_t *p_operator_code = NULL;
	uint8_t *p_national_area_code = NULL;
	uint8_t *p_core_number = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->reserved_1 = buf[pos + 2] >> 6;
	p_descriptor->foreign_availability = (buf[pos + 2] >> 5) & 0x01;
	p_descriptor->connection_type = buf[pos + 2] & 0x1f;
	p_descriptor->reserved_2 = buf[pos + 3] >> 7;
	p_descriptor->country_prefix_length = (buf[pos + 3] >> 5) & 0x03;
	p_descriptor->international_area_code_length = (buf[pos + 3] >> 2) & 0x07;
	p_descriptor->operator_code_length = buf[pos + 3] & 0x03;
	p_descriptor->reserved_3 = buf[pos + 4] >> 7;
	p_descriptor->national_area_code_length = (buf[pos + 4] >> 4) & 0x07;
	p_descriptor->core_number_length = buf[pos + 4] & 0x0f;

	p_country_prefix = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->country_prefix_length);
	for (i = 0; i < p_descriptor->country_prefix_length; i++)
		p_country_prefix[i] = buf[pos + 5 + i];
	p_descriptor->p_country_prefix = p_country_prefix;
	pos += 6 + i;

	p_international_area_code = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->international_area_code_length);
	for (i = 0; i < p_descriptor->international_area_code_length; i++)
		p_international_area_code[i] = buf[pos + i];
	p_descriptor->p_international_area_code = p_international_area_code;
	pos += i;

	p_operator_code = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->operator_code_length);
	for (i = 0; i < p_descriptor->operator_code_length; i++)
		p_operator_code[i] = buf[pos + i];
	p_descriptor->p_operator_code = p_operator_code;
	pos += i;

	p_national_area_code = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->national_area_code_length);
	for (i = 0; i < p_descriptor->national_area_code_length; i++)
		p_national_area_code[i] = buf[pos + i];
	p_descriptor->p_national_area_code = p_national_area_code;
	pos += i;

	p_core_number = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->core_number_length);
	for (i = 0; i < p_descriptor->core_number_length; i++)
		p_core_number[i] = buf[pos + i];
	p_descriptor->p_core_number = p_core_number;
	pos += i;

	return pos;
}

void deallocate_telephone_descriptor(struct telephone_descriptor *p_descriptor)
{
	if (p_descriptor->country_prefix_length)
		free(p_descriptor->p_country_prefix);
	if (p_descriptor->international_area_code_length)
		free(p_descriptor->p_international_area_code);
	if (p_descriptor->operator_code_length)
		free(p_descriptor->p_operator_code);
	if (p_descriptor->national_area_code_length)
		free(p_descriptor->p_national_area_code);
	if (p_descriptor->core_number_length)
		free(p_descriptor->p_core_number);
}

uint16_t parse_teletext_descriptor
	(struct teletext_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct teletext *p_teletext = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	p_teletext = (struct teletext *)
		malloc(sizeof (struct teletext) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i += 5) {
		p_teletext[i].iso_639_language_code =
			(((buf[pos + 2 + i] << 8) | buf[pos + 3 + i]) << 8) | buf[pos + 4 + i];
		p_teletext[i].teletext_type = buf[pos + 5 + i] >> 3;
		p_teletext[i].teletext_magazine_number = buf[pos + 5 + i] & 0x07;
		p_teletext[i].teletext_page_number = buf[pos + 6 + i];
	}
	p_descriptor->p_teletext = p_teletext;
	pos += 6 + i;

	return pos;
}

void deallocate_teletext_descriptor(struct teletext_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_teletext);
}

uint16_t parse_parental_rating_descriptor
	(struct parental_rating_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct parental_rating *p_parental_rating = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_parental_rating = (struct parental_rating *)
		malloc(sizeof (struct parental_rating) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i+= 4) {
		p_parental_rating[i].country_code =
			(((buf[pos + 2 + i] << 8) | buf[pos + 3 + i]) << 8) | buf[pos + 4 + 1];
		p_parental_rating[i].rating = buf[pos + 5 + i];
	}
	p_descriptor->p_parental_rating = p_parental_rating;
	pos += 6 + i;

	return pos;
}

void deallocate_parental_rating_descriptor(struct parental_rating_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_parental_rating);
}

uint16_t parse_content_descriptor
	(struct content_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct content *p_content = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	p_content = (struct content *)
		malloc(sizeof (struct content) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i += 2) {
		p_content[i].content_nibble_level_1 = buf[pos + 2 + i] >> 4;
		p_content[i].content_nibble_level_2 = buf[pos + 2 + i] & 0x0f;
		p_content[i].user_nibble_1 = buf[pos + 3 + i] >> 4;
		p_content[i].user_nibble_2 = buf[pos + 3 + i] & 0x0f;
	}
	p_descriptor->p_content = p_content;
	pos += 4 + i;

	return pos;
}

void deallocate_content_descriptor(struct content_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_content);
}

uint16_t parse_component_descriptor
	(struct component_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_text_char = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->reserved = buf[pos + 2] >> 4;
	p_descriptor->stream_content = buf[pos + 2] & 0x0f;
	p_descriptor->component_type = buf[pos + 3];
	p_descriptor->component_tag = buf[pos + 4];
	p_descriptor->iso_639_language_code =
		(((buf[pos + 5] << 8) | buf[pos + 6]) << 8) | buf[pos + 7];

	p_text_char = (uint8_t *)
		malloc(sizeof (uint8_t) * (p_descriptor->descriptor_length - 6) );
	for (i = 0; i < (p_descriptor->descriptor_length - 6); i++)
		p_text_char[i] = buf[pos + 8 + i];
	p_descriptor->p_text_char = p_text_char;
	pos += 9 + i;

	return pos;
}

void deallocate_component_descriptor(struct component_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_text_char);
}

uint16_t parse_time_shifted_event_descriptor
	(struct time_shifted_event_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->reference_service_id = (buf[pos + 2] << 8) | buf[pos + 3];
	p_descriptor->reference_event_id = (buf[pos + 4] << 8) | buf[pos + 5];

	pos += 6;

	return pos;
}

void deallocate_time_shifted_event_descriptor
	(struct time_shifted_event_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_extended_event_descriptor
	(struct extended_event_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i, j;

	struct items *p_items = NULL;
	uint8_t *p_item_description = NULL;
	uint8_t *p_text_char = NULL;
	uint8_t *p_text = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->descriptor_number = buf[pos + 2] >> 4;
	p_descriptor->last_descriptor_number = buf[pos + 2] & 0x0f;
	p_descriptor->iso_639_language_code =
		(((buf[pos + 3] << 8) | buf[pos + 4]) << 8) | buf[pos + 5];
	p_descriptor->length_of_items = buf[pos + 6];

	p_items = (struct items *)
		malloc(sizeof (struct items) * p_descriptor->length_of_items);
	for (i = 0; i < p_descriptor->length_of_items; i++) {

		p_items[i].item_description_length = buf[pos + 7 + i];
		p_item_description = (uint8_t *)
			malloc(sizeof (uint8_t) * p_items[i].item_description_length);
		for (j = 0; j < p_items[i].item_description_length; j++)
			p_item_description[j] = buf[pos + 7 + i + j];
		p_items[i].p_item_description = p_item_description;
		pos += j;

		p_items[i].item_length = buf[pos + 8 + i]; // since j is already added
		p_text_char = (uint8_t *)
			malloc(sizeof (uint8_t) * p_items[i].item_length);
		for (j = 0; j < p_items[i].item_length; j++)
			p_text_char[j] = buf[pos + 9 + i + j];
		p_items[i].p_item_char = p_text_char;
		pos += j;
	}
	p_descriptor->p_items = p_items;
	pos += 10 + i;

	p_descriptor->text_length = buf[pos + 0];
	p_text = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->text_length);

	for (i = 0; i < p_descriptor->text_length; i++)
		p_text[i] = buf[pos + 1 + i];
	p_descriptor->p_text_char = p_text;
	pos += 2 + i;


	return pos;
}

void deallocate_extended_event_descriptor
	(struct extended_event_descriptor *p_descriptor)
{
	if (p_descriptor->length_of_items)
		free(p_descriptor->p_items);
	if (p_descriptor->text_length)
		free(p_descriptor->p_text_char);
}

uint16_t parse_short_event_descriptor
	(struct short_event_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_event = NULL;
	uint8_t *p_char = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->iso_639_language_code =
		(((buf[pos + 2] << 8) | buf[pos + 3]) << 8) | buf[pos + 4];
	p_descriptor->event_name_length = buf[pos + 5];

	p_event = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->event_name_length);
	for (i = 0; i < p_descriptor->event_name_length; i++)
		p_event[i] = buf[pos + 6 + i];

	p_descriptor->p_event_name = p_event;
	pos += 7 + i;

	p_descriptor->text_length = buf[pos + 0];
	p_char = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->text_length);
	for (i = 0; i < p_descriptor->text_length; i++)
		p_char[i] = buf[pos + 1 + i];

	p_descriptor->p_text_char = p_char;
	pos += 2 + i;

	return pos;
}

void deallocate_short_event_descriptor
	(struct short_event_descriptor *p_descriptor)
{
	if (p_descriptor->event_name_length)
		free(p_descriptor->p_event_name);
	if (p_descriptor->text_length)
		free(p_descriptor->p_text_char);
}

uint16_t parse_time_shifted_service_descriptor
	(struct time_shifted_service_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->reference_service_id =
		(buf[pos + 2] << 8) | buf[pos + 3];

	pos += 4;

	return pos;
}

void deallocate_time_shifted_service_descriptor
	(struct time_shifted_service_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_nvod_reference_descriptor
	(struct nvod_reference_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct nvod_reference *p_reference = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	p_reference = (struct nvod_reference *)
		malloc(sizeof (struct nvod_reference) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_tag; i += 8) {
		p_reference[i].transport_stream_id =
			(buf[pos + 2 + i] << 8) | buf[pos + 3 + i];
		p_reference[i].original_network_id =
			(buf[pos + 4 + i] << 8) | buf[pos + 5 + i];
		p_reference[i].service_id =
			(buf[pos + 6 + i] << 8) | buf[pos + 7 + i];
	}

	p_descriptor->p_nvod_reference = p_reference;
	pos += 8 + i;

	return pos;
}

void deallocate_nvod_reference_descriptor
	(struct nvod_reference_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_nvod_reference);
}

uint16_t parse_linkage_descriptor
	(struct linkage_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->transport_stream_id = (buf[pos + 2] << 8) | buf[pos + 3];
	p_descriptor->original_network_id = (buf[pos + 4] << 8) | buf[pos + 5];
	p_descriptor->service_id =
		((p_descriptor->service_id | buf[pos + 6]) << 8) | buf[pos + 7];
	p_descriptor->linkage_type = buf[pos + 8];

	pos += 9;

	return pos;
}

void deallocate_linkage_descriptor(struct linkage_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_country_availability_descriptor
	(struct country_availability_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct country_loop *p_country = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->country_availability_flag = buf[pos + 2] >> 7;
	p_descriptor->reserved = buf[pos + 2] & 0x7f;

	p_country = (struct country_loop *)
		malloc(sizeof (struct country_loop) * (p_descriptor->descriptor_length - 1) );
	for (i = 0; i < (p_descriptor->descriptor_length - 1); i += 3)
		p_country[i].country_code =
			(((buf[pos + 3 + i] << 8) | buf[pos + 4 + i]) << 8) | buf[pos + 5 + i];

	p_descriptor->p_country_loop = p_country;
	pos += 6 + i;

	return pos;
}

void deallocate_country_availability_descriptor
	(struct country_availability_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_country_loop);
}

uint16_t parse_service_descriptor
	(struct service_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_provider_name = NULL;
	uint8_t *p_service = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->service_type = buf[pos + 2];
	p_descriptor->service_provider_name_length = buf[pos + 3];

	p_provider_name = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->service_provider_name_length);
	for (i = 0; i < p_descriptor->service_provider_name_length; i++)
		p_provider_name[i] = buf[pos + 4 + i];

	p_descriptor->p_service_provider_name = p_provider_name;
	pos += 5;
	p_descriptor->service_name_length = buf[pos + 0];
	p_service = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->service_name_length);
	for (i = 0; i < p_descriptor->service_name_length; i++)
		p_service[i] = buf[pos + 1 + i];
	p_descriptor->p_service_name = p_service;
	pos += 2;

	return pos;
}

void deallocate_service_descriptor(struct service_descriptor *p_descriptor)
{
	if (p_descriptor->service_provider_name_length)
		free(p_descriptor->p_service_provider_name);
	if (p_descriptor->service_name_length)
		free(p_descriptor->p_service_name);
}

uint16_t parse_bouquet_name_descriptor
	(struct bouquet_name_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_bouquet = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	p_bouquet = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i++)
		p_bouquet[i] = buf[pos + 2 + i];

	p_descriptor->p_bouquet_name = p_bouquet;
	pos += 3 + i;

	return pos;
}

void deallocate_bouquet_name_descriptor
	(struct bouquet_name_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_bouquet_name);
}

uint16_t parse_cable_delivery_system
	(struct cable_delivery_system *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->frequency =
		(((((buf[pos + 2] << 8) | buf[pos + 3]) << 8) | buf[pos + 4]) << 8) |
				buf[pos + 5];
	p_descriptor->reserved = (buf[pos + 6] << 4) | (buf[pos + 7] >> 4);
	p_descriptor->fec_outer = buf[pos + 8] & 0x0f;
	p_descriptor->modulation = buf[pos + 9];
	p_descriptor->symbol_rate =
		(((((buf[pos + 10] << 8) | buf[pos + 11]) << 8) | buf[pos + 12]) << 8) |
				(buf[pos + 13] >> 4);
	p_descriptor->fec_inner = buf[pos + 14] & 0x0f;

	pos += 15;

	return pos;
}

void deallocate_cable_delivery_system
	(struct cable_delivery_system *p_descriptor)
{
	return;
}

uint16_t parse_satellite_delivery_system
	(struct satellite_delivery_system *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->frequency =
		((((buf[pos + 2] << 8) | buf[pos + 3]) << 8) | buf[pos + 4] << 8) |
				buf[pos + 5];
	p_descriptor->orbital_position = (buf[pos + 6] << 8) | buf[pos + 7];
	p_descriptor->west_east_flag = buf[pos + 8] >> 7;
	p_descriptor->polarization = (buf[pos + 8] >> 5) & 0x03;
	p_descriptor->modulation = buf[pos + 8] & 0x1f;
	p_descriptor->symbol_rate =
		(((((buf[pos + 9] << 8) | buf[pos + 10]) << 8) | buf[pos + 11]) << 6) |
				(buf[pos + 12] >> 2);
	p_descriptor->fec_inner = buf[pos + 12] & 0x0f;

	pos += 13;

	return pos;
}

void deallocate_satellite_delivery_system
	(struct satellite_delivery_system *p_descriptor)
{
	return;
}

uint16_t parse_stuffing_descriptor
	(struct stuffing_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_stuffing_words = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];


	p_stuffing_words = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->descriptor_length);

	for (i = 0; i < p_descriptor->descriptor_length; i++)
		p_stuffing_words[i] = buf[pos + 2 + i];

	p_descriptor->p_stuffing_bytes = p_stuffing_words;

	pos += 3 + i;

	return pos;
}

void deallocate_stuffing_descriptor(struct stuffing_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_stuffing_bytes);
}

uint16_t parse_service_list_descriptor
	(struct service_list_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct service_list *p_services = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	p_services = (struct service_list *)
		malloc(sizeof (struct service_list) * (p_descriptor->descriptor_length / 3));

	for (i = 0; i < p_descriptor->descriptor_length; i += 3) {
		p_services[i].service_id = (buf[pos + 2 + i] << 8) | buf[pos + 3 + i];
		p_services[i].service_type = buf[pos + 4 + i];
	}

	p_descriptor->p_service_list = p_services;

	pos += 4 + i;

	return pos;
}

void deallocate_service_list_descriptor(struct service_list_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_service_list);
}

uint16_t parse_network_name_descriptor
	(struct network_name_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_name = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_name = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->descriptor_length);

	for (i = 0; i < p_descriptor->descriptor_length; i++)
		p_name[i] = buf[pos + 2 + i];

	p_descriptor->p_network_name = p_name;

	pos += 2 + i;

	return pos;
}

void deallocate_network_name_descriptor
	(struct network_name_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_network_name);
}

uint16_t parse_multiplex_buffer_descriptor
	(struct multiplex_buffer_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->mb_buffer_size =
		(((buf[pos + 2] << 8) | buf[pos + 3]) << 8) | buf[pos + 4];
	p_descriptor->tb_leak_rate =
		(((buf[pos + 5] << 8) | buf[pos + 6]) << 8) | buf[pos + 7];

	pos += 8;

	return pos;
}

void deallocate_multiplex_buffer_descriptor
	(struct multiplex_buffer_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_fmxbuffer_size_descriptor
	(struct fmxbuffer_size_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	/*	need ISO/IEC 14496-1	*/
	return pos;
}

void deallocate_fmxbuffer_size_descriptor
	(struct fmxbuffer_size_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_muxcode_descriptor
	(struct muxcode_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	/*	need ISO/IEC 14496-1	*/
	return pos;
}

void deallocate_muxcode_descriptor
	(struct muxcode_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_external_es_id_descriptor
	(struct external_es_id_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->external_es_id = (buf[pos + 2] << 8) | buf[pos + 3];

	pos += 4;

	return pos;
}

void deallocate_external_es_id_descriptor
	(struct external_es_id_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_fmc_descriptor
	(struct fmc_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct flex_mux *p_flexmux = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	p_flexmux = (struct flex_mux *)
		malloc(sizeof (struct flex_mux) * (p_descriptor->descriptor_length / 3));
	for (i = 0; i < p_descriptor->descriptor_length; i += 3) {
		p_descriptor->p_flex_mux[i].es_id =
			(buf[pos + 2 + i] << 8) | buf[pos + 3 + i];
		p_descriptor->p_flex_mux[i].flex_mux_channel = buf[pos + 4 + i];
	}
	p_descriptor->p_flex_mux = p_flexmux;

	pos += 5 + i;

	return pos;
}

void deallocate_fmc_descriptor(struct fmc_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_flex_mux);
}

uint16_t parse_sl_descriptor
	(struct sl_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	/*	need ISO/IEC 14496-1	*/

	return pos;
}

void deallocate_sl_descriptor(struct sl_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_iod_descriptor
	(struct iod_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	/*	need ISO/IEC 14496-1	*/

	return pos;
}

void deallocate_iod_descriptor(struct iod_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_mpeg_4_audio_descriptor
	(struct mpeg_4_audio_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->mpeg_4_audio_profile_and_level = buf[pos + 2];

	pos += 3;

	return pos;
}

void deallocate_mpeg_4_audio_descriptor
	(struct mpeg_4_audio_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_mpeg_4_video_descriptor
	(struct mpeg_4_video_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->mpeg_4_visual_profile_and_level = buf[pos + 2];

	pos += 3;

	return pos;
}

void deallocate_mpeg_4_video_descriptor
	(struct mpeg_4_video_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_ibp_descriptor
	(struct ibp_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->closed_gop_flag = buf[pos + 2] >> 7;
	p_descriptor->identical_gop_flag = (buf[pos + 2] >> 6) & 0x01;
	p_descriptor->max_gop_length =
		((buf[pos + 2] & 0x3f) << 8) | buf[pos + 3];

	pos += 5;

	return pos;
}

void deallocate_ibp_descriptor(struct ibp_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_std_descriptor
	(struct std_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->reserved = buf[pos + 2] >> 1;
	p_descriptor->leak_valid_flag = buf[pos + 2] & 0x01;

	pos += 3;

	return pos;
}

void deallocate_std_descriptor(struct std_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_smoothing_buffer_descriptor
	(struct smoothing_buffer_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->reserved_1 = buf[pos + 2] >> 6;
	p_descriptor->sb_leak_rate =
		((((buf[pos + 2] & 0x3f) << 8) | buf[pos + 3]) << 8) | buf[pos + 4];
	p_descriptor->reserved_2 = buf[pos + 5] >> 6;
	p_descriptor->sb_size =
		((((buf[pos + 5] & 0x3f) << 8) | buf[pos + 6]) << 8) | buf[pos + 7];

	pos += 8;

	return pos;
}

void deallocate_smoothing_buffer_descriptor
	(struct smoothing_buffer_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_private_data_indicator_descriptor
	(struct private_data_indicator_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->private_data_indicator =
		(((((buf[pos + 2] << 8) | buf[pos + 3]) << 8) | buf[pos + 4]) << 8) |
				buf[pos + 5];

	pos += 6;

	return pos;
}

void deallocate_private_data_indicator_descriptor
	(struct private_data_indicator_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_maximum_bitrate_descriptor
	(struct maximum_bitrate_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->reserved = buf[pos + 2] >> 6;
	p_descriptor->maximum_bitrate =
		((((buf[pos + 2] & 0x3f) << 8) | buf[pos + 3]) << 8) | buf[pos + 4];

	pos += 5;

	return pos;
}

void deallocate_maximum_bitrate_descriptor
	(struct maximum_bitrate_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_copyright_descriptor
	(struct copyright_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint8_t i;
	uint8_t *p_additional_copyright_info = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->copyright_identifier =
		(((((buf[pos + 2] << 8) | buf[pos + 3]) << 8) | buf[pos + 4]) << 8) |
					buf[pos + 5];

	p_additional_copyright_info = (uint8_t *)
		malloc(sizeof (uint8_t) * (p_descriptor->descriptor_length - 4) );
	for (i = 0; i < (p_descriptor->descriptor_length - 4); i++)
	//p_descriptor->p_additional_copyright_info[i] = buf[pos + i + 5];
		p_additional_copyright_info[i] = buf[pos + i + 5];
	p_descriptor->p_additional_copyright_info = p_additional_copyright_info;

	pos += i + 6;

	return pos;
}

void deallocate_copyright_descriptor(struct copyright_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length - 4)
		free(p_descriptor->p_additional_copyright_info);
}

uint16_t parse_multiplex_buffer_utilization_descriptor
	(struct multiplex_buffer_utilization_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__,
			p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->bound_valid_flag = buf[pos + 2] & 0x01;
	p_descriptor->ltw_offset_lower_bound = (buf[pos + 2] << 8) | buf[pos + 3];
	p_descriptor->reserved = buf[pos + 4] & 0x01;
	p_descriptor->ltw_offset_upper_bound = ((buf[pos + 4] >> 1) << 7) | (buf[pos + 5] >> 1);

	pos = pos + 6;

	return pos;
}

void deallocate_multiplex_buffer_utilization_descriptor
	(struct multiplex_buffer_utilization_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_system_clock_descriptor
	(struct system_clock_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->external_clock_reference_indicator = buf[pos + 2] & 0x01;
	p_descriptor->reserved_1 = (buf[pos + 2] & 0x03) >> 1;
	p_descriptor->clock_accuracy_integer = buf[pos + 2] >> 2;
	p_descriptor->clock_accuracy_exponent = buf[pos + 3] >> 5;
	p_descriptor->reserved_2 = buf[pos + 3] & 0x1f;

	pos = pos + 4;

	return pos;
}

void deallocate_system_clock_descriptor(struct system_clock_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_iso_639_language_descriptor
	(struct iso_639_language_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t loops;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->p_iso_639_language_code = (struct iso_639_language_code *)
	malloc(sizeof (struct iso_639_language_code) * (p_descriptor->descriptor_length / 4));

	for (loops = 0; loops < (p_descriptor->descriptor_length / 4); loops++) {
		p_descriptor->p_iso_639_language_code[loops].iso_639_language_code =
			((((buf[pos + 2 + loops] << 8) | buf[pos + 3 + loops]) << 8) |
					buf[pos + 4 + loops]);
		p_descriptor->p_iso_639_language_code[loops].audio_type =
				buf[pos + 5 + loops];
	}
	pos = pos + 2 + loops;

	return pos;
}

void deallocate_iso_639_language_descriptor
	(struct iso_639_language_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_iso_639_language_code);
}

uint16_t parse_ca_identifier_descriptor
	(struct ca_identifier_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint16_t *p_ca_identifier = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	p_ca_identifier = (uint16_t *)
		malloc(sizeof (uint16_t) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i += 2)
		p_ca_identifier[i] = ((buf[pos + 2 + i]) << 8) | buf[pos + 3 + i];

	p_descriptor->p_ca_identifier = p_ca_identifier;
	pos += 4 + i;

	return pos;
}

void deallocate_ca_identifier_descriptor(struct ca_identifier_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_ca_identifier);
}

uint16_t parse_stream_identifier_descriptor
	(struct stream_identifier_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->component_tag = buf[pos + 2];

	pos += 3;

	return pos;
}

void deallocate_stream_identifier_descriptor
	(struct stream_identifier_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_mosaic_descriptor
	(struct mosaic_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i, j;

	struct mosaic_info *p_mosaic_info = NULL;
	struct elementary_cell_field *p_elementary_cell_field = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	p_descriptor->mosaic_entry_point = buf[pos + 2] >> 7;
	p_descriptor->number_of_horiz_elementary_cells = (buf[pos + 2] >> 4) & 0x07;
	p_descriptor->reserved = buf[pos + 2] >> 3;
	p_descriptor->number_of_vert_elementary_cells = buf[pos + 2] & 0x07;

	p_mosaic_info = (struct mosaic_info *)
		malloc(sizeof (struct mosaic_info) * (p_descriptor->descriptor_length - 1));
	for (i = 0; i < (p_descriptor->descriptor_length - 1); i++) {
		p_mosaic_info[i].logical_cell_id = buf[pos + 3 + i] >> 2;
		p_mosaic_info[i].reserved =
			((buf[pos + 3 + i] & 0x03) << 5) | (buf[pos + 4 + i] >> 3);
		p_mosaic_info[i].logical_cell_presentation_info = buf[pos + 4 + i] & 0x07;

		p_mosaic_info[i].elementary_cell_field_length = buf[pos + 5 + i];
		p_elementary_cell_field = (struct elementary_cell_field *)
			malloc(sizeof (struct elementary_cell_field) * p_mosaic_info[i].elementary_cell_field_length);
		for (j = 0; j < p_mosaic_info[i].elementary_cell_field_length; j++) {
			p_elementary_cell_field[j].reserved = buf[pos + 6 + i + j] >> 6;
			p_elementary_cell_field[j].elementary_cell_id = buf[pos + 6 + i + j] & 0x3f;
		}
		p_mosaic_info[i].p_elementary_cell_field = p_elementary_cell_field;
		pos += j;

		p_mosaic_info[i].cell_linkage_info = buf[pos + 7 + i];
		switch (p_mosaic_info[i].cell_linkage_info) {
			case 0x01:
				p_mosaic_info[i].bouquet_id = (buf[pos + 8 + 1] << 8) |
						buf[pos + 9 + i];
				break;

			case 0x02:
			case 0x03:
				p_mosaic_info[i].original_network_id = (buf[pos + 10 + i] << 8) |
						buf[pos + 11 + i];
				p_mosaic_info[i].transport_stream_id = (buf[pos + 12 + i] << 8) |
						buf[pos + 13 + i];
				p_mosaic_info[i].service_id = (buf[pos + 14 + i] << 8) |
						buf[pos + 15 + i];
				break;

			case 0x04:
				p_mosaic_info[i].original_network_id = (buf[pos + 10 + i] << 8) |
						buf[pos + 11 + i];
				p_mosaic_info[i].transport_stream_id = (buf[pos + 12 + i] << 8) |
						buf[pos + 13 + i];
				p_mosaic_info[i].service_id = (buf[pos + 14 + i] << 8) |
						buf[pos + 15 + i];
				p_mosaic_info[i].event_id = (buf[pos + 16 + i] << 8) |
						buf[pos + 17 + i];
				break;
			default:
				break;
				return -1;
		}
	}
	p_descriptor->p_mosaic_info = p_mosaic_info;
	pos += 18 + i;

	return pos;
}

void deallocate_mosaic_descriptor(struct mosaic_descriptor *p_descriptor)
{
	uint8_t i;

	if (p_descriptor->descriptor_length) {
		for (i = 0; i < (p_descriptor->descriptor_length - 1); i++) {
			if (p_descriptor->p_mosaic_info[i].elementary_cell_field_length)
				free(p_descriptor->p_mosaic_info[i].p_elementary_cell_field);
		}
		free(p_descriptor->p_mosaic_info);
	}
}

uint16_t parse_ca_descriptor(struct ca_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint8_t loops = 0;
	uint8_t *p_private_data = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->ca_system_id = (buf[pos + 2] << 8) | buf[pos + 3];
	p_descriptor->reserved = buf[pos + 4] >> 5;
	p_descriptor->ca_pid = ((buf[pos + 4] & 0x1f) << 8) | buf[pos + 5];
	p_private_data = (uint8_t *)
		malloc(sizeof (uint8_t) * (p_descriptor->descriptor_length - 4));

	printf("%s: Tag=[%02x], Length=[%02x], CA System=[%02x], CA PID=[%02x]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length,
		p_descriptor->ca_system_id, p_descriptor->ca_pid);
	printf("%s: CA Private Data=[ ", __FUNCTION__);
	while (loops < (p_descriptor->descriptor_length - 4)) {
		p_private_data[loops] = buf[pos + 6 + loops];
		printf("%02x ", p_private_data[loops]);
		loops++;
	}
	printf("]\n");
	p_descriptor->p_private_data_byte = p_private_data;

	pos += loops + 6;
	printf("%s: Pos=[%d]\n", __FUNCTION__, pos);

	return pos;
}

void deallocate_ca_descriptor(struct ca_descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->p_private_data_byte);
}

uint16_t parse_video_window_descriptor
	(struct video_window_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->horizontal_offset = (buf[pos + 2] << 6) | ((buf[pos + 3] >> 2) & 0x3f);
	p_descriptor->vertical_offset = ((((buf[pos + 3] >> 2) << 8) | buf[pos + 4]) << 4) |
			((buf[pos + 5] >> 4) & 0x0f);
	p_descriptor->window_priority =	buf[pos + 5] & 0x0f;

	pos = pos + 6;

	return pos;
}

void deallocate_video_window_descriptor
	(struct video_window_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_target_background_grid_descriptor
	(struct target_background_grid_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->horizontal_size = (buf[pos + 2] << 6) | ((buf[pos + 3] >> 2) & 0x3f);
	p_descriptor->vertical_size =
		((((buf[pos + 3] >> 2) << 8) | buf[pos + 4]) << 4) | ((buf[pos + 5] >> 4) & 0x0f);
	p_descriptor->aspect_ratio_information = buf[pos + 5] & 0x0f;

	pos = pos + 6;

	return pos;
}

void deallocate_target_background_grid_descriptor
	(struct target_background_grid_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_data_stream_alignment_descriptor
	(struct data_stream_alignment_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->alignment_type = buf[pos + 2];

	pos = pos + 3;

	return pos;
}

void deallocate_data_stream_alignment_descriptor
	(struct data_stream_alignment_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_registration_descriptor
	(struct registration_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint32_t i;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->format_identifier =
		((((((buf[pos + 2] << 8) | buf[pos + 3])) << 8) | buf[pos + 4]) << 8) | buf[pos + 5];
	p_descriptor->p_additional_id_info = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->format_identifier);

	for (i = 0; i < p_descriptor->format_identifier; i++)
		p_descriptor->p_additional_id_info[i] = buf[pos + 6 + i];
	pos = pos + i + 7;

	return pos;
}

void deallocate_registration_descriptor(struct registration_descriptor *p_descriptor)
{
	if (p_descriptor->format_identifier)
		free(p_descriptor->p_additional_id_info);
}

uint16_t parse_hierarchy_descriptor
	(struct hierarchy_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->reserved_1 = (buf[pos + 2] >> 4) & 0x0f;
	p_descriptor->hierarchy_type = buf[pos + 2] & 0x0f;
	p_descriptor->reserved_2 = (buf[pos + 3] >> 6) & 0x03;
	p_descriptor->hierarchy_layer_index = buf[pos + 3] & 0x3f;
	p_descriptor->reserved_3 = (buf[pos + 4] >> 6) & 0x03;
	p_descriptor->hierarchy_embedded_layer_index = buf[pos + 4] & 0x3f;
	p_descriptor->reserved_4 = (buf[pos + 5] >> 6) & 0x03;
	p_descriptor->hierarchy_channel = buf[pos + 5] & 0x3f;

	pos = pos + 6;

	return pos;
}

void deallocate_hierarchy_descriptor
	(struct hierarchy_descriptor *p_descriptor)
{
	return;
}

uint16_t parse_audio_stream_descriptor
	(struct audio_stream_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->free_format_flag = (buf[pos + 2] >> 7) & 0x01;
	p_descriptor->id = (buf[pos + 2] >> 6) & 0x01;
	p_descriptor->layer = (buf[pos + 2] >> 4) & 0x03;
	p_descriptor->variable_rate_audio_indicator = (buf[pos + 2] >> 3) & 0x01;
	p_descriptor->reserved = buf[pos + 2] & 0x07;

	pos = pos + 3;

	return pos;
}

void deallocate_audio_stream_descriptor(struct audio_stream_descriptor *p_descrioptor)
{
	return;
}

uint16_t parse_video_stream_descriptor
	(struct video_stream_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->multiple_frame_rate_flag = buf[pos + 2] >> 7;
	p_descriptor->frame_rate_code = (buf[pos + 2] >> 3) & 0x0f;
	p_descriptor->mpeg_1_only_flag = (buf[pos + 2] >> 2) & 0x01;
	p_descriptor->constrained_parameter_flag = (buf[pos + 2] >> 1) & 0x01;
	p_descriptor->still_picture_flag = buf[pos + 2] & 0x01;

	if (p_descriptor->mpeg_1_only_flag == 0) {
		p_descriptor->profile_and_level_indication = buf[pos + 3];
		p_descriptor->chroma_format = (buf[pos + 4] >> 6) & 0x03;
		p_descriptor->frame_rate_extension_flag	= (buf[pos + 4] >> 5) & 0x01;
		p_descriptor->reserved = buf[pos + 4] & 0x1f;
	}
	pos = pos + 5;

	return pos;
}

void deallocate_video_stream_descriptor(struct video_stream_descriptor *p_descriptor)
{
	return;
}

void *allocate_descriptor_storage(struct descriptor_info *p_descriptor_info)
{
	void *p_descriptor = NULL;
	switch (p_descriptor_info->descriptor_tag) {
		case 0x02:
			p_descriptor = (struct video_stream_descriptor *)
			malloc(sizeof (struct video_stream_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x03:
			p_descriptor = (struct audio_stream_descriptor *)
			malloc(sizeof (struct audio_stream_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x04:
			p_descriptor = (struct hierarchy_descriptor *)
			malloc(sizeof (struct hierarchy_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x05:
			p_descriptor = (struct registration_descriptor *)
			malloc(sizeof (struct registration_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x06:
			p_descriptor = (struct data_stream_alignment_descriptor *)
			malloc(sizeof (struct data_stream_alignment_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x07:
			p_descriptor = (struct target_background_grid_descriptor *)
			malloc(sizeof (struct target_background_grid_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x08:
			p_descriptor = (struct video_window_descriptor *)
			malloc(sizeof (struct video_window_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x09:
			p_descriptor = (struct ca_descriptor *)
			malloc(sizeof (struct ca_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x0a:
			p_descriptor = (struct iso_639_language_descriptor *)
			malloc(sizeof (struct iso_639_language_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x0b:
			p_descriptor = (struct system_clock_descriptor *)
			malloc(sizeof (struct system_clock_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x0c:
			p_descriptor = (struct multiplex_buffer_utilization_descriptor *)
			malloc(sizeof (struct multiplex_buffer_utilization_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x0d:
			p_descriptor = (struct copyright_descriptor *)
			malloc(sizeof (struct copyright_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x0e:
			p_descriptor = (struct maximum_bitrate_descriptor *)
			malloc(sizeof (struct maximum_bitrate_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x0f:
			p_descriptor = (struct private_data_indicator_descriptor *)
			malloc(sizeof (struct private_data_indicator_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x10:
			p_descriptor = (struct smoothing_buffer_descriptor *)
			malloc(sizeof (struct smoothing_buffer_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x11:
			p_descriptor = (struct std_descriptor *)
			malloc(sizeof (struct std_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x12:
			p_descriptor = (struct ibp_descriptor *)
			malloc(sizeof (struct ibp_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x1b:
			p_descriptor = (struct mpeg_4_video_descriptor *)
			malloc(sizeof (struct mpeg_4_video_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x1c:
			p_descriptor = (struct mpeg_4_audio_descriptor *)
			malloc(sizeof (struct mpeg_4_audio_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x1d:
			p_descriptor = (struct iod_descriptor *)
			malloc(sizeof (struct iod_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x1e:
			p_descriptor = (struct sl_descriptor *)
			malloc(sizeof (struct sl_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x1f:
			p_descriptor = (struct fmc_descriptor *)
			malloc(sizeof (struct fmc_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x20:
			p_descriptor = (struct external_es_id_descriptor *)
			malloc(sizeof (struct external_es_id_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x21:
			p_descriptor = (struct muxcode_descriptor *)
			malloc(sizeof (struct muxcode_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x22:
			p_descriptor = (struct fmxbuffer_size_descriptor *)
			malloc(sizeof (struct fmxbuffer_size_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x23:
			p_descriptor = (struct multiplex_buffer_descriptor *)
			malloc(sizeof (struct multiplex_buffer_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x40:
			p_descriptor = (struct network_name_descriptor *)
			malloc(sizeof (struct network_name_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x41:
			p_descriptor = (struct service_list_descriptor *)
			malloc(sizeof (struct network_name_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x42:
			p_descriptor = (struct stuffing_descriptor *)
			malloc(sizeof (struct stuffing_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x43:
			p_descriptor = (struct satellite_delivery_system_descriptor *)
			malloc(sizeof (struct satellite_delivery_system) * MAX_DESCRIPTOR);
			break;

		case 0x44:
			p_descriptor = (struct cable_delivery_system_descriptor *)
			malloc(sizeof (struct cable_delivery_system) * MAX_DESCRIPTOR);
			break;

		case 0x47:
			p_descriptor = (struct bouquet_name_descriptor *)
			malloc(sizeof (struct bouquet_name_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x48:
			p_descriptor = (struct service_descriptor *)
			malloc(sizeof (struct service_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x49:
			p_descriptor = (struct country_availability_descriptor *)
			malloc(sizeof (struct country_availability_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x4a:
			p_descriptor = (struct linkage_descriptor *)
			malloc(sizeof (struct linkage_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x4b:
			p_descriptor = (struct nvod_reference_descriptor *)
			malloc(sizeof (struct nvod_reference_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x4c:
			p_descriptor = (struct time_shifted_service_descriptor *)
			malloc(sizeof (struct time_shifted_service_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x4d:
			p_descriptor = (struct short_event_descriptor *)
			malloc(sizeof (struct short_event_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x4e:
			p_descriptor = (struct extended_event_descriptor *)
			malloc(sizeof (struct extended_event_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x4f:
			p_descriptor = (struct time_shifted_event_descriptor *)
			malloc(sizeof (struct time_shifted_event_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x50:
			p_descriptor = (struct component_descriptor *)
			malloc(sizeof (struct component_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x51:
			p_descriptor = (struct mosaic_descriptor *)
			malloc(sizeof (struct mosaic_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x52:
			p_descriptor = (struct stream_identifier_descriptor *)
			malloc(sizeof (struct stream_identifier_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x53:
			p_descriptor = (struct ca_identifier_descriptor *)
			malloc(sizeof (struct ca_identifier_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x54:
			p_descriptor = (struct content_descriptor *)
			malloc(sizeof (struct content_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x55:
			p_descriptor = (struct parental_rating_descriptor *)
			malloc(sizeof (struct parental_rating_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x56:
			p_descriptor = (struct teletext_descriptor *)
			malloc(sizeof (struct teletext_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x57:
			p_descriptor = (struct telephone_descriptor *)
			malloc(sizeof (struct telephone_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x58:
			p_descriptor = (struct local_time_offset_descriptor *)
			malloc(sizeof (struct local_time_offset_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x59:
			p_descriptor = (struct subtitling_descriptor *)
			malloc(sizeof (struct subtitling_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x5a:
			p_descriptor = (struct terrestrial_delivery_system *)
			malloc(sizeof (struct terrestrial_delivery_system) * MAX_DESCRIPTOR);
			break;

		case 0x5b:
			p_descriptor = (struct multilingual_network_name_descriptor *)
			malloc(sizeof (struct multilingual_network_name_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x5c:
			p_descriptor = (struct nultilingual_bouquet_name_descriptor *)
			malloc(sizeof (struct multilingual_bouquet_name_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x5d:
			p_descriptor = (struct multilingual_service_name_descriptor *)
			malloc(sizeof (struct multilingual_service_name_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x5e:
			p_descriptor = (struct multilingual_component_descriptor *)
			malloc(sizeof (struct multilingual_component_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x5f:
			p_descriptor = (struct private_data_specifier_descriptor *)
			malloc(sizeof (struct private_data_specifier_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x60:
			p_descriptor = (struct service_move_descriptor *)
			malloc(sizeof (struct service_move_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x61:
			p_descriptor = (struct short_smoothing_buffer_descriptor *)
			malloc(sizeof (struct short_smoothing_buffer_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x62:
			p_descriptor = (struct frequency_list_descriptor *)
			malloc(sizeof (struct frequency_list_descriptor) * MAX_DESCRIPTOR);
			break;
/*
		case 0x63:
			p_descriptor = (struct partial_transport_stream_descriptor *)
			malloc(sizeof (struct partial_transport_stream_descriptor) * MAX_DESCRIPTOR);
			break;
*/
		case 0x64:
			p_descriptor = (struct data_broadcast_descriptor *)
			malloc(sizeof (struct data_broadcast_descriptor) * MAX_DESCRIPTOR);
			break;
/*
		case 0x65:
			p_descriptor = (struct ca_system_descriptor *)
			malloc(sizeof (struct ca_system_descriptor) * MAX_DESCRIPTOR);
			break;
*/
		case 0x66:
			p_descriptor = (struct data_broadcast_id_descriptor *)
			malloc(sizeof (struct data_broadcast_id_descriptor) * MAX_DESCRIPTOR);
			break;

	}

	return p_descriptor;
}

void deallocate_descriptor_storage(void *p_descr)
{
//	uint16_t descriptor_loop = 0;

//	parse_descriptor_info(p_descriptor_info, buf, pos);
//	switch (p_descriptor_info->descriptor_tag) {

		struct descriptor_info *p_descriptor_info = NULL;
		p_descriptor_info = (struct descriptor_info *) p_descr;

		switch(p_descriptor_info->descriptor_tag) {
		case 0x02:
			{
				struct video_stream_descriptor *p_descriptor =
					(struct video_stream_descriptor *) p_descr;
				deallocate_video_stream_descriptor(p_descriptor);
			}
			break;

		case 0x03:
			{
				struct audio_stream_descriptor *p_descriptor =
					(struct audio_stream_descriptor *) p_descr;
				deallocate_audio_stream_descriptor(p_descriptor);
			}
			break;

		case 0x04:
			{
				struct hierarchy_descriptor *p_descriptor =
					(struct hierarchy_descriptor *) p_descr;
				deallocate_hierarchy_descriptor(p_descriptor);
			}
			break;

		case 0x05:
			{
				struct registration_descriptor *p_descriptor =
					(struct registration_descriptor *) p_descr;
				deallocate_registration_descriptor(p_descriptor);
			}
			break;

		case 0x06:
			{
				struct data_stream_alignment_descriptor *p_descriptor =
					(struct data_stream_alignment_descriptor *) p_descr;
				deallocate_data_stream_alignment_descriptor(p_descriptor);
			}
			break;

		case 0x07:
			{
				struct target_background_grid_descriptor *p_descriptor =
					(struct target_background_grid_descriptor *) p_descr;
				deallocate_target_background_grid_descriptor(p_descriptor);
			}
			break;

		case 0x08:
			{
				struct video_window_descriptor *p_descriptor =
					(struct video_window_descriptor *) p_descr;
				deallocate_video_window_descriptor(p_descriptor);
			}
			break;

		case 0x09:	/*	CA descriptor		*/
			{
				struct ca_descriptor *p_descriptor =
					(struct ca_descriptor *) p_descr;
				deallocate_ca_descriptor(p_descriptor);
			}
			break;


		case 0x0a:
			{
				struct iso_639_language_descriptor *p_descriptor =
					(struct iso_639_language_descriptor *) p_descr;
				deallocate_iso_639_language_descriptor(p_descriptor);
			}
			break;

		case 0x0b:
			{
				struct system_clock_descriptor *p_descriptor =
					(struct system_clock_descriptor *) p_descr;
				deallocate_system_clock_descriptor(p_descriptor);
			}
			break;

		case 0x0c:
			{
				struct multiplex_buffer_utilization_descriptor *p_descriptor =
					(struct multiplex_buffer_utilization_descriptor *) p_descr;
				deallocate_multiplex_buffer_utilization_descriptor(p_descriptor);
			}
			break;

		case 0x0d:
			{
				struct copyright_descriptor *p_descriptor =
					(struct copyright_descriptor *) p_descr;
				deallocate_copyright_descriptor(p_descriptor);
			}
			break;

		case 0x0e:
			{
				struct maximum_bitrate_descriptor *p_descriptor =
					(struct maximum_bitrate_descriptor *) p_descr;
				deallocate_maximum_bitrate_descriptor(p_descriptor);
			}
			break;

		case 0x0f:
			{
				struct private_data_indicator_descriptor *p_descriptor =
					(struct private_data_indicator_descriptor *) p_descr;
				deallocate_private_data_indicator_descriptor(p_descriptor);
			}
			break;

		case 0x10:
			{
				struct smoothing_buffer_descriptor *p_descriptor =
					(struct smoothing_buffer_descriptor *) p_descr;
				deallocate_smoothing_buffer_descriptor(p_descriptor);
			}
			break;

		case 0x11:
			{
				struct std_descriptor *p_descriptor =
					(struct std_descriptor *) p_descr;
				deallocate_std_descriptor(p_descriptor);
			}
			break;

		case 0x12:
			{
				struct ibp_descriptor *p_descriptor =
					(struct ibp_descriptor *) p_descr;
				deallocate_ibp_descriptor(p_descriptor);
			}
			break;

		case 0x1b:
			{
				struct mpeg_4_video_descriptor *p_descriptor =
					(struct mpeg_4_video_descriptor *) p_descr;
				deallocate_mpeg_4_video_descriptor(p_descriptor);
			}
			break;

		case 0x1c:
			{
				struct mpeg_4_audio_descriptor *p_descriptor =
					(struct mpeg_4_audio_descriptor *) p_descr;
				deallocate_mpeg_4_audio_descriptor(p_descriptor);
			}
			break;

		case 0x1d:
			{
				struct iod_descriptor *p_descriptor =
					(struct iod_descriptor *) p_descr;
				deallocate_iod_descriptor(p_descriptor);
			}
			break;

		case 0x1e:
			{
				struct sl_descriptor *p_descriptor =
					(struct sl_descriptor *) p_descr;
				deallocate_sl_descriptor(p_descriptor);
			}
			break;

		case 0x1f:
			{
				struct fmc_descriptor *p_descriptor =
					(struct fmc_descriptor *) p_descr;
				deallocate_fmc_descriptor(p_descriptor);
			}
			break;

		case 0x20:
			{
				struct external_es_id_descriptor *p_descriptor =
					(struct external_es_id_descriptor *) p_descr;
				deallocate_external_es_id_descriptor(p_descriptor);
			}
			break;

		case 0x21:
			{
				struct muxcode_descriptor *p_descriptor =
					(struct muxcode_descriptor *) p_descr;
				deallocate_muxcode_descriptor(p_descriptor);
			}
			break;

		case 0x22:
			{
				struct fmxbuffer_size_descriptor *p_descriptor =
					(struct fmxbuffer_size_descriptor *) p_descr;
				deallocate_fmxbuffer_size_descriptor(p_descriptor);
			}
			break;

		case 0x23:
			{
				struct multiplex_buffer_descriptor *p_descriptor =
					(struct multiplex_buffer_descriptor *) p_descr;
				deallocate_multiplex_buffer_descriptor(p_descriptor);
			}
			break;

		case 0x40:
			{
				struct network_name_descriptor *p_descriptor =
					(struct network_name_descriptor *) p_descr;
				deallocate_network_name_descriptor(p_descriptor);
			}
			break;

		case 0x41:
			{
				struct service_list_descriptor *p_descriptor =
					(struct service_list_descriptor *) p_descr;
				deallocate_service_list_descriptor(p_descriptor);
			}
			break;

		case 0x42:
			{
				struct stuffing_descriptor *p_descriptor =
					(struct stuffing_descriptor *) p_descr;
				deallocate_stuffing_descriptor(p_descriptor);
			}
			break;

		case 0x43:
			{
				struct satellite_delivery_system *p_descriptor =
					(struct satellite_delivery_system *) p_descr;
				deallocate_satellite_delivery_system(p_descriptor);
			}
			break;

		case 0x44:
			{
				struct cable_delivery_system *p_descriptor =
					(struct cable_delivery_system *) p_descr;
				deallocate_cable_delivery_system(p_descriptor);
			}
			break;

		case 0x47:
			{
				struct bouquet_name_descriptor *p_descriptor =
					(struct bouquet_name_descriptor *) p_descr;
				deallocate_bouquet_name_descriptor(p_descriptor);
			}
			break;

		case 0x48:
			{
				struct service_descriptor *p_descriptor =
					(struct service_descriptor *) p_descr;
				deallocate_service_descriptor(p_descriptor);
			}
			break;

		case 0x49:
			{
				struct country_availability_descriptor *p_descriptor =
					(struct country_availability_descriptor *) p_descr;
				deallocate_country_availability_descriptor(p_descriptor);
			}
			break;

		case 0x4a:
			{
				struct linkage_descriptor *p_descriptor =
					(struct linkage_descriptor *) p_descr;
				deallocate_linkage_descriptor(p_descriptor);
			}
			break;

		case 0x4b:
			{
				struct nvod_reference_descriptor *p_descriptor =
					(struct nvod_reference_descriptor *) p_descr;
				deallocate_nvod_reference_descriptor(p_descriptor);
			}
			break;

		case 0x4c:
			{
				struct time_shifted_service_descriptor *p_descriptor =
					(struct time_shifted_service_descriptor *) p_descr;
				deallocate_time_shifted_service_descriptor(p_descriptor);
			}
			break;

		case 0x4d:
			{
				struct short_event_descriptor *p_descriptor =
					(struct short_event_descriptor *) p_descr;
				deallocate_short_event_descriptor(p_descriptor);
			}
			break;

		case 0x4e:
			{
				struct extended_event_descriptor *p_descriptor =
					(struct extended_event_descriptor *) p_descr;
				deallocate_extended_event_descriptor(p_descriptor);
			}
			break;

		case 0x4f:
			{
				struct time_shifted_event_descriptor *p_descriptor =
					(struct time_shifted_event_descriptor *) p_descr;
				deallocate_time_shifted_event_descriptor(p_descriptor);
			}
			break;

		case 0x50:
			{
				struct component_descriptor *p_descriptor =
					(struct component_descriptor *) p_descr;
				deallocate_component_descriptor(p_descriptor);
			}
			break;

		case 0x51:
			{
				struct mosaic_descriptor *p_descriptor =
					(struct mosaic_descriptor *) p_descr;
				deallocate_mosaic_descriptor(p_descriptor);
			}
			break;

		case 0x52:
			{
				struct stream_identifier_descriptor *p_descriptor =
					(struct stream_identifier_descriptor *) p_descr;
				deallocate_stream_identifier_descriptor(p_descriptor);
			}
			break;

		case 0x53:
			{
				struct ca_identifier_descriptor *p_descriptor =
					(struct ca_identifier_descriptor *) p_descr;
				deallocate_ca_identifier_descriptor(p_descriptor);
			}
			break;

		case 0x54:
			{
				struct content_descriptor *p_descriptor =
					(struct content_descriptor *) p_descr;
				deallocate_content_descriptor(p_descriptor);
			}
			break;

		case 0x55:
			{
				struct parental_rating_descriptor *p_descriptor =
					(struct parental_rating_descriptor *) p_descr;
				deallocate_parental_rating_descriptor(p_descriptor);
			}
			break;

		case 0x56:
			{
				struct teletext_descriptor *p_descriptor =
					(struct teletext_descriptor *) p_descr;
				deallocate_teletext_descriptor(p_descriptor);
			}
			break;

		case 0x57:
			{
				struct telephone_descriptor *p_descriptor =
					(struct telephone_descriptor *) p_descr;
				deallocate_telephone_descriptor(p_descriptor);
			}
			break;
		case 0x58:
			{
				struct local_time_offset_descriptor *p_descriptor =
					(struct local_time_offset_descriptor *) p_descr;
				deallocate_local_time_offset_descriptor(p_descriptor);
			}
			break;

		case 0x59:
			{
				struct subtitling_descriptor *p_descriptor =
					(struct subtitling_descriptor *) p_descr;
				deallocate_subtitling_descriptor(p_descriptor);
			}
			break;

		case 0x5a:
			{
				struct terrestrial_delivery_system *p_descriptor =
					(struct terrestrial_delivery_system *) p_descr;
				deallocate_terrestrial_delivery_system(p_descriptor);
			}
			break;

		case 0x5b:
			{
				struct multilingual_network_name_descriptor *p_descriptor =
					(struct multilingual_network_name_descriptor *) p_descr;
				deallocate_multilingual_network_name_descriptor(p_descriptor);
			}
			break;

		case 0x5c:
			{
				struct multilingual_bouquet_name_descriptor *p_descriptor =
					(struct multilingual_bouquet_name_descriptor *) p_descr;
				deallocate_multilingual_bouquet_name_descriptor(p_descriptor);
			}
			break;

/*		case 0x5d:
			{
				struct multilingual_service_name_descriptor *p_descriptor =
					(struct multilingual_service_name_descriptor *) p_descr;
				deallocate_multilingual_service_name_descriptor(p_descriptor);
			}
			break;

		case 0x5e:
			{
				struct multilingual_component_descriptor *p_descriptor =
					(struct multilingual_component_descriptor *) p_descr;
				deallocate_multilingual_component_descriptor(p_descriptor);
			}
			break;

		case 0x5f:
			{
				struct private_data_specifier_descriptor *p_descriptor =
					(struct private_data_specifier_descriptor *) p_descr;
				deallocate_private_data_specifier_descriptor(p_descriptor);
			}
			break;

		case 0x60:
			{
				struct service_move_descriptor *p_descriptor =
					(struct service_move_descriptor *) p_descr;
				deallocate_service_move_descriptor(p_descriptor);
			}
			break;

		case 0x61:
			{
				struct short_smoothing_buffer_descriptor *p_descriptor =
					(struct short_smoothing_buffer_descriptor *) p_descr;
				deallocate_short_smoothing_buffer_descriptor(p_descriptor);
			}
			break;

		case 0x62:
			{
				struct frequency_list_descriptor *p_descriptor =
					(struct frequency_list_descriptor *) p_descr;
				deallocate_frequency_list_descriptor(p_descriptor);
			}
			break;
*/
//		case 0x63:
//				p_descriptor = (struct partial_transport_stream_descriptor *)
//				malloc(sizeof (struct partial_transport_stream_descriptor) * MAX_DESCRIPTOR);
//			break;

/*		case 0x64:
			{
				struct data_broadcast_descriptor *p_descriptor =
					(struct data_broadcast_descriptor *) p_descr;
				deallocate_data_broadcast_descriptor(p_descriptor);
			}
			break;
*/
//		case 0x65:
//				p_descriptor = (struct ca_system_descriptor *)
//				malloc(sizeof (struct ca_system_descriptor) * MAX_DESCRIPTOR);
//			break;

/*		case 0x66:
			{
				struct data_broadcast_id_descriptor *p_descriptor =
					(struct data_broadcast_id_descriptor *) p_descr;
				deallocate_data_broadcast_id_descriptor(p_descriptor);
			}
			break;
*/
		default:	/*	parse an unknown	*/
//				pos += p_descriptor_info->descriptor_length;
				break;
	}

//	return pos;
	return;
}


uint16_t parse_descriptor(void *p_descr, struct descriptor_info *p_descriptor_info, uint8_t *buf, uint16_t pos)
{
	uint16_t descriptor_loop = 0;

	parse_descriptor_info(p_descriptor_info, buf, pos);
	switch (p_descriptor_info->descriptor_tag) {
		case 0x02:
			{
				struct video_stream_descriptor *p_descriptor =
						(struct video_stream_descriptor *) p_descr;
				pos = parse_video_stream_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x03:
			{
				struct audio_stream_descriptor *p_descriptor =
						(struct audio_stream_descriptor *) p_descr;
				pos = parse_audio_stream_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x04:
			{
				struct hierarchy_descriptor *p_descriptor =
						(struct hierarchy_descriptor *) p_descr;
				pos = parse_hierarchy_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x05:
			{
				struct registration_descriptor *p_descriptor =
						(struct registration_descriptor *) p_descr;
				pos = parse_registration_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x06:
			{
				struct data_stream_alignment_descriptor *p_descriptor =
						(struct data_stream_alignment_descriptor *) p_descr;
				pos = parse_data_stream_alignment_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x07:
			{
				struct target_background_grid_descriptor *p_descriptor =
						(struct target_background_grid_descriptor *) p_descr;
				pos = parse_target_background_grid_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x08:
			{
				struct video_window_descriptor *p_descriptor =
						(struct video_window_descriptor *) p_descr;
				pos = parse_video_window_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x09:	/*	CA descriptor		*/
			{
				struct ca_descriptor *p_descriptor = (struct ca_descriptor *) p_descr;
				pos = parse_ca_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;


		case 0x0a:
			{
				struct iso_639_language_descriptor *p_descriptor =
						(struct iso_639_language_descriptor *) p_descr;
				pos = parse_iso_639_language_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x0b:
			{
				struct system_clock_descriptor *p_descriptor =
						(struct system_clock_descriptor *) p_descr;
				pos = parse_system_clock_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x0c:
			{
				struct multiplex_buffer_utilization_descriptor *p_descriptor =
						(struct multiplex_buffer_utilization_descriptor *) p_descr;
				pos = parse_multiplex_buffer_utilization_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x0d:
			{
				struct copyright_descriptor *p_descriptor =
						(struct copyright_descriptor *) p_descr;
				pos = parse_copyright_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x0e:
			{
				struct maximum_bitrate_descriptor *p_descriptor =
						(struct maximum_bitrate_descriptor *) p_descr;
				pos = parse_maximum_bitrate_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x0f:
			{
				struct private_data_indicator_descriptor *p_descriptor =
						(struct private_data_indicator_descriptor *) p_descr;
				pos = parse_private_data_indicator_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x10:
			{
				struct smoothing_buffer_descriptor *p_descriptor =
						(struct smoothing_buffer_descriptor *) p_descr;
				pos = parse_smoothing_buffer_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x11:
			{
				struct std_descriptor *p_descriptor = (struct std_descriptor *) p_descr;
				pos = parse_std_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x12:
			{
				struct ibp_descriptor *p_descriptor = (struct ibp_descriptor *) p_descr;
				pos = parse_ibp_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x1b:
			{
				struct mpeg_4_video_descriptor *p_descriptor =
						(struct mpeg_4_video_descriptor *) p_descr;
				pos = parse_mpeg_4_video_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x1c:
			{
				struct mpeg_4_audio_descriptor *p_descriptor =
						(struct mpeg_4_audio_descriptor *) p_descr;
				pos = parse_mpeg_4_audio_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x1d:
			{
				struct iod_descriptor *p_descriptor = (struct iod_descriptor *) p_descr;
				pos = parse_iod_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x1e:
			{
				struct sl_descriptor *p_descriptor = (struct sl_descriptor *) p_descr;
				pos = parse_sl_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x1f:
			{
				struct fmc_descriptor *p_descriptor = (struct fmc_descriptor *) p_descr;
				pos = parse_fmc_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x20:
			{
				struct external_es_id_descriptor *p_descriptor =
						(struct external_es_id_descriptor *) p_descr;
				pos = parse_external_es_id_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x21:
			{
				struct muxcode_descriptor *p_descriptor =
						(struct muxcode_descriptor *) p_descr;
				pos = parse_muxcode_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x22:
			{
				struct fmxbuffer_size_descriptor *p_descriptor =
						(struct fmxbuffer_size_descriptor *) p_descr;
				pos = parse_fmxbuffer_size_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x23:
			{
				struct multiplex_buffer_descriptor *p_descriptor =
						(struct multiplex_buffer_descriptor *) p_descr;
				pos = parse_multiplex_buffer_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x40:
			{
				struct network_name_descriptor *p_descriptor =
						(struct network_name_descriptor *) p_descr;
				pos = parse_network_name_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x41:
			{
				struct service_list_descriptor *p_descriptor =
						(struct service_list_descriptor *) p_descr;
				pos = parse_service_list_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x42:
			{
				struct stuffing_descriptor *p_descriptor =
						(struct stuffing_descriptor *) p_descr;
				pos = parse_stuffing_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x43:
			{
				struct satellite_delivery_system *p_descriptor =
						(struct satellite_delivery_system *) p_descr;
				pos = parse_satellite_delivery_system(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x44:
			{
				struct cable_delivery_system *p_descriptor =
						(struct cable_delivery_system *) p_descr;
				pos = parse_cable_delivery_system(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x47:
			{
				struct bouquet_name_descriptor *p_descriptor =
						(struct bouquet_name_descriptor *) p_descr;
				pos = parse_bouquet_name_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x48:
			{
				struct service_descriptor *p_descriptor =
						(struct service_descriptor *) p_descr;
				pos = parse_service_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x49:
			{
				struct country_availability_descriptor *p_descriptor =
						(struct country_availability_descriptor *) p_descr;
				pos = parse_country_availability_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x4a:
			{
				struct linkage_descriptor *p_descriptor =
						(struct linkage_descriptor *) p_descr;
				pos = parse_linkage_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x4b:
			{
				struct nvod_reference_descriptor *p_descriptor =
						(struct nvod_reference_descriptor *) p_descr;
				pos = parse_nvod_reference_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x4c:
			{
				struct time_shifted_service_descriptor *p_descriptor =
						(struct time_shifted_service_descriptor *) p_descr;
				pos = parse_time_shifted_service_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x4d:
			{
				struct short_event_descriptor *p_descriptor =
						(struct short_event_descriptor *) p_descr;
				pos = parse_short_event_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x4e:
			{
				struct extended_event_descriptor *p_descriptor =
						(struct extended_event_descriptor *) p_descr;
				pos = parse_extended_event_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x4f:
			{
				struct time_shifted_event_descriptor *p_descriptor =
						(struct time_shifted_event_descriptor *) p_descr;
				pos = parse_time_shifted_event_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x50:
			{
				struct component_descriptor *p_descriptor =
						(struct component_descriptor *) p_descr;
				pos = parse_component_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x51:
			{
				struct mosaic_descriptor *p_descriptor =
						(struct mosaic_descriptor *) p_descr;
				pos = parse_mosaic_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x52:
			{
				struct stream_identifier_descriptor *p_descriptor =
						(struct stream_identifier_descriptor *) p_descr;
				pos = parse_stream_identifier_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x53:
			{
				struct ca_identifier_descriptor *p_descriptor =
						(struct ca_identifier_descriptor *) p_descr;
				pos = parse_ca_identifier_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x54:
			{
				struct content_descriptor *p_descriptor =
						(struct content_descriptor *) p_descr;
				pos = parse_content_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x55:
			{
				struct parental_rating_descriptor *p_descriptor =
						(struct parental_rating_descriptor *) p_descr;
				pos = parse_parental_rating_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x56:
			{
				struct teletext_descriptor *p_descriptor =
						(struct teletext_descriptor *) p_descr;
				pos = parse_teletext_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x57:
			{
				struct telephone_descriptor *p_descriptor =
						(struct telephone_descriptor *) p_descr;
				pos = parse_telephone_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x58:
			{
				struct local_time_offset_descriptor *p_descriptor =
						(struct local_time_offset_descriptor *) p_descr;
				pos = parse_local_time_offset_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x59:
			{
				struct subtitling_descriptor *p_descriptor =
						(struct subtitling_descriptor *) p_descr;
				pos = parse_subtitling_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x5a:
			{
				struct terrestrial_delivery_system *p_descriptor =
						(struct terrestrial_delivery_system *) p_descr;
				pos = parse_terrestrial_delivery_system(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x5b:
			{
				struct multilingual_network_name_descriptor *p_descriptor =
						(struct multilingual_network_name_descriptor *) p_descr;
				pos = parse_multilingual_network_name_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x5c:
			{
				struct multilingual_bouquet_name_descriptor *p_descriptor =
						(struct multilingual_bouquet_name_descriptor *) p_descr;
				pos = parse_multilingual_bouquet_name_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;
/*
		case 0x5d:
			{
				struct multilingual_service_name_descriptor *p_descriptor =
						(struct multilingual_service_name_descriptor *) p_descr;
				pos = parse_multilingual_service_name_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x5e:
			{
				struct multilingual_component_descriptor *p_descriptor =
						(struct multilingual_component_descriptor *) p_descr;
				pos = parse_multilingual_component_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x5f:
			{
				struct private_data_specifier_descriptor *p_descriptor =
						(struct private_data_specifier_descriptor *) p_descr;
				pos = parse_private_data_specifier_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x60:
			{
				struct service_move_descriptor *p_descriptor =
						(struct service_move_descriptor *) p_descr;
				pos = parse_service_move_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x61:
			{
				struct short_smoothing_buffer_descriptor *p_descriptor =
						(struct short_smoothing_buffer_descriptor *) p_descr;
				pos = parse_short_smoothing_buffer_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x62:
			{
				struct frequency_list_descriptor *p_descriptor =
						(struct frequency_list_descriptor *) p_descr;
				pos = parsse_frequency_list_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x63:
				p_descriptor = (struct partial_transport_stream_descriptor *)
					malloc(sizeof (struct partial_transport_stream_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x64:
			{
				struct data_broadcast_descriptor *p_descriptor =
						(struct data_broadcast_descriptor *) p_descr;
				pos = parse_data_broadcast_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;

		case 0x65:
				p_descriptor = (struct ca_system_descriptor *) malloc(sizeof (struct ca_system_descriptor) * MAX_DESCRIPTOR);
			break;

		case 0x66:
			{
				struct data_broadcast_id_descriptor *p_descriptor =
						(struct data_broadcast_id_descriptor *) p_descr;
				pos = parse_data_broadcast_id_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;
*/
		default:	/*	parse an unknown	*/
				pos += p_descriptor_info->descriptor_length;
				break;
	}

	return pos;
}
