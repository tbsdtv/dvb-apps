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

#ifndef _UCSI_MPEG_FMXBUFFER_SIZE_DESCRIPTOR
#define _UCSI_MPEG_FMXBUFFER_SIZE_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/common.h>

struct mpeg_fmxbuffer_size_descriptor {
	struct descriptor d;

	/* uint8_t descriptors[] */
} packed;

static inline struct mpeg_fmxbuffer_size_descriptor*
	mpeg_fmxbuffer_size_descriptor_parse(struct descriptor* d)
{
	return (struct mpeg_fmxbuffer_size_descriptor*) d;
}

static inline uint8_t *
	mpeg_fmxbuffer_size_descriptor_descriptors(struct mpeg_fmxbuffer_size_descriptor *d)
{
	return (uint8_t *) d + sizeof(struct mpeg_fmxbuffer_size_descriptor);
}

static inline int
	mpeg_fmxbuffer_size_descriptor_descriptors_length(struct mpeg_fmxbuffer_size_descriptor *d)
{
	return d->d.len;
}

#endif
