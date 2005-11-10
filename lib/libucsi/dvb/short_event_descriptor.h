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

#ifndef _UCSI_DVB_SHORT_EVENT_DESCRIPTOR
#define _UCSI_DVB_SHORT_EVENT_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/common.h>

struct dvb_short_event_descriptor {
	struct descriptor d;

	uint8_t iso_639_language_code[3];
	uint8_t event_name_length;
	/* uint8_t event_name[] */
	/* struct dvb_short_event_descriptor_part2 part2 */
};

struct dvb_short_event_descriptor_part2 {
	uint8_t text_length;
	/* uint8_t text[] */
} packed;

static inline struct dvb_short_event_descriptor*
	dvb_short_event_descriptor_parse(struct descriptor* d)
{
	struct dvb_short_event_descriptor *p =
		(struct dvb_short_event_descriptor*) d;
	struct dvb_short_event_descriptor_part2 *p2;
	int pos = sizeof(struct dvb_short_event_descriptor) - 2;
	int len = d->len;

	if (pos > len)
		return NULL;

	pos += p->event_name_length;

	if (pos > len)
		return NULL;

	p2 = (struct dvb_short_event_descriptor_part2*) ((uint8_t*) d + 2 + pos);

	pos += sizeof(struct dvb_short_event_descriptor_part2);

	if (pos > len)
		return NULL;

	pos += p2->text_length;

	if (pos != len)
		return NULL;

	return p;
}

static inline uint8_t *
	dvb_short_event_descriptor_event_name(struct dvb_short_event_descriptor *d)
{
	return (uint8_t *) d + sizeof(struct dvb_short_event_descriptor);
}

static inline struct dvb_short_event_descriptor_part2 *
	dvb_short_event_descriptor_part2(struct dvb_short_event_descriptor *d)
{
	return (struct dvb_short_event_descriptor_part2 *)
		((uint8_t*) d + sizeof(struct dvb_short_event_descriptor) +
		 d->event_name_length);
}

static inline uint8_t *
	dvb_short_event_descriptor_text(struct dvb_short_event_descriptor_part2 *d)
{
	return (uint8_t *) d + sizeof(struct dvb_short_event_descriptor_part2);
}

#endif
