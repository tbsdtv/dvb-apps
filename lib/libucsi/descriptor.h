/*
 * section and descriptor parser
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
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

#ifndef _UCSI_DESCRIPTOR_H
#define _UCSI_DESCRIPTOR_H 1

#include <ucsi/common.h>

#include <stdint.h>
#include <stdlib.h>

struct descriptor {
	uint8_t tag;
	uint8_t len;
} packed;

static inline struct descriptor *
	next_descriptor(uint8_t * buf, int len, struct descriptor * pos)
{
	uint8_t* next;

	if (pos == NULL)
		return NULL;

	next = (uint8_t*) pos + 2 + pos->len;
	if (next >= buf + len)
		return NULL;

	return (struct descriptor *) next;
}




struct unknown_descriptor {
	struct descriptor d;

	/* uint8_t data [] */
} packed;

static inline uint8_t *
	unknown_descriptor_data(struct unknown_descriptor *d)
{
	return (uint8_t *) d + sizeof(struct unknown_descriptor);
}

static inline int
	unknown_descriptor_data_size(struct unknown_descriptor *d)
{
	return d->d.len;
}










/******************************** PRIVATE CODE ********************************/
static inline int verify_descriptors(uint8_t * buf, int len)
{
	int pos = 0;

	while (pos < len) {
		if ((pos + 2) > len)
			return -1;

		pos += 2 + buf[pos+1];
	}

	if (pos != len)
		return -1;

	return 0;
}

#endif
