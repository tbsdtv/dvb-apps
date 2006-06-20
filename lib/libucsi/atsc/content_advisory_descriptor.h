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

#ifndef _UCSI_ATSC_CONTENT_ADVISORY_DESCRIPTOR
#define _UCSI_ATSC_CONTENT_ADVISORY_DESCRIPTOR 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <libucsi/descriptor.h>
#include <libucsi/endianops.h>
#include <libucsi/types.h>

/**
 * atsc_content_advisory_descriptor structure.
 */
struct atsc_content_advisory_descriptor {
	struct descriptor d;

  EBIT2(uint8_t reserved		: 2; ,
	uint8_t rating_region_count	: 6; );
	/* struct atsc_content_advisory_entry entries[] */
} __ucsi_packed;

/**
 * An entry in the entries field of a atsc_content_advisory_descriptor.
 */
struct atsc_content_advisory_entry {
	uint8_t rating_region;
	uint8_t rated_dimensions;
	/* struct atsc_content_advisory_entry_dimension dimensions[] */
	/* struct atsc_content_advisory_entry_part2 part2 */
} __ucsi_packed;

/**
 * An entry in the entries field of a atsc_content_advisory_descriptor.
 */
struct atsc_content_advisory_entry_dimension {
	uint8_t rating_dimension_j;
  EBIT2(uint8_t reserved		: 4; ,
	uint8_t rating_value		: 4; );
} __ucsi_packed;

/**
 * Part2 of an atsc_content_advisory_entry.
 */
struct atsc_content_advisory_entry_part2 {
	uint8_t rating_description_length;
	/* struct atsc_text description */
} __ucsi_packed;

// FIXME

/**
 * Process an atsc_content_advisory_descriptor.
 *
 * @param d Generic descriptor pointer.
 * @return atsc_content_advisory_descriptor pointer, or NULL on error.
 */
static inline struct atsc_content_advisory_descriptor*
	atsc_content_advisory_descriptor_codec(struct descriptor* d)
{
	struct atsc_content_advisory_descriptor *ret =
		(struct atsc_content_advisory_descriptor *) d;
	uint8_t *buf = (uint8_t*) d + 2;
	int pos = 0;
	int idx;

	if (d->len < 1)
		return NULL;
	pos++;

	for(idx = 0; idx < ret->number_of_services; idx++) {
		if (d->len < (pos + sizeof(struct atsc_content_advisory_entry)))
			return NULL;
		struct atsc_content_advisory_entry *entry =
			(struct atsc_content_advisory_entry *) (buf + pos);

		bswap16(buf+pos+4);

		pos += sizeof(struct atsc_content_advisory_entry);
	}

	return (struct atsc_content_advisory_descriptor*) d;
}

/**
 * Iterator for entries field of a atsc_content_advisory_descriptor.
 *
 * @param d atsc_content_advisory_descriptor pointer.
 * @param pos Variable holding a pointer to the current atsc_content_advisory_entry.
 */
#define atsc_content_advisory_descriptor_entries_for_each(d, pos) \
	for ((pos) = atsc_content_advisory_descriptor_entries_first(d); \
	     (pos); \
	     (pos) = atsc_content_advisory_descriptor_entries_next(d, pos))










/******************************** PRIVATE CODE ********************************/
static inline struct atsc_content_advisory_entry*
	atsc_content_advisory_descriptor_entries_first(struct atsc_content_advisory_descriptor *d)
{
	if (d->d.len == 0)
		return NULL;

	return (struct atsc_content_advisory_entry *)
		((uint8_t*) d + sizeof(struct atsc_content_advisory_descriptor));
}

static inline struct atsc_content_advisory_entry*
	atsc_content_advisory_descriptor_entries_next(struct atsc_content_advisory_descriptor *d,
					     struct atsc_content_advisory_entry *pos)
{
	uint8_t *end = (uint8_t*) d + 2 + d->d.len;
	uint8_t *next =	(uint8_t *) pos + sizeof(struct atsc_content_advisory_entry);

	if (next >= end)
		return NULL;

	return (struct atsc_content_advisory_entry *) next;
}

#ifdef __cplusplus
}
#endif

#endif
