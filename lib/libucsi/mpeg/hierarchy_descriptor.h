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

#ifndef _UCSI_MPEG_HIERARCHY_DESCRIPTOR
#define _UCSI_MPEG_HIERARCHY_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/endianops.h>

/**
 * mpeg_hierarchy_descriptor structure.
 */
struct mpeg_hierarchy_descriptor {
	struct descriptor d;

  EBIT2(uint8_t reserved_1			: 4; ,
	uint8_t hierarchy_type			: 4; );
  EBIT2(uint8_t reserved_2			: 2; ,
	uint8_t hierarchy_layer_index		: 6; );
  EBIT2(uint8_t reserved_3			: 2; ,
	uint8_t hierarchy_embedded_layer_index	: 6; );
  EBIT2(uint8_t reserved_4			: 2; ,
	uint8_t hierarchy_channel		: 6; );
} packed;

/**
 * Process an mpeg_hierarchy_descriptor.
 *
 * @param d Generic descriptor structure.
 * @return Pointer to mpeg_hierarchy_descriptor structure, or NULL on error.
 */
static inline struct mpeg_hierarchy_descriptor*
	mpeg_hierarchy_descriptor_codec(struct descriptor* d)
{
	if (d->len != (sizeof(struct mpeg_hierarchy_descriptor) - 2))
		return NULL;

	return (struct mpeg_hierarchy_descriptor*) d;
}

#endif
