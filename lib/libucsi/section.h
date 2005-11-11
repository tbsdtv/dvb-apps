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

#ifndef _UCSI_SECTION_H
#define _UCSI_SECTION_H 1

#include <ucsi/endianops.h>
#include <ucsi/descriptor.h>
#include <ucsi/crc32.h>
#include <stdint.h>
#include <string.h>

#define CRC_SIZE 4


/**
 * Generic section header.
 */
struct section {
	uint8_t	table_id;
  EBIT4(uint16_t syntax_indicator	: 1; ,
	uint16_t private_indicator	: 1; , /* 2.4.4.10 */
	uint16_t reserved 		: 2; ,
	uint16_t length			:12; );
} packed;

/**
 * Generic extended section header structure.
 */
struct section_ext {
	uint8_t	table_id;
  EBIT4(uint16_t syntax_indicator	: 1; ,
	uint16_t private_indicator	: 1; , /* 2.4.4.10 */
	uint16_t reserved		: 2; ,
	uint16_t length			:12; );

	uint16_t table_id_ext;
  EBIT3(uint8_t reserved1		: 2; ,
	uint8_t version_number		: 5; ,
	uint8_t current_next_indicator	: 1; );
	uint8_t section_number;
	uint8_t last_section_number;
} packed;

/**
 * Process a section structure in-place.
 *
 * @param buf Pointer to the data.
 * @param len Length of data.
 * @return Pointer to the section structure, or NULL if invalid.
 */
static inline struct section * section_codec(uint8_t * buf, int len)
{
	struct section * ret = (struct section *)buf;

	if (len < 3)
		return NULL;

	bswap16(buf+1);

	if (len != ret->length + 3)
		return NULL;

	return ret;
}

/**
 * Decode an extended section structure.
 *
 * @param section Pointer to the processed section structure.
 * @param check_crc If 1, the CRC of the section will also be checked.
 * @return Pointer to the parsed section_ext structure, or NULL if invalid.
 */
static inline struct section_ext * section_ext_decode(struct section * section,
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

/**
 * Encode an extended section structure for transmission.
 *
 * @param section Pointer to the section_ext structure.
 * @param update_crc If 1, the CRC of the section will also be updated.
 * @return Pointer to the encoded section_ext structure, or NULL if invalid.
 */
static inline struct section_ext * section_ext_encode(struct section_ext* section,
						      int update_crc)
{
	if (section->syntax_indicator == 0)
		return NULL;

	bswap16((uint8_t *)section + sizeof(struct section));

	if (update_crc) {
		uint8_t * buf = (uint8_t *) section;
		int len = sizeof(struct section) + section->length;
		uint32_t crc;

		/* zap the current CRC value */
		memset(buf+len-4, 0, 4);

		/* the crc has to be performed on the swapped data */
		bswap16(buf+1);
		crc = crc32(CRC32_INIT, buf, len);
		bswap16(buf+1);

		/* update the CRC */
		*((uint32_t*) (buf+len-4)) = crc;
	}

	return (struct section_ext *)section;
}

/**
 * Determine the total length of a section, including the header.
 *
 * @param section The parsed section structure.
 * @return The length.
 */
static inline int section_length(struct section *section)
{
	return section->length + sizeof(struct section);
}

/**
 * Determine the total length of an extended section, including the header,
 * but omitting the CRC.
 *
 * @param section The parsed section_ext structure.
 * @return The length.
 */
static inline int section_ext_length(struct section_ext * section)
{
	return section->length + sizeof(struct section) - CRC_SIZE;
}

#endif
