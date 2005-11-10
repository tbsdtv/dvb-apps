/*
 * section and descriptor parser
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef _UCSI_DVB_VBI_DATA_DESCRIPTOR
#define _UCSI_DVB_VBI_DATA_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/common.h>

struct dvb_vbi_data_descriptor {
	struct descriptor d;

	/* struct dvb_vbi_data_entry entries[] */
} packed;

struct dvb_vbi_data_entry {
	uint8_t data_service_id;
	uint8_t data_service_descriptor_length;
	/* uint8_t data[] */
} packed;

struct dvb_vbi_data_1 {
  EBIT3(uint8_t reserved 	: 2; ,
	uint8_t field_parity 	: 1; ,
	uint8_t line_offset	: 5; );
} packed;

static inline struct dvb_vbi_data_descriptor*
	dvb_vbi_data_descriptor_parse(struct descriptor* d)
{
	uint8_t* p = (uint8_t*) d + 2;
	int pos = 0;
	int len = d->len;

	while(pos < len) {
		struct dvb_vbi_data_entry *e =
			(struct dvb_vbi_data_entry*) (p+pos);

		pos += sizeof(struct dvb_vbi_data_entry);

		if (pos > len)
			return NULL;

		pos += e->data_service_descriptor_length;

		if (pos > len)
			return NULL;

		if (e->data_service_id == 0x01)
			if (e->data_service_descriptor_length != sizeof(struct dvb_vbi_data_1))
				return NULL;
	}

	return (struct dvb_vbi_data_descriptor*) d;
}

#define dvb_vbi_data_descriptor_entries_for_each(d, pos) \
	for ((pos) = dvb_vbi_data_descriptor_entries_first(d); \
	     (pos); \
	     (pos) = dvb_vbi_data_descriptor_entries_next(d, pos))

static inline uint8_t *
	dvb_vbi_data_entry_data(struct dvb_vbi_data_entry *d)
{
	return (uint8_t *) d + sizeof(struct dvb_vbi_data_entry);
}










/******************************** PRIVATE CODE ********************************/
static inline struct dvb_vbi_data_entry*
	dvb_vbi_data_descriptor_entries_first(struct dvb_vbi_data_descriptor *d)
{
	if (d->d.len == 0)
		return NULL;

	return (struct dvb_vbi_data_entry *)
		((uint8_t*) d + sizeof(struct dvb_vbi_data_descriptor));
}

static inline struct dvb_vbi_data_entry*
	dvb_vbi_data_descriptor_entries_next(struct dvb_vbi_data_descriptor *d,
					     struct dvb_vbi_data_entry *pos)
{
	uint8_t *end = (uint8_t*) d + 2 + d->d.len;
	uint8_t *next =	(uint8_t *) pos + sizeof(struct dvb_vbi_data_entry) +
			pos->data_service_descriptor_length;

	if (next >= end)
		return NULL;

	return (struct dvb_vbi_data_entry *) next;
}

#endif
