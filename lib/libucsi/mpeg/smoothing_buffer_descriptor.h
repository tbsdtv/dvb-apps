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

#ifndef _UCSI_MPEG_SMOOTHING_BUFFER_DESCRIPTOR
#define _UCSI_MPEG_SMOOTHING_BUFFER_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/common.h>

struct mpeg_smoothing_buffer_descriptor {
	struct descriptor d;

  EBIT4(uint64_t reserved_1  		: 2; ,
	uint64_t sb_leak_rate		:22; ,
	uint64_t reserved_2		: 2; ,
	uint64_t sb_size		:22; );
} packed;

static inline struct mpeg_smoothing_buffer_descriptor*
	mpeg_smoothing_buffer_descriptor_parse(struct descriptor* d)
{
	if (d->len != (sizeof(struct mpeg_smoothing_buffer_descriptor) - 2))
		return NULL;

	bswap48((uint8_t*) d + 2);

	return (struct mpeg_smoothing_buffer_descriptor*) d;
}

#endif
