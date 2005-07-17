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

#ifndef _SI_SECTION_H
#define _SI_SECTION_H 1

#include <si/common.h>
#include <si/descriptor.h>

#include <stdint.h>

#define CRC_SIZE 4

/* section heads */

struct section {
	uint8_t	table_id;
  EBIT4(uint16_t syntax_indicator	: 1; ,
	uint16_t private_indicator	: 1; , /* 2.4.4.10 */
	uint16_t reserved 		: 2; ,
	uint16_t length			:12; );
} packed;

struct section_ext {
	uint8_t	table_id;
  EBIT4(uint16_t syntax_indicator	: 1; ,
	uint16_t private_indicator	: 1; , /* 2.4.4.10 */
	uint16_t reserved 		: 2; ,
	uint16_t length			:12; );

	uint16_t table_id_ext;
  EBIT3(uint8_t reserved1 		: 2; ,
	uint8_t version_number		: 5; ,
	uint8_t current_next_indicator	: 1; );
	uint8_t section_number;
	uint8_t last_section_number;
} packed;

/**
 * All parsers relies on this entry point, where the buffer must be
 * valid throughout the use of struct section * and any other
 * structures retrived using this pointer.
 */

struct section * parse_section(uint8_t * buf, int len);
struct section_ext * parse_section_ext(struct section *, int check_crc);

static inline int section_length(struct section *section)
{
	return section->length + sizeof(struct section);
}

static inline int section_ext_length(struct section_ext * section)
{
	return section->length + sizeof(struct section) - CRC_SIZE;
}

#endif
