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

#ifndef _UCSI_DVB_MULTILINGUAL_SERVICE_NAME_DESCRIPTOR
#define _UCSI_DVB_MULTILINGUAL_SERVICE_NAME_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/common.h>

struct dvb_multilingual_service_name_descriptor {
	struct descriptor d;

	/* struct dvb_multilingual_service_name service_names[] */
} packed;

struct dvb_multilingual_service_name {
	uint8_t iso_639_language_code[3];
	uint8_t service_provider_name_length;
	/* uint8_t service_provider_name[] */
	/* struct dvb_multilingual_service_name_part2 part2 */
} packed;

struct dvb_multilingual_service_name_part2 {
	uint8_t service_name_length;
	/* uint8_t service_name[] */
} packed;

static inline struct dvb_multilingual_service_name_descriptor*
	dvb_multilingual_service_name_descriptor_parse(struct descriptor* d)
{
	uint8_t* buf = (uint8_t*) d + 2;
	int pos = 0;
	int len = d->len;

	while(pos < len) {
		struct dvb_multilingual_service_name *e =
			(struct dvb_multilingual_service_name*) (buf+pos);
		struct dvb_multilingual_service_name_part2 *e2;

		pos += sizeof(struct dvb_multilingual_service_name);

		if (pos > len)
			return NULL;

		pos += e->service_provider_name_length;

		if (pos > len)
			return NULL;

		e2 = (struct dvb_multilingual_service_name_part2*) (buf+pos);

		pos += sizeof(struct dvb_multilingual_service_name_part2);

		if (pos > len)
			return NULL;

		pos += e2->service_name_length;

		if (pos > len)
			return NULL;
	}

	return (struct dvb_multilingual_service_name_descriptor*) d;
}

#define dvb_multilingual_service_name_descriptor_for_each(d, pos) \
	for ((pos) = dvb_multilingual_service_name_descriptor_names_first(d); \
	     (pos); \
	     (pos) = dvb_multilingual_service_name_descriptor_names_next(d, pos))

static inline uint8_t *
	dvb_multilingual_service_name_service_provider_name(struct dvb_multilingual_service_name *e)
{
	return (uint8_t *) e + sizeof(struct dvb_multilingual_service_name);
}

static inline struct dvb_multilingual_service_name_part2 *
	dvb_multilingual_service_name_part2(struct dvb_multilingual_service_name *e)
{
	return (struct dvb_multilingual_service_name_part2 *)
		((uint8_t *) e + sizeof(struct dvb_multilingual_service_name) +
		 e->service_provider_name_length);
}

static inline uint8_t *
	dvb_multilingual_service_name_service_name(struct dvb_multilingual_service_name_part2 *e)
{
	return (uint8_t *) e + sizeof(struct dvb_multilingual_service_name_part2);
}










/******************************** PRIVATE CODE ********************************/
static inline struct dvb_multilingual_service_name*
	dvb_multilingual_service_name_descriptor_names_first(struct dvb_multilingual_service_name_descriptor *d)
{
	if (d->d.len == 0)
		return NULL;

	return (struct dvb_multilingual_service_name *)
		(uint8_t*) d + sizeof(struct dvb_multilingual_service_name_descriptor);
}

static inline struct dvb_multilingual_service_name*
	dvb_multilingual_service_name_descriptor_names_next(struct dvb_multilingual_service_name_descriptor *d,
							    struct dvb_multilingual_service_name *pos)
{
	struct dvb_multilingual_service_name_part2 * part2 =
			dvb_multilingual_service_name_part2(pos);
	uint8_t *end = (uint8_t*) d + 2 + d->d.len;
	uint8_t *next =	(uint8_t *) part2+
			sizeof(struct dvb_multilingual_service_name_part2) +
			part2->service_name_length;

	if (next >= end)
		return NULL;

	return (struct dvb_multilingual_service_name *) next;
}

#endif
