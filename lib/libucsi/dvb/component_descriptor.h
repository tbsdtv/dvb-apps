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

#ifndef _UCSI_DVB_COMPONENT_DESCRIPTOR
#define _UCSI_DVB_COMPONENT_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/common.h>

struct dvb_component_descriptor {
	struct descriptor d;

  EBIT2(uint8_t reserved	: 4; ,
	uint8_t stream_content	: 4; );
	uint8_t component_type;
	uint8_t component_tag;
	uint8_t iso_639_language_code[3];
	/* uint8_t text[] */
} packed;

static inline struct dvb_component_descriptor*
	dvb_component_descriptor_parse(struct descriptor* d)
{
	if (d->len < (sizeof(struct dvb_component_descriptor) - 2))
		return NULL;

	return (struct dvb_component_descriptor*) d;
}

static inline uint8_t *
	dvb_component_descriptor_text(struct dvb_component_descriptor *d)
{
	return (uint8_t *) d + sizeof(struct dvb_component_descriptor);
}

static inline int
	dvb_component_descriptor_text_length(struct dvb_component_descriptor *d)
{
	return d->d.len - 6;
}

#endif
