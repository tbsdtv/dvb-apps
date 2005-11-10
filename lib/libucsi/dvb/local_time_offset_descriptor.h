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

#ifndef _UCSI_DVB_LOCAL_TIME_OFFSET_DESCRIPTOR
#define _UCSI_DVB_LOCAL_TIME_OFFSET_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/common.h>

struct dvb_local_time_offset_descriptor {
	struct descriptor d;

	/* struct dvb_local_time_offset offsets[] */
} packed;

struct dvb_local_time_offset {
	uint8_t country_code[3];
  EBIT3(uint8_t country_region_id		: 6; ,
	uint8_t reserved			: 1; ,
	uint8_t local_time_offset_polarity	: 1; );
	uint16_t local_time_offset;
	uint8_t time_of_change[5];
	uint16_t next_time_offset;
} packed;

static inline struct dvb_local_time_offset_descriptor*
	dvb_local_time_offset_descriptor_parse(struct descriptor* d)
{
	int len = d->len;
	uint8_t* buf = (uint8_t*) d + 2;
	int pos = 0;

	if (len % sizeof(struct dvb_local_time_offset))
		return NULL;

	while(pos < len) {
		bswap16(buf+pos+4);
		bswap16(buf+pos+9);

		pos += sizeof(struct dvb_local_time_offset);
	}

	return (struct dvb_local_time_offset_descriptor*) d;
}

#define dvb_local_time_offset_descriptor_offsets_for_each(d, pos) \
	for ((pos) = dvb_local_time_offset_descriptor_offsets_first(d); \
	     (pos); \
	     (pos) = dvb_local_time_offset_descriptor_offsets_next(d, pos))










/******************************** PRIVATE CODE ********************************/
static inline struct dvb_local_time_offset*
	dvb_local_time_offset_descriptor_offsets_first(struct dvb_local_time_offset_descriptor *d)
{
	if (d->d.len == 0)
		return NULL;

	return (struct dvb_local_time_offset *)
		(uint8_t*) d + sizeof(struct dvb_local_time_offset_descriptor);
}

static inline struct dvb_local_time_offset*
	dvb_local_time_offset_descriptor_offsets_next(struct dvb_local_time_offset_descriptor *d,
						      struct dvb_local_time_offset *pos)
{
	uint8_t *end = (uint8_t*) d + 2 + d->d.len;
	uint8_t *next =	(uint8_t *) pos + sizeof(struct dvb_local_time_offset);

	if (next >= end)
		return NULL;

	return (struct dvb_local_time_offset *) next;
}

#endif
