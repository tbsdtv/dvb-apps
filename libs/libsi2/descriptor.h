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

#ifndef _SI_DESCRIPTOR_H
#define _SI_DESCRIPTOR_H 1

#include <si/common.h>

#include <stdint.h>
#include <stdlib.h>

#define packed __attribute__((packed))

#if 0
#define EBIT2(x1,x2) x1 x2
#define EBIT3(x1,x2,x3) x1 x2 x3
#define EBIT4(x1,x2,x3,x4) x1 x2 x3 x4
#define EBIT5(x1,x2,x3,x4,x5) x1 x2 x3 x4 x5
#define EBIT6(x1,x2,x3,x4,x5,x6) x1 x2 x3 x4 x5 x6
#else
#define EBIT2(x1,x2) x2 x1
#define EBIT3(x1,x2,x3) x3 x2 x1
#define EBIT4(x1,x2,x3,x4) x4 x3 x2 x1
#define EBIT5(x1,x2,x3,x4,x5) x5 x4 x3 x2 x1
#define EBIT6(x1,x2,x3,x4,x5,x6) x6 x5 x4 x3 x2 x1
#endif

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
extern int verify_descriptors(uint8_t * buf, int len);

#endif
