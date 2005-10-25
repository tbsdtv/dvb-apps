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

uint8_t parse_descriptor_header(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];

	return pos + 2;
}

uint16_t parse_multilingual_bouquet_name_descriptor(struct descriptor *p_descriptor, uint8_t * buf, uint16_t pos)
{
	uint8_t i, j;
	struct multilingual_bouquet_name *p_multilingual_bouquet_name = NULL;
	uint8_t *p_char = NULL;

	printf("--->%s: Parsing Multilingual Bouqet Name Descriptor\n", __FUNCTION__);
	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_multilingual_bouquet_name = (struct multilingual_bouquet_name *)
		malloc(sizeof (struct multilingual_bouquet_name) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i += 4) {
		p_multilingual_bouquet_name[i].iso_639_language_code =
			(((buf[pos + i] << 8) | buf[pos + 1 + i]) << 8) | buf[pos + 2 + i];
		p_multilingual_bouquet_name[i].bouquet_name_length = buf[pos + 3 + i];

		p_char = (uint8_t *) malloc(sizeof (uint8_t) * (p_multilingual_bouquet_name[i].bouquet_name_length));
		for (j = 0; j < p_multilingual_bouquet_name[i].bouquet_name_length; j++)
			p_char[j] = buf[pos + 4 + i + j];
		i += j;
		p_multilingual_bouquet_name[i].p_char = p_char;
	}
	p_descriptor->multilingual_bouquet_name.p_multilingual_bouquet_name = p_multilingual_bouquet_name;
	pos += 5 + i;

	return pos;
}

void deallocate_multilingual_bouquet_name_descriptor(struct descriptor *p_descriptor)
{
	uint8_t i;

	if (p_descriptor->descriptor_length) {
		for (i = 0; i < p_descriptor->descriptor_length; i++) {
			if (p_descriptor->multilingual_bouquet_name.p_multilingual_bouquet_name[i].bouquet_name_length)
				free(p_descriptor->multilingual_bouquet_name.p_multilingual_bouquet_name[i].p_char);
		}
		free(p_descriptor->multilingual_bouquet_name.p_multilingual_bouquet_name);
	}
}

uint16_t parse_multilingual_network_name_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint8_t i, j;
	struct multilingual_network_name *p_multilingual_network_name = NULL;
	uint8_t *p_char = NULL;

	printf("--->%s: Parsing Multilingual Network Name Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_multilingual_network_name = (struct multilingual_network_name *)
		malloc(sizeof (struct multilingual_network_name) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i += 4) {
		p_multilingual_network_name[i].iso_639_language_code =
			(((buf[pos + i] << 8) | buf[pos + 1 + i]) << 8) | buf[pos + 2 + i];
		p_multilingual_network_name[i].network_name_length = buf[pos + 3 + i];

		p_char = (uint8_t *) malloc(sizeof (uint8_t) * p_multilingual_network_name[i].network_name_length);
		for (j = 0; j < p_multilingual_network_name[i].network_name_length; j++)
			p_char[j] = buf[pos + 4 + i + j];
		i += j;
		p_multilingual_network_name[i].p_char = p_char;
	}
	p_descriptor->multilingual_network_name.p_multilingual_network_name = p_multilingual_network_name;
	pos += 5 + i;

	return pos;
}

void deallocate_multilingual_network_name_descriptor(struct descriptor *p_descriptor)
{
	uint8_t i;

	if (p_descriptor->descriptor_length) {
		for (i = 0; i < p_descriptor->descriptor_length; i++) {
			if (p_descriptor->multilingual_network_name.p_multilingual_network_name[i].network_name_length)
				free(p_descriptor->multilingual_network_name.p_multilingual_network_name[i].p_char);
		}
		free(p_descriptor->multilingual_network_name.p_multilingual_network_name);
	}
}


uint16_t parse_terrestrial_delivery_system
	(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Terrestrial Delivery System Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->terrestrial_delivery_system.centre_frequency =
		(((((buf[pos] << 8 ) | buf[pos + 1]) << 8) | buf[pos + 2]) << 8) | buf[pos + 3];
	p_descriptor->terrestrial_delivery_system.bandwidth = buf[pos +4] >> 5;
	p_descriptor->terrestrial_delivery_system.reserved_1 = buf[pos + 4] & 0x1f;
	p_descriptor->terrestrial_delivery_system.constellation = buf[pos + 5] >> 6;
	p_descriptor->terrestrial_delivery_system.hierarchy_information = (buf[pos + 5] >> 3) & 0x07;
	p_descriptor->terrestrial_delivery_system.code_rate_hp_stream = buf[pos + 5] & 0x07;
	p_descriptor->terrestrial_delivery_system.code_rate_lp_stream = buf[pos + 6] >> 5;
	p_descriptor->terrestrial_delivery_system.guard_interval = (buf[pos + 6] >> 3) & 0x03;
	p_descriptor->terrestrial_delivery_system.transmission_mode = (buf[pos + 6] >> 1) & 0x03;
	p_descriptor->terrestrial_delivery_system.other_frequency_flag = buf[pos + 6] & 0x01;
	p_descriptor->terrestrial_delivery_system.reserved_2 =
		(((((buf[pos + 7] << 8) | buf[pos + 8]) << 8) | buf[pos + 9]) << 8) | buf[pos + 10];

	pos += 11;

	return pos;
}

void deallocate_terrestrial_delivery_system(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_subtitling_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct subtitle *p_subtitle = NULL;

	printf("--->%s: Parsing Subtitling Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_subtitle = (struct subtitle *)
		malloc(sizeof (struct subtitle) * (p_descriptor->descriptor_length / 64));
	for (i = 0; i < p_descriptor->descriptor_length; i += 64) {
		p_subtitle[i].iso_639_language_code =
			(((buf[pos] << 8) | buf[pos + 1]) << 8) | buf[pos + 2];
		p_subtitle[i].subtitling_type = buf[pos + 3];
		p_subtitle[i].composition_page_id = (buf[pos + 4] << 8) | buf[pos + 5];
		p_subtitle[i].ancillary_page_id = (buf[pos + 6] << 8) | buf[pos + 7];
	}
	p_descriptor->subtitling.p_subtitle = p_subtitle;
	pos += 8;

	return pos;
}

void deallocate_subtitling_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->subtitling.p_subtitle);
}

uint16_t parse_local_time_offset_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct local_time_offset *p_local_time_offset = NULL;

	printf("--->%s: Parsing Local Time Offset Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_local_time_offset = (struct local_time_offset *)
		malloc(sizeof (struct local_time_offset) * (p_descriptor->descriptor_length / 13) );
	for (i = 0; i < p_descriptor->descriptor_length; i += 13) {
		p_local_time_offset[i].country_code =
			(((buf[pos] << 8) | buf[pos + 1]) << 8) | buf[pos + 2];
		p_local_time_offset[i].country_region_id = buf[pos + 3] >> 2;
		p_local_time_offset[i].reserved = (buf[pos + 3] >> 1) & 0x01;
		p_local_time_offset[i].local_time_offset_polarity = buf[pos + 3] & 0x01;
		p_local_time_offset[i].local_time_offset = (buf[pos + 4] << 8) | buf[pos + 5];
		p_local_time_offset[i].time_of_change =
			(((((((buf[pos + 6] << 8) | buf[pos + 7]) << 8) | buf[pos + 8]) << 8) |
					buf[pos + 9]) << 8) | buf[pos + 10];
		p_local_time_offset[i].next_time_offset = (buf[pos + 11] << 8) | buf[pos + 12];
	}
	p_descriptor->local_time_offset.p_local_time_offset = p_local_time_offset;
	pos += 13;

	return pos;
}

void deallocate_local_time_offset_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->local_time_offset.p_local_time_offset);
}

uint16_t parse_telephone_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_country_prefix = NULL;
	uint8_t *p_international_area_code = NULL;
	uint8_t *p_operator_code = NULL;
	uint8_t *p_national_area_code = NULL;
	uint8_t *p_core_number = NULL;

	printf("--->%s: Parsing Telephone Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->telephone.reserved_1 = buf[pos] >> 6;
	p_descriptor->telephone.foreign_availability = (buf[pos] >> 5) & 0x01;
	p_descriptor->telephone.connection_type = buf[pos] & 0x1f;
	p_descriptor->telephone.reserved_2 = buf[pos + 1] >> 7;
	p_descriptor->telephone.country_prefix_length = (buf[pos + 1] >> 5) & 0x03;
	p_descriptor->telephone.international_area_code_length = (buf[pos + 1] >> 2) & 0x07;
	p_descriptor->telephone.operator_code_length = buf[pos + 1] & 0x03;
	p_descriptor->telephone.reserved_3 = buf[pos + 2] >> 7;
	p_descriptor->telephone.national_area_code_length = (buf[pos + 2] >> 4) & 0x07;
	p_descriptor->telephone.core_number_length = buf[pos + 2] & 0x0f;

	p_country_prefix = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->telephone.country_prefix_length);
	for (i = 0; i < p_descriptor->telephone.country_prefix_length; i++)
		p_country_prefix[i] = buf[pos + 3 + i];
	p_descriptor->telephone.p_country_prefix = p_country_prefix;
	pos += 4 + i;

	p_international_area_code = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->telephone.international_area_code_length);
	for (i = 0; i < p_descriptor->telephone.international_area_code_length; i++)
		p_international_area_code[i] = buf[pos + i];
	p_descriptor->telephone.p_international_area_code = p_international_area_code;
	pos += i;

	p_operator_code = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->telephone.operator_code_length);
	for (i = 0; i < p_descriptor->telephone.operator_code_length; i++)
		p_operator_code[i] = buf[pos + i];
	p_descriptor->telephone.p_operator_code = p_operator_code;
	pos += i;

	p_national_area_code = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->telephone.national_area_code_length);
	for (i = 0; i < p_descriptor->telephone.national_area_code_length; i++)
		p_national_area_code[i] = buf[pos + i];
	p_descriptor->telephone.p_national_area_code = p_national_area_code;
	pos += i;

	p_core_number = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->telephone.core_number_length);
	for (i = 0; i < p_descriptor->telephone.core_number_length; i++)
		p_core_number[i] = buf[pos + i];
	p_descriptor->telephone.p_core_number = p_core_number;
	pos += i;

	return pos;
}

void deallocate_telephone_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->telephone.country_prefix_length)
		free(p_descriptor->telephone.p_country_prefix);
	if (p_descriptor->telephone.international_area_code_length)
		free(p_descriptor->telephone.p_international_area_code);
	if (p_descriptor->telephone.operator_code_length)
		free(p_descriptor->telephone.p_operator_code);
	if (p_descriptor->telephone.national_area_code_length)
		free(p_descriptor->telephone.p_national_area_code);
	if (p_descriptor->telephone.core_number_length)
		free(p_descriptor->telephone.p_core_number);
}

uint16_t parse_teletext_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct teletext *p_teletext = NULL;

	printf("--->%s: Parsing Teletext Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_teletext = (struct teletext *)
		malloc(sizeof (struct teletext) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i += 5) {
		p_teletext[i].iso_639_language_code =
			(((buf[pos + i] << 8) | buf[pos + 1 + i]) << 8) | buf[pos + 2 + i];
		p_teletext[i].teletext_type = buf[pos + 3 + i] >> 3;
		p_teletext[i].teletext_magazine_number = buf[pos + 3 + i] & 0x07;
		p_teletext[i].teletext_page_number = buf[pos + 4 + i];
	}
	p_descriptor->teletext.p_teletext = p_teletext;
	pos += p_descriptor->descriptor_length;

	return pos;
}

void deallocate_teletext_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->teletext.p_teletext);
}

uint16_t parse_parental_rating_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct parental_rating *p_parental_rating = NULL;

	printf("--->%s: Parsing Parental Rating Descriptor Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_parental_rating = (struct parental_rating *)
		malloc(sizeof (struct parental_rating) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i+= 4) {
		p_parental_rating[i].country_code =
			(((buf[pos + i] << 8) | buf[pos + 1 + i]) << 8) | buf[pos + 2 + 1];
		p_parental_rating[i].rating = buf[pos + 3 + i];
	}
	p_descriptor->parental_rating.p_parental_rating = p_parental_rating;
	pos += 4 + i;

	return pos;
}

void deallocate_parental_rating_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->parental_rating.p_parental_rating);
}

uint16_t parse_content_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct content *p_content = NULL;

	printf("--->%s: Parsing Content Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_content = (struct content *)
		malloc(sizeof (struct content) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i += 2) {
		p_content[i].content_nibble_level_1 = buf[pos + i] >> 4;
		p_content[i].content_nibble_level_2 = buf[pos + i] & 0x0f;
		p_content[i].user_nibble_1 = buf[pos + 1 + i] >> 4;
		p_content[i].user_nibble_2 = buf[pos + 1 + i] & 0x0f;
	}
	p_descriptor->content.p_content = p_content;
	pos += 2 + i;

	return pos;
}

void deallocate_content_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->content.p_content);
}

uint16_t parse_component_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_text_char = NULL;

	printf("--->%s: Parsing Component Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->component.reserved = buf[pos] >> 4;
	p_descriptor->component.stream_content = buf[pos] & 0x0f;
	p_descriptor->component.component_type = buf[pos + 1];
	p_descriptor->component.component_tag = buf[pos + 2];
	p_descriptor->component.iso_639_language_code =
		(((buf[pos + 3] << 8) | buf[pos + 4]) << 8) | buf[pos + 5];

	p_text_char = (uint8_t *)
		malloc(sizeof (uint8_t) * (p_descriptor->descriptor_length - 6) );
	for (i = 0; i < (p_descriptor->descriptor_length - 6); i++)
		p_text_char[i] = buf[pos + 6 + i];
	p_descriptor->component.p_text_char = p_text_char;
	pos += 7 + i;

	return pos;
}

void deallocate_component_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->component.p_text_char);
}

uint16_t parse_time_shifted_event_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Time Shifted Event Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->time_shifted_event.reference_service_id = (buf[pos] << 8) | buf[pos + 1];
	p_descriptor->time_shifted_event.reference_event_id = (buf[pos + 2] << 8) | buf[pos + 3];

	pos += 4;

	return pos;
}

void deallocate_time_shifted_event_descriptor(struct descriptor *p_descriptor)
{
	return;
}

// TODO: This one needs some more looking into!!!
uint16_t parse_extended_event_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i, j;

	struct items *p_items = NULL;
	uint8_t *p_item_description = NULL;
	uint8_t *p_text_char = NULL;
	uint8_t *p_text = NULL;

	printf("--->%s: Parsing Extended Event Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->extended_event.descriptor_number = buf[pos] >> 4;
	p_descriptor->extended_event.last_descriptor_number = buf[pos] & 0x0f;
	p_descriptor->extended_event.iso_639_language_code =
		(((buf[pos + 1] << 8) | buf[pos + 2]) << 8) | buf[pos + 3];
	p_descriptor->extended_event.length_of_items = buf[pos + 4];

	p_items = (struct items *)
		malloc(sizeof (struct items) * p_descriptor->extended_event.length_of_items);
	for (i = 0; i < p_descriptor->extended_event.length_of_items; i++) {

		p_items[i].item_description_length = buf[pos + 5 + i];
		p_item_description = (uint8_t *)
			malloc(sizeof (uint8_t) * p_items[i].item_description_length);
		for (j = 0; j < p_items[i].item_description_length; j++)
			p_item_description[j] = buf[pos + 5 + i + j];
		p_items[i].p_item_description = p_item_description;
		pos += j;

		p_items[i].item_length = buf[pos + 6 + i]; // since j is already added
		p_text_char = (uint8_t *)
			malloc(sizeof (uint8_t) * p_items[i].item_length);
		for (j = 0; j < p_items[i].item_length; j++)
			p_text_char[j] = buf[pos + 7 + i + j];
		p_items[i].p_item_char = p_text_char;
		pos += j;
	}
	p_descriptor->extended_event.p_items = p_items;
	pos += 8 + i;

	p_descriptor->extended_event.text_length = buf[pos + 0];
	p_text = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->extended_event.text_length);

	for (i = 0; i < p_descriptor->extended_event.text_length; i++)
		p_text[i] = buf[pos + 1 + i];
	p_descriptor->extended_event.p_text_char = p_text;
	pos += 2 + i;


	return pos;
}

void deallocate_extended_event_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->extended_event.length_of_items)
		free(p_descriptor->extended_event.p_items);
	if (p_descriptor->extended_event.text_length)
		free(p_descriptor->extended_event.p_text_char);
}

uint16_t parse_short_event_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_event = NULL;
	uint8_t *p_char = NULL;

	printf("--->%s: Parsing Short Event Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->short_event.iso_639_language_code =
		(((buf[pos] << 8) | buf[pos + 1]) << 8) | buf[pos + 2];
	p_descriptor->short_event.event_name_length = buf[pos + 3];

	p_event = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->short_event.event_name_length);
	for (i = 0; i < p_descriptor->short_event.event_name_length; i++)
		p_event[i] = buf[pos + 4 + i];

	p_descriptor->short_event.p_event_name = p_event;
	pos += 5 + i;

	p_descriptor->short_event.text_length = buf[pos + 0];
	p_char = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->short_event.text_length);
	for (i = 0; i < p_descriptor->short_event.text_length; i++)
		p_char[i] = buf[pos + 1 + i];

	p_descriptor->short_event.p_text_char = p_char;
	pos += 2 + i;

	return pos;
}

void deallocate_short_event_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->short_event.event_name_length)
		free(p_descriptor->short_event.p_event_name);
	if (p_descriptor->short_event.text_length)
		free(p_descriptor->short_event.p_text_char);
}

uint16_t parse_time_shifted_service_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Time Shifted Service Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->time_shifted_service.reference_service_id =
		(buf[pos + 0] << 8) | buf[pos + 1];

	pos += 2;

	return pos;
}

void deallocate_time_shifted_service_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_nvod_reference_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct nvod_reference *p_reference = NULL;

	printf("--->%s: Parsing Nvod Reference Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_reference = (struct nvod_reference *)
		malloc(sizeof (struct nvod_reference) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i += 8) {
		p_reference[i].transport_stream_id =
			(buf[pos + 0 + i] << 8) | buf[pos + 1 + i];
		p_reference[i].original_network_id =
			(buf[pos + 2 + i] << 8) | buf[pos + 3 + i];
		p_reference[i].service_id =
			(buf[pos + 4 + i] << 8) | buf[pos + 5 + i];
	}

	p_descriptor->nvod_reference.p_nvod_reference = p_reference;
	pos += 6 + i;

	return pos;
}

void deallocate_nvod_reference_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->nvod_reference.p_nvod_reference);
}

uint16_t parse_linkage_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Linkage Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->linkage.transport_stream_id = (buf[pos + 0] << 8) | buf[pos + 1];
	p_descriptor->linkage.original_network_id = (buf[pos + 2] << 8) | buf[pos + 3];
	p_descriptor->linkage.service_id =
		((p_descriptor->linkage.service_id | buf[pos + 4]) << 8) | buf[pos + 5];
	p_descriptor->linkage.linkage_type = buf[pos + 6];

	pos += 7;

	return pos;
}

void deallocate_linkage_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_country_availability_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct country_loop *p_country = NULL;

	printf("--->%s: Parsing Country Availability Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->country_availability.country_availability_flag = buf[pos + 0] >> 7;
	p_descriptor->country_availability.reserved = buf[pos + 0] & 0x7f;

	p_country = (struct country_loop *)
		malloc(sizeof (struct country_loop) * (p_descriptor->descriptor_length - 1) );
	for (i = 0; i < (p_descriptor->descriptor_length - 1); i += 3)
		p_country[i].country_code =
			(((buf[pos + 1 + i] << 8) | buf[pos + 2 + i]) << 8) | buf[pos + 3 + i];

	p_descriptor->country_availability.p_country_loop = p_country;
	pos += 4 + i;

	return pos;
}

void deallocate_country_availability_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->country_availability.p_country_loop);
}

uint16_t parse_service_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_provider_name = NULL;
	uint8_t *p_service = NULL;

	printf("--->%s: Parsing Service Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->service.service_type = buf[pos + 0];
	p_descriptor->service.service_provider_name_length = buf[pos + 1];

	p_provider_name = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->service.service_provider_name_length);
	for (i = 0; i < p_descriptor->service.service_provider_name_length; i++)
		p_provider_name[i] = buf[pos + 2 + i];

	p_descriptor->service.p_service_provider_name = p_provider_name;
	pos += 3;
	p_descriptor->service.service_name_length = buf[pos + 0];
	p_service = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->service.service_name_length);
	for (i = 0; i < p_descriptor->service.service_name_length; i++)
		p_service[i] = buf[pos + 1 + i];
	p_descriptor->service.p_service_name = p_service;
	pos += 2;

	return pos;
}

void deallocate_service_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->service.service_provider_name_length)
		free(p_descriptor->service.p_service_provider_name);
	if (p_descriptor->service.service_name_length)
		free(p_descriptor->service.p_service_name);
}

uint16_t parse_bouquet_name_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_bouquet = NULL;

	printf("--->%s: Parsing Boquet Name Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_bouquet = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i++)
		p_bouquet[i] = buf[pos + 0 + i];

	p_descriptor->bouquet_name.p_bouquet_name = p_bouquet;
	pos += 1 + i;

	return pos;
}

void deallocate_bouquet_name_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->bouquet_name.p_bouquet_name);
}

uint16_t parse_cable_delivery_system(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Cable Delivery System Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->cable_delivery_system.frequency =
		(((((buf[pos + 0] << 8) | buf[pos + 1]) << 8) | buf[pos + 2]) << 8) |
				buf[pos + 3];
	p_descriptor->cable_delivery_system.reserved = (buf[pos + 4] << 4) | (buf[pos + 5] >> 4);
	p_descriptor->cable_delivery_system.fec_outer = buf[pos + 6] & 0x0f;
	p_descriptor->cable_delivery_system.modulation = buf[pos + 7];
	p_descriptor->cable_delivery_system.symbol_rate =
		(((((buf[pos + 8] << 8) | buf[pos + 9]) << 8) | buf[pos + 10]) << 8) |
				(buf[pos + 11] >> 4);
	p_descriptor->cable_delivery_system.fec_inner = buf[pos + 12] & 0x0f;

	pos += 13;

	return pos;
}

void deallocate_cable_delivery_system(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_satellite_delivery_system(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Sattelite Delivery System Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->satellite_delivery_system.frequency =
		((((buf[pos + 0] << 8) | buf[pos + 1]) << 8) | buf[pos + 2] << 8) |
				buf[pos + 3];
	p_descriptor->satellite_delivery_system.orbital_position = (buf[pos + 4] << 8) | buf[pos + 5];
	p_descriptor->satellite_delivery_system.west_east_flag = buf[pos + 6] >> 7;
	p_descriptor->satellite_delivery_system.polarization = (buf[pos + 6] >> 5) & 0x03;
	p_descriptor->satellite_delivery_system.modulation = buf[pos + 6] & 0x1f;
	p_descriptor->satellite_delivery_system.symbol_rate =
		(((((buf[pos + 7] << 8) | buf[pos + 8]) << 8) | buf[pos + 9]) << 6) |
				(buf[pos + 10] >> 2);
	p_descriptor->satellite_delivery_system.fec_inner = buf[pos + 10] & 0x0f;

	pos += 11;

	return pos;
}

void deallocate_satellite_delivery_system(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_stuffing_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_stuffing_words = NULL;

	printf("--->%s: Parsing Stuffing Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_stuffing_words = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->descriptor_length);

	for (i = 0; i < p_descriptor->descriptor_length; i++)
		p_stuffing_words[i] = buf[pos + 0 + i];

	p_descriptor->stuffing.p_stuffing_bytes = p_stuffing_words;

	pos += 1 + i;

	return pos;
}

void deallocate_stuffing_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->stuffing.p_stuffing_bytes);
}

uint16_t parse_service_list_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct service_list *p_services = NULL;

	printf("--->%s: Parsing Service List Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_services = (struct service_list *)
		malloc(sizeof (struct service_list) * (p_descriptor->descriptor_length / 3));

	for (i = 0; i < p_descriptor->descriptor_length; i += 3) {
		p_services[i].service_id = (buf[pos + 0 + i] << 8) | buf[pos + 1 + i];
		p_services[i].service_type = buf[pos + 2 + i];
	}

	p_descriptor->service_list.p_service_list = p_services;

	pos += 2 + i;

	return pos;
}

void deallocate_service_list_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->service_list.p_service_list);
}

uint16_t parse_network_name_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint8_t *p_name = NULL;

	printf("--->%s: Parsing Network Name Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_name = (uint8_t *)
		malloc(sizeof (uint8_t) * p_descriptor->descriptor_length);

	for (i = 0; i < p_descriptor->descriptor_length; i++)
		p_name[i] = buf[pos + 0 + i];

	p_descriptor->network_name.p_network_name = p_name;

	pos += i;

	return pos;
}

void deallocate_network_name_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->network_name.p_network_name);
}

uint16_t parse_multiplex_buffer_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Multiplex Buffer  Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->multiplex_buffer.mb_buffer_size =
		(((buf[pos + 0] << 8) | buf[pos + 1]) << 8) | buf[pos + 2];
	p_descriptor->multiplex_buffer.tb_leak_rate =
		(((buf[pos + 3] << 8) | buf[pos + 4]) << 8) | buf[pos + 5];

	pos += 6;

	return pos;
}

void deallocate_multiplex_buffer_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_fmxbuffer_size_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	/*	need ISO/IEC 14496-1	*/
	return pos;
}

void deallocate_fmxbuffer_size_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_muxcode_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	/*	need ISO/IEC 14496-1	*/
	return pos;
}

void deallocate_muxcode_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_external_es_id_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing ES ID Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->external_es_id.external_es_id = (buf[pos + 0] << 8) | buf[pos + 1];

	pos += 2;

	return pos;
}

void deallocate_external_es_id_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_fmc_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	struct flex_mux *p_flexmux = NULL;

	printf("--->%s: Parsing FMC Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_flexmux = (struct flex_mux *)
		malloc(sizeof (struct flex_mux) * (p_descriptor->descriptor_length / 3));
	for (i = 0; i < p_descriptor->descriptor_length; i += 3) {
		p_descriptor->fmc.p_flex_mux[i].es_id =
			(buf[pos + 0 + i] << 8) | buf[pos + 1 + i];
		p_descriptor->fmc.p_flex_mux[i].flex_mux_channel = buf[pos + 2 + i];
	}
	p_descriptor->fmc.p_flex_mux = p_flexmux;

	pos += 3 + i;

	return pos;
}

void deallocate_fmc_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->fmc.p_flex_mux);
}

uint16_t parse_sl_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	/*	need ISO/IEC 14496-1	*/

	return pos;
}

void deallocate_sl_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_iod_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	/*	need ISO/IEC 14496-1	*/

	return pos;
}

void deallocate_iod_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_mpeg_4_audio_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing MPEG-4 Audio Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->mpeg_4_audio.mpeg_4_audio_profile_and_level = buf[pos + 0];

	pos += 1;

	return pos;
}

void deallocate_mpeg_4_audio_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_mpeg_4_video_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing MPEG-4 Video Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->mpeg_4_video.mpeg_4_visual_profile_and_level = buf[pos + 0];

	pos += 1;

	return pos;
}

void deallocate_mpeg_4_video_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_ibp_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing IBP Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->ibp.closed_gop_flag = buf[pos + 0] >> 7;
	p_descriptor->ibp.identical_gop_flag = (buf[pos + 0] >> 6) & 0x01;
	p_descriptor->ibp.max_gop_length =
		((buf[pos + 0] & 0x3f) << 8) | buf[pos + 1];

	pos += 3;

	return pos;
}

void deallocate_ibp_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_std_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing STD Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->std.reserved = buf[pos + 0] >> 1;
	p_descriptor->std.leak_valid_flag = buf[pos + 0] & 0x01;

	pos += 1;

	return pos;
}

void deallocate_std_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_smoothing_buffer_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Smoothing Buffer Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->smoothing_buffer.reserved_1 = buf[pos + 0] >> 6;
	p_descriptor->smoothing_buffer.sb_leak_rate =
		((((buf[pos + 0] & 0x3f) << 8) | buf[pos + 1]) << 8) | buf[pos + 2];
	p_descriptor->smoothing_buffer.reserved_2 = buf[pos + 3] >> 6;
	p_descriptor->smoothing_buffer.sb_size =
		((((buf[pos + 3] & 0x3f) << 8) | buf[pos + 4]) << 8) | buf[pos + 5];

	pos += 6;

	return pos;
}

void deallocate_smoothing_buffer_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_private_data_indicator_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Private Data Indicator Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->private_data_indicator.private_data_indicator =
		(((((buf[pos + 0] << 8) | buf[pos + 1]) << 8) | buf[pos + 2]) << 8) |
				buf[pos + 3];

	pos += 4;

	return pos;
}

void deallocate_private_data_indicator_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_maximum_bitrate_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Maximum Bitrate Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->maximum_bitrate.reserved = buf[pos + 0] >> 6;
	p_descriptor->maximum_bitrate.maximum_bitrate =
		((((buf[pos + 0] & 0x3f) << 8) | buf[pos + 1]) << 8) | buf[pos + 2];

	pos += 3;

	return pos;
}

void deallocate_maximum_bitrate_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_copyright_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint8_t i;
	uint8_t *p_additional_copyright_info = NULL;

	printf("--->%s: Parsing Copyright Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->copyright.copyright_identifier =
		(((((buf[pos + 0] << 8) | buf[pos + 1]) << 8) | buf[pos + 2]) << 8) |
					buf[pos + 3];

	p_additional_copyright_info = (uint8_t *)
		malloc(sizeof (uint8_t) * (p_descriptor->descriptor_length - 4) );
	for (i = 0; i < (p_descriptor->descriptor_length - 4); i++)
	//p_descriptor->copyright.p_additional_copyright_info[i] = buf[pos + i + 3];
		p_additional_copyright_info[i] = buf[pos + i + 3];
	p_descriptor->copyright.p_additional_copyright_info = p_additional_copyright_info;

	pos += i + 4;

	return pos;
}

void deallocate_copyright_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length - 4)
		free(p_descriptor->copyright.p_additional_copyright_info);
}

uint16_t parse_multiplex_buffer_utilization_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Multiplex Buffer Utilization Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->multiplex_buffer_utilization.bound_valid_flag = buf[pos + 0] & 0x01;
	p_descriptor->multiplex_buffer_utilization.ltw_offset_lower_bound = (buf[pos + 0] << 8) | buf[pos + 1];
	p_descriptor->multiplex_buffer_utilization.reserved = buf[pos + 2] & 0x01;
	p_descriptor->multiplex_buffer_utilization.ltw_offset_upper_bound = ((buf[pos + 2] >> 1) << 7) | (buf[pos + 3] >> 1);

	pos += 4;

	return pos;
}

void deallocate_multiplex_buffer_utilization_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_system_clock_descriptor(struct  descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing System Clock Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->system_clock.external_clock_reference_indicator = buf[pos + 0] & 0x01;
	p_descriptor->system_clock.reserved_1 = (buf[pos + 0] & 0x03) >> 1;
	p_descriptor->system_clock.clock_accuracy_integer = buf[pos + 0] >> 2;
	p_descriptor->system_clock.clock_accuracy_exponent = buf[pos + 1] >> 5;
	p_descriptor->system_clock.reserved_2 = buf[pos + 1] & 0x1f;

	pos += 2;

	return pos;
}

void deallocate_system_clock_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_iso_639_language_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t loops;

	printf("--->%s: Parsing ISO 639 Language Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->iso_639_language.p_iso_639_language_code = (struct iso_639_language_code *)
	malloc(sizeof (struct iso_639_language_code) * (p_descriptor->descriptor_length / 4));

	for (loops = 0; loops < (p_descriptor->descriptor_length / 4); loops++) {
		p_descriptor->iso_639_language.p_iso_639_language_code[loops].iso_639_language_code =
			((((buf[pos + 0 + loops] << 8) | buf[pos + 1 + loops]) << 8) |
					buf[pos + 2 + loops]);
		p_descriptor->iso_639_language.p_iso_639_language_code[loops].audio_type =
				buf[pos + 3 + loops];
	}
	pos += loops * 4;

	return pos;
}

void deallocate_iso_639_language_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->iso_639_language.p_iso_639_language_code);
}

uint16_t parse_ca_identifier_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i;
	uint16_t *p_ca_identifier = NULL;

	printf("--->%s: Parsing CA Identifier Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_ca_identifier = (uint16_t *)
		malloc(sizeof (uint16_t) * p_descriptor->descriptor_length);
	for (i = 0; i < p_descriptor->descriptor_length; i += 2)
		p_ca_identifier[i] = ((buf[pos + 0 + i]) << 8) | buf[pos + 1 + i];

	p_descriptor->ca_identifier.p_ca_identifier = p_ca_identifier;
	pos += 2 + i;

	return pos;
}

void deallocate_ca_identifier_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->ca_identifier.p_ca_identifier);
}

uint16_t parse_stream_identifier_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Stream Identifier Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->stream_identifier.component_tag = buf[pos + 0];

	pos += 1;

	return pos;
}

void deallocate_stream_identifier_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_mosaic_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint16_t i, j;

	struct mosaic_info *p_mosaic_info = NULL;
	struct elementary_cell_field *p_elementary_cell_field = NULL;

	printf("--->%s: Parsing Mosaic Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->mosaic.mosaic_entry_point = buf[pos + 0] >> 7;
	p_descriptor->mosaic.number_of_horiz_elementary_cells = (buf[pos + 0] >> 4) & 0x07;
	p_descriptor->mosaic.reserved = buf[pos + 0] >> 3;
	p_descriptor->mosaic.number_of_vert_elementary_cells = buf[pos + 0] & 0x07;

	p_mosaic_info = (struct mosaic_info *)
		malloc(sizeof (struct mosaic_info) * (p_descriptor->descriptor_length - 1));
	for (i = 0; i < (p_descriptor->descriptor_length - 1); i++) {
		p_mosaic_info[i].logical_cell_id = buf[pos + 1 + i] >> 2;
		p_mosaic_info[i].reserved =
			((buf[pos + 1 + i] & 0x03) << 5) | (buf[pos + 2 + i] >> 3);
		p_mosaic_info[i].logical_cell_presentation_info = buf[pos + 2 + i] & 0x07;

		p_mosaic_info[i].elementary_cell_field_length = buf[pos + 3 + i];
		p_elementary_cell_field = (struct elementary_cell_field *)
			malloc(sizeof (struct elementary_cell_field) * p_mosaic_info[i].elementary_cell_field_length);
		for (j = 0; j < p_mosaic_info[i].elementary_cell_field_length; j++) {
			p_elementary_cell_field[j].reserved = buf[pos + 4 + i + j] >> 6;
			p_elementary_cell_field[j].elementary_cell_id = buf[pos + 4 + i + j] & 0x3f;
		}
		p_mosaic_info[i].p_elementary_cell_field = p_elementary_cell_field;
		pos += j;

		p_mosaic_info[i].cell_linkage_info = buf[pos + 5 + i];
		switch (p_mosaic_info[i].cell_linkage_info) {
			case 0x01:
				p_mosaic_info[i].bouquet_id = (buf[pos + 6 + 1] << 8) |
						buf[pos + 7 + i];
				break;

			case 0x02:
			case 0x03:
				p_mosaic_info[i].original_network_id = (buf[pos + 8 + i] << 8) |
						buf[pos + 9 + i];
				p_mosaic_info[i].transport_stream_id = (buf[pos + 10 + i] << 8) |
						buf[pos + 11 + i];
				p_mosaic_info[i].service_id = (buf[pos + 12 + i] << 8) |
						buf[pos + 13 + i];
				break;

			case 0x04:
				p_mosaic_info[i].original_network_id = (buf[pos + 8 + i] << 8) |
						buf[pos + 9 + i];
				p_mosaic_info[i].transport_stream_id = (buf[pos + 10 + i] << 8) |
						buf[pos + 11 + i];
				p_mosaic_info[i].service_id = (buf[pos + 12 + i] << 8) |
						buf[pos + 13 + i];
				p_mosaic_info[i].event_id = (buf[pos + 14 + i] << 8) |
						buf[pos + 15 + i];
				break;
			default:
				break;
				return -1;
		}
	}
	p_descriptor->mosaic.p_mosaic_info = p_mosaic_info;
	pos += 16 + i;

	return pos;
}

void deallocate_mosaic_descriptor(struct descriptor *p_descriptor)
{
	uint8_t i;

	if (p_descriptor->descriptor_length) {
		for (i = 0; i < (p_descriptor->descriptor_length - 1); i++) {
			if (p_descriptor->mosaic.p_mosaic_info[i].elementary_cell_field_length)
				free(p_descriptor->mosaic.p_mosaic_info[i].p_elementary_cell_field);
		}
		free(p_descriptor->mosaic.p_mosaic_info);
	}
}

uint16_t parse_ca_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint8_t loops = 0;
	uint8_t *p_private_data = NULL;

	printf("--->%s: Parsing CA Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->ca.ca_system_id = (buf[pos + 0] << 8) | buf[pos + 1];
	p_descriptor->ca.reserved = buf[pos + 2] >> 5;
	p_descriptor->ca.ca_pid = ((buf[pos + 2] & 0x1f) << 8) | buf[pos + 3];
	p_private_data = (uint8_t *)
		malloc(sizeof (uint8_t) * (p_descriptor->descriptor_length - 4));

	printf("--->%s: CA System=[%02x], CA PID=[%02x]\n", __FUNCTION__,
	       p_descriptor->ca.ca_system_id, p_descriptor->ca.ca_pid);
	printf("--->%s: CA Private Data=[ ", __FUNCTION__);
	while (loops < (p_descriptor->descriptor_length - 4)) {
		p_private_data[loops] = buf[pos + 4 + loops];
		printf("%02x ", p_private_data[loops]);
		loops++;
	}
	printf("]\n");
	p_descriptor->ca.p_private_data_byte = p_private_data;

	pos += loops + 4;

	return pos;
}

void deallocate_ca_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->descriptor_length)
		free(p_descriptor->ca.p_private_data_byte);
}

uint16_t parse_video_window_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Video Window Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->video_window.horizontal_offset = (buf[pos + 0] << 6) | ((buf[pos + 1] >> 2) & 0x3f);
	p_descriptor->video_window.vertical_offset = ((((buf[pos + 1] >> 2) << 8) | buf[pos + 2]) << 4) |
			((buf[pos + 3] >> 4) & 0x0f);
	p_descriptor->video_window.window_priority = buf[pos + 3] & 0x0f;

	pos += 4;

	return pos;
}

void deallocate_video_window_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_target_background_grid_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Target Background Grid Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->target_background_grid.horizontal_size = (buf[pos + 0] << 6) | ((buf[pos + 1] >> 2) & 0x3f);
	p_descriptor->target_background_grid.vertical_size =
		((((buf[pos + 1] >> 2) << 8) | buf[pos + 2]) << 4) | ((buf[pos + 3] >> 4) & 0x0f);
	p_descriptor->target_background_grid.aspect_ratio_information = buf[pos + 3] & 0x0f;

	pos += 4;

	return pos;
}

void deallocate_target_background_grid_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_data_stream_alignment_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Data Stream Alignment Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->data_stream_alignment.alignment_type = buf[pos + 0];

	pos += 1;

	return pos;
}

void deallocate_data_stream_alignment_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_registration_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint32_t i = 0;

	printf("--->%s: Parsing registration Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->registration.format_identifier =
		((((((buf[pos + 0] << 8) | buf[pos + 1])) << 8) | buf[pos + 2]) << 8) | buf[pos + 3];
	if(p_descriptor->descriptor_length - 4 > 0) {

		p_descriptor->registration.p_additional_id_info = (uint8_t *)
			malloc(sizeof (uint8_t) * (p_descriptor->descriptor_length - 4));

		for (i = 0; i < p_descriptor->descriptor_length - 4; i++)
			p_descriptor->registration.p_additional_id_info[i] = buf[pos + 4 + i];
	}

	pos += i + 4;

	return pos;
}

void deallocate_registration_descriptor(struct descriptor *p_descriptor)
{
	if (p_descriptor->registration.format_identifier)
		free(p_descriptor->registration.p_additional_id_info);
}

uint16_t parse_hierarchy_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Hierarchy Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->hierarchy.reserved_1 = (buf[pos + 0] >> 4) & 0x0f;
	p_descriptor->hierarchy.hierarchy_type = buf[pos + 0] & 0x0f;
	p_descriptor->hierarchy.reserved_2 = (buf[pos + 1] >> 6) & 0x03;
	p_descriptor->hierarchy.hierarchy_layer_index = buf[pos + 1] & 0x3f;
	p_descriptor->hierarchy.reserved_3 = (buf[pos + 2] >> 6) & 0x03;
	p_descriptor->hierarchy.hierarchy_embedded_layer_index = buf[pos + 2] & 0x3f;
	p_descriptor->hierarchy.reserved_4 = (buf[pos + 3] >> 6) & 0x03;
	p_descriptor->hierarchy.hierarchy_channel = buf[pos + 3] & 0x3f;

	pos += 4;

	return pos;
}

void deallocate_hierarchy_descriptor(struct descriptor *p_descriptor)
{
	return;
}

uint16_t parse_audio_stream_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Audio Stream Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->audio_stream.free_format_flag = (buf[pos + 0] >> 7) & 0x01;
	p_descriptor->audio_stream.id = (buf[pos + 0] >> 6) & 0x01;
	p_descriptor->audio_stream.layer = (buf[pos + 0] >> 4) & 0x03;
	p_descriptor->audio_stream.variable_rate_audio_indicator = (buf[pos + 0] >> 3) & 0x01;
	p_descriptor->audio_stream.reserved = buf[pos + 0] & 0x07;

	pos += 1;

	return pos;
}

void deallocate_audio_stream_descriptor(struct descriptor *p_descrioptor)
{
	return;
}

uint16_t parse_video_stream_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	printf("--->%s: Parsing Video Stream Descriptor\n", __FUNCTION__);

	pos = parse_descriptor_header(p_descriptor, buf, pos);

	p_descriptor->video_stream.multiple_frame_rate_flag = buf[pos + 0] >> 7;
	p_descriptor->video_stream.frame_rate_code = (buf[pos + 0] >> 3) & 0x0f;
	p_descriptor->video_stream.mpeg_1_only_flag = (buf[pos + 0] >> 2) & 0x01;
	p_descriptor->video_stream.constrained_parameter_flag = (buf[pos + 0] >> 1) & 0x01;
	p_descriptor->video_stream.still_picture_flag = buf[pos + 0] & 0x01;

	if (p_descriptor->video_stream.mpeg_1_only_flag == 0) {
		p_descriptor->video_stream.profile_and_level_indication = buf[pos + 1];
		p_descriptor->video_stream.chroma_format = (buf[pos + 2] >> 6) & 0x03;
		p_descriptor->video_stream.frame_rate_extension_flag	= (buf[pos + 2] >> 5) & 0x01;
		p_descriptor->video_stream.reserved = buf[pos + 2] & 0x1f;
	}
	pos += 3;

	return pos;
}

void deallocate_video_stream_descriptor(struct descriptor *p_descriptor)
{
	return;
}

void deallocate_descriptor_storage(struct descriptor *p_descriptor)
{

	switch(p_descriptor->descriptor_tag) {
		case TAG_VIDEO_STREAM_DESCRIPTOR:
				deallocate_video_stream_descriptor(p_descriptor);
			break;
		case TAG_AUDIO_STREAM_DESCRIPTOR:
				deallocate_audio_stream_descriptor(p_descriptor);
			break;
		case TAG_HIERARCHY_DESCRIPTOR:
				deallocate_hierarchy_descriptor(p_descriptor);
			break;
		case TAG_REGISTRATION_DESCRIPTOR:
				deallocate_registration_descriptor(p_descriptor);
			break;
		case TAG_DATA_STREAM_ALIGNMENT_DESCRIPTOR:
				deallocate_data_stream_alignment_descriptor(p_descriptor);
			break;
		case TAG_TARGET_BACKGROUND_GRID_DESCRIPTOR:
				deallocate_target_background_grid_descriptor(p_descriptor);
			break;
		case TAG_VIDEO_WINDOW_DESCRIPTOR:
				deallocate_video_window_descriptor(p_descriptor);
			break;
		case TAG_CA_DESCRIPTOR:	/*	CA descriptor		*/
				deallocate_ca_descriptor(p_descriptor);
			break;
		case TAG_ISO_639_LANGUAGE_DESCRIPTOR:
				deallocate_iso_639_language_descriptor(p_descriptor);
			break;
		case TAG_SYSTEM_CLOCK_DESCRIPTOR:
				deallocate_system_clock_descriptor(p_descriptor);
			break;
		case TAG_MULTIPLEX_BUFFER_UTILIZATION_DESCRIPTOR:
				deallocate_multiplex_buffer_utilization_descriptor(p_descriptor);
			break;
		case TAG_COPYRIGHT_DESCRIPTOR:
				deallocate_copyright_descriptor(p_descriptor);
			break;
		case TAG_MAXIMUM_BITRATE_DESCRIPTOR:
				deallocate_maximum_bitrate_descriptor(p_descriptor);
			break;
		case TAG_PRIVATE_DATA_INDICATOR_DESCRIPTOR:
				deallocate_private_data_indicator_descriptor(p_descriptor);
			break;
		case TAG_SMOOTHING_BUFFER_DESCRIPTOR:
				deallocate_smoothing_buffer_descriptor(p_descriptor);
			break;
		case TAG_STD_DESCRIPTOR:
				deallocate_std_descriptor(p_descriptor);
			break;
		case TAG_IBP_DESCRIPTOR:
				deallocate_ibp_descriptor(p_descriptor);
			break;
		case TAG_MPEG_4_VIDEO_DESCRIPTOR:
				deallocate_mpeg_4_video_descriptor(p_descriptor);
			break;
		case TAG_MPEG_4_AUDIO_DESCRIPTOR:
				deallocate_mpeg_4_audio_descriptor(p_descriptor);
			break;
		case TAG_IOD_DESCRIPTOR:
				deallocate_iod_descriptor(p_descriptor);
			break;
		case TAG_SL_DESCRIPTOR:
				deallocate_sl_descriptor(p_descriptor);
			break;
		case TAG_FMC_DESCRIPTOR:
				deallocate_fmc_descriptor(p_descriptor);
			break;
		case TAG_EXTERNAL_ES_ID_DESCRIPTOR:
				deallocate_external_es_id_descriptor(p_descriptor);
			break;
		case TAG_MUXCODE_DESCRIPTOR:
				deallocate_muxcode_descriptor(p_descriptor);
			break;
		case TAG_FMX_BUFFER_SIZE_DESCRIPTOR:
				deallocate_fmxbuffer_size_descriptor(p_descriptor);
			break;
		case TAG_MULTIPLEX_BUFFER_DESCRIPTOR:
				deallocate_multiplex_buffer_descriptor(p_descriptor);
			break;
		case TAG_NETWORK_NAME_DESCRIPTOR:
				deallocate_network_name_descriptor(p_descriptor);
			break;
		case TAG_SERVICE_LIST_DESCRIPTOR:
				deallocate_service_list_descriptor(p_descriptor);
			break;
		case TAG_STUFFING_DESCRIPTOR:
				deallocate_stuffing_descriptor(p_descriptor);
			break;
		case TAG_SATTELITE_DELIVERY_SYSTEM_DESCRIPTOR:
				deallocate_satellite_delivery_system(p_descriptor);
			break;
		case TAG_CABLE_DELIVERY_SYSTEM_DESCRIPTOR:
				deallocate_cable_delivery_system(p_descriptor);
			break;
		case TAG_BOUQUET_NAME_DESCRIPTOR:
				deallocate_bouquet_name_descriptor(p_descriptor);
			break;
		case TAG_SERVICE_DESCRIPTOR:
				deallocate_service_descriptor(p_descriptor);
			break;
		case TAG_COUNTRY_AVAILABILITY_DESCRIPTOR:
				deallocate_country_availability_descriptor(p_descriptor);
			break;
		case TAG_LINKAGE_DESCRIPTOR:
				deallocate_linkage_descriptor(p_descriptor);
			break;
		case TAG_NVOD_REFERENCE_DESCRIPTOR:
				deallocate_nvod_reference_descriptor(p_descriptor);
			break;
		case TAG_TIME_SHIFTED_SERVICE_DESCRIPTOR:
				deallocate_time_shifted_service_descriptor(p_descriptor);
			break;
		case TAG_SHORT_EVENT_DESCRIPTOR:
				deallocate_short_event_descriptor(p_descriptor);
			break;
		case TAG_EXTENDED_EVENT_DESCRIPTOR:
				deallocate_extended_event_descriptor(p_descriptor);
			break;
		case TAG_TIME_SHIFTED_EVENT_DESCRIPTOR:
				deallocate_time_shifted_event_descriptor(p_descriptor);
			break;
		case TAG_COMPONENT_DESCRIPTOR:
				deallocate_component_descriptor(p_descriptor);
			break;
		case TAG_MOSAIC_DESCRIPTOR:
				deallocate_mosaic_descriptor(p_descriptor);
			break;
		case TAG_STREAM_IDENTIFIER_DESCRIPTOR:
				deallocate_stream_identifier_descriptor(p_descriptor);
			break;
		case TAG_CA_IDENTIFIER_DESCRIPTOR:
				deallocate_ca_identifier_descriptor(p_descriptor);
			break;
		case TAG_CONTENT_DESCRIPTOR:
				deallocate_content_descriptor(p_descriptor);
			break;
		case TAG_PARENTAL_RATING_DESCRIPTOR:
				deallocate_parental_rating_descriptor(p_descriptor);
			break;
		case TAG_TELETEXT_DESCRIPTOR:
				deallocate_teletext_descriptor(p_descriptor);
			break;
		case TAG_TELEPHONE_DESCRIPTOR:
				deallocate_telephone_descriptor(p_descriptor);
			break;
		case TAG_LOCAL_TIME_OFFSET_DESCRIPTOR:
				deallocate_local_time_offset_descriptor(p_descriptor);
			break;
		case TAG_SUBTITLING_DESCRIPTOR:
				deallocate_subtitling_descriptor(p_descriptor);
			break;
		case TAG_TERRESTRIAL_DELIVERY_SYSTEM_DESCRIPTOR:
				deallocate_terrestrial_delivery_system(p_descriptor);
			break;
		case TAG_MULTILINGUAL_NETWORK_NAME_DESCRIPTOR:
				deallocate_multilingual_network_name_descriptor(p_descriptor);
			break;
		case TAG_MULTILINGUAL_BOUQUET_NAME_DESCRIPTOR:
				deallocate_multilingual_bouquet_name_descriptor(p_descriptor);
			break;
/*	case TAG_MULTILINGUAL_SERVICE_NAME_DESCRIPTOR: */
/*		deallocate_multilingual_service_name_descriptor(p_descriptor); */
/*		break; */
/*	case TAG_MULTILINGUAL_COMPONENT_DESCRIPTOR: */
/*		deallocate_multilingual_component_descriptor(p_descriptor); */
/*		break; */
/*	case TAG_PRIVATE_DATA_SPECIFIER_DESCRIPTOR: */
/*		deallocate_private_data_specifier_descriptor(p_descriptor); */
/*		break; */
/*	case TAG_SERVICE_MOVE_DESCRIPTOR: */
/*		deallocate_service_move_descriptor(p_descriptor); */
/*		break; */
/*	case TAG_SHORT_SMOOTHING_BUFFER_DESCRIPTOR: */
/*		deallocate_short_smoothing_buffer_descriptor(p_descriptor); */
/*		break; */
/*	case TAG_FREQUENCY_LIST_DESCRIPTOR: */
/*		deallocate_frequency_list_descriptor(p_descriptor); */
/*		break; */
/*	case TAG_PARTIAL_TRANSPORT_DESCRIPTOR: */
/*		dealloc_partial_transport_stream(p_descriptor); */
/*		break; */
/*	case TAG_DATA_BROADCAST_DESCRIPTOR: */
/*		deallocate_data_broadcast_descriptor(p_descriptor); */
/*		break; */
/*	case TAG_CA_SYSTEM_DESCRIPTOR: */
/*		dealloc_ca_system_descriptor(p_descriptor); */
/*		break; */
/*	case TAG_DATA_BROADCAST_ID_DESCRIPTOR: */
/*		deallocate_data_broadcast_id_descriptor(p_descriptor); */
/*		break; */
		default:	/*	parse an unknown	*/
		printf("ERROR! Tried to free descriptor with unknown tag [%d]. This should not happen.\n",
		       p_descriptor->descriptor_tag);
				break;
	}

	return;
}


uint16_t parse_descriptor(struct descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	// Parse header but do not move pos
	parse_descriptor_header(p_descriptor, buf, pos);

	printf("-->%s: Descriptor: Tag=[0x%02x], Length=[%d]\n", __FUNCTION__,
	       p_descriptor->descriptor_tag, p_descriptor->descriptor_length);

	switch (p_descriptor->descriptor_tag) {
		case TAG_VIDEO_STREAM_DESCRIPTOR:
		pos = parse_video_stream_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_AUDIO_STREAM_DESCRIPTOR:
		pos = parse_audio_stream_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_HIERARCHY_DESCRIPTOR:
		pos = parse_hierarchy_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_REGISTRATION_DESCRIPTOR:
		pos = parse_registration_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_DATA_STREAM_ALIGNMENT_DESCRIPTOR:
		pos = parse_data_stream_alignment_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_TARGET_BACKGROUND_GRID_DESCRIPTOR:
		pos = parse_target_background_grid_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_VIDEO_WINDOW_DESCRIPTOR:
		pos = parse_video_window_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_CA_DESCRIPTOR:	/*	CA descriptor		*/
		pos = parse_ca_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_ISO_639_LANGUAGE_DESCRIPTOR:
		pos = parse_iso_639_language_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_SYSTEM_CLOCK_DESCRIPTOR:
		pos = parse_system_clock_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_MULTIPLEX_BUFFER_UTILIZATION_DESCRIPTOR:
		pos = parse_multiplex_buffer_utilization_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_COPYRIGHT_DESCRIPTOR:
		pos = parse_copyright_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_MAXIMUM_BITRATE_DESCRIPTOR:
		pos = parse_maximum_bitrate_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_PRIVATE_DATA_INDICATOR_DESCRIPTOR:
		pos = parse_private_data_indicator_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_SMOOTHING_BUFFER_DESCRIPTOR:
		pos = parse_smoothing_buffer_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_STD_DESCRIPTOR:
		pos = parse_std_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_IBP_DESCRIPTOR:
		pos = parse_ibp_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_MPEG_4_VIDEO_DESCRIPTOR:
		pos = parse_mpeg_4_video_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_MPEG_4_AUDIO_DESCRIPTOR:
		pos = parse_mpeg_4_audio_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_IOD_DESCRIPTOR:
		pos = parse_iod_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_SL_DESCRIPTOR:
		pos = parse_sl_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_FMC_DESCRIPTOR:
		pos = parse_fmc_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_EXTERNAL_ES_ID_DESCRIPTOR:
		pos = parse_external_es_id_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_MUXCODE_DESCRIPTOR:
		pos = parse_muxcode_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_FMX_BUFFER_SIZE_DESCRIPTOR:
		pos = parse_fmxbuffer_size_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_MULTIPLEX_BUFFER_DESCRIPTOR:
		pos = parse_multiplex_buffer_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_NETWORK_NAME_DESCRIPTOR:
		pos = parse_network_name_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_SERVICE_LIST_DESCRIPTOR:
		pos = parse_service_list_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_STUFFING_DESCRIPTOR:
		pos = parse_stuffing_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_SATTELITE_DELIVERY_SYSTEM_DESCRIPTOR:
		pos = parse_satellite_delivery_system(p_descriptor, buf, pos);
			break;
		case TAG_CABLE_DELIVERY_SYSTEM_DESCRIPTOR:
		pos = parse_cable_delivery_system(p_descriptor, buf, pos);
			break;
		case TAG_BOUQUET_NAME_DESCRIPTOR:
		pos = parse_bouquet_name_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_SERVICE_DESCRIPTOR:
		pos = parse_service_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_COUNTRY_AVAILABILITY_DESCRIPTOR:
		pos = parse_country_availability_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_LINKAGE_DESCRIPTOR:
		pos = parse_linkage_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_NVOD_REFERENCE_DESCRIPTOR:
		pos = parse_nvod_reference_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_TIME_SHIFTED_SERVICE_DESCRIPTOR:
		pos = parse_time_shifted_service_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_SHORT_EVENT_DESCRIPTOR:
		pos = parse_short_event_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_EXTENDED_EVENT_DESCRIPTOR:
		pos = parse_extended_event_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_TIME_SHIFTED_EVENT_DESCRIPTOR:
		pos = parse_time_shifted_event_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_COMPONENT_DESCRIPTOR:
		pos = parse_component_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_MOSAIC_DESCRIPTOR:
		pos = parse_mosaic_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_STREAM_IDENTIFIER_DESCRIPTOR:
		pos = parse_stream_identifier_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_CA_IDENTIFIER_DESCRIPTOR:
		pos = parse_ca_identifier_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_CONTENT_DESCRIPTOR:
		pos = parse_content_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_PARENTAL_RATING_DESCRIPTOR:
		pos = parse_parental_rating_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_TELETEXT_DESCRIPTOR:
		pos = parse_teletext_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_TELEPHONE_DESCRIPTOR:
		pos = parse_telephone_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_LOCAL_TIME_OFFSET_DESCRIPTOR:
		pos = parse_local_time_offset_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_SUBTITLING_DESCRIPTOR:
		pos = parse_subtitling_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_TERRESTRIAL_DELIVERY_SYSTEM_DESCRIPTOR:
		pos = parse_terrestrial_delivery_system(p_descriptor, buf, pos);
			break;
		case TAG_MULTILINGUAL_NETWORK_NAME_DESCRIPTOR:
		pos = parse_multilingual_network_name_descriptor(p_descriptor, buf, pos);
			break;
		case TAG_MULTILINGUAL_BOUQUET_NAME_DESCRIPTOR:
		pos = parse_multilingual_bouquet_name_descriptor(p_descriptor, buf, pos);
			break;
/*	case TAG_MULTILINGUAL_SERVICE_NAME_DESCRIPTOR: */
/*		pos = parse_multilingual_service_name_descriptor(p_descriptor, buf, pos); */
/*		break; */
/*	case TAG_MULTILINGUAL_COMPONENT_DESCRIPTOR: */
/*		pos = parse_multilingual_component_descriptor(p_descriptor, buf, pos); */
/*		break; */
/*	case TAG_PRIVATE_DATA_SPECIFIER_DESCRIPTOR: */
/*		pos = parse_private_data_specifier_descriptor(p_descriptor, buf, pos); */
/*		break; */
/*	case TAG_SERVICE_MOVE_DESCRIPTOR: */
/*		pos = parse_service_move_descriptor(p_descriptor, buf, pos); */
/*		break; */
/*	case TAG_SHORT_SMOOTHING_BUFFER_DESCRIPTOR: */
/*		pos = parse_short_smoothing_buffer_descriptor(p_descriptor, buf, pos); */
/*		break; */
/*	case TAG_FREQUENCY_LIST_DESCRIPTOR: */
/*		pos = parse_frequency_list_descriptor(p_descriptor, buf, pos); */
/*		break; */
/*	case TAG_PARTIAL_TRANSPORT_DESCRIPTOR: */
/*		pos = pars_partial_transport_stream(p_descriptor, buf, pos); */
/*		break; */
/*	case TAG_DATA_BROADCAST_DESCRIPTOR: */
/*		pos = parse_data_broadcast_descriptor(p_descriptor, buf, pos); */
/*		break; */
/*	case TAG_CA_SYSTEM_DESCRIPTOR: */
/*		pos = parse_ca_system_descriptor(p_descriptor, buf, pos); */
/*		break; */
/*	case TAG_DATA_BROADCAST_ID_DESCRIPTOR: */
/*		pos = parse_data_broadcast_id_descriptor(p_descriptor, buf, pos); */
/*		break; */
		default:	/*	parse an unknown	*/
		pos += 2 + p_descriptor->descriptor_length;
		printf("--->Skipping unknown descriptor tag [0x%02x]\n", p_descriptor->descriptor_tag);
				break;
	}

	return pos;
}
