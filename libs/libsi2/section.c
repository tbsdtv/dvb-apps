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

#include <section.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <stdio.h>
#include "crc32.h"

inline struct section * parse_section(uint8_t * buf, int len)
{
	struct section * ret = (struct section *)buf;

	if (len < 3)
		return NULL;

	bswap16(buf+1);

	if (len != ret->length + 3)
		return NULL;

	return ret;
}

inline struct section_ext * parse_section_ext(struct section * section,
                                              int check_crc)
{
	if (section->syntax_indicator == 0)
		return NULL;

	if (check_crc) {
		uint8_t * buf = (uint8_t *) section;
		int len = sizeof(struct section) + section->length;
		uint32_t crc;

		/* the crc check has to be performed on the unswapped data */
		bswap16(buf+1);
		crc = crc32(CRC32_INIT, buf, len);
		bswap16(buf+1);

		/* the crc check includes the crc value,
		 * the result should therefore be zero.
		 */
		if (crc)
			return NULL;
	}

	bswap16((uint8_t *)section + sizeof(struct section));

	return (struct section_ext *)section;
}

