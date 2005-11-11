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

#ifndef _UCSI_DVB_TELETEXT_DESCRIPTOR
#define _UCSI_DVB_TELETEXT_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/endianops.h>

/**
 * dvb_teletext_descriptor structure.
 */
struct dvb_teletext_descriptor {
	struct descriptor d;

	/* struct dvb_teletext_entry entries[] */
} packed;

/**
 * An entry in the entries field of a dvb_teletext_descriptor.
 */
struct dvb_teletext_entry {
	uint8_t iso_639_language_code[3];
  EBIT2(uint8_t teletext_type		: 5; ,
	uint8_t teletext_magazine_number: 3; );
	uint8_t teletext_page_number;
} packed;

/**
 * Process a dvb_teletext_descriptor.
 *
 * @param d Generic descriptor.
 * @return dvb_teletext_descriptor pointer, or NULL on error.
 */
static inline struct dvb_teletext_descriptor*
	dvb_teletext_descriptor_codec(struct descriptor* d)
{
	if (d->len % sizeof(struct dvb_teletext_entry))
		return NULL;

	return (struct dvb_teletext_descriptor*) d;
}

/**
 * Iterator for entries field of a dvb_teletext_descriptor.
 *
 * @param d dvb_teletext_descriptor pointer.
 * @param pos Variable holding a pointer to the current dvb_teletext_entry.
 */
#define dvb_teletext_descriptor_entries_for_each(d, pos) \
	for ((pos) = dvb_teletext_descriptor_entries_first(d); \
	     (pos); \
	     (pos) = dvb_teletext_descriptor_entries_next(d, pos))










/******************************** PRIVATE CODE ********************************/
static inline struct dvb_teletext_entry*
	dvb_teletext_descriptor_entries_first(struct dvb_teletext_descriptor *d)
{
	if (d->d.len == 0)
		return NULL;

	return (struct dvb_teletext_entry *)
		(uint8_t*) d + sizeof(struct dvb_teletext_descriptor);
}

static inline struct dvb_teletext_entry*
	dvb_teletext_descriptor_entries_next(struct dvb_teletext_descriptor *d,
					     struct dvb_teletext_entry *pos)
{
	uint8_t *end = (uint8_t*) d + 2 + d->d.len;
	uint8_t *next =	(uint8_t *) pos + sizeof(struct dvb_teletext_entry);

	if (next >= end)
		return NULL;

	return (struct dvb_teletext_entry *) next;
}

#endif
