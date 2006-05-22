/*
 * section and descriptor parser
 *
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

#ifndef _UCSI_ATSC_TYPES_H
#define _UCSI_ATSC_TYPES_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <time.h>
#include <libucsi/types.h>

enum atsc_vct_modulation {
	ATSC_VCT_MODULATION_ANALOG 	= 0x01,
	ATSC_VCT_MODULATION_SCTE_MODE1 	= 0x02,
	ATSC_VCT_MODULATION_SCTE_MODE2 	= 0x03,
	ATSC_VCT_MODULATION_8VSB 	= 0x04,
	ATSC_VCT_MODULATION_16VSB 	= 0x05,
};

enum atsc_vct_service_type {
	ATSC_VCT_SERVICE_TYPE_ANALOG 	= 0x01,
	ATSC_VCT_SERVICE_TYPE_TV 	= 0x02,
	ATSC_VCT_SERVICE_TYPE_AUDIO 	= 0x03,
	ATSC_VCT_SERVICE_TYPE_DATA 	= 0x04,
};

enum atsc_etm_location {
	ATSC_VCT_ETM_NONE	 	= 0x00,
	ATSC_VCT_ETM_IN_THIS_PTC 	= 0x01,
	ATSC_VCT_ETM_IN_CHANNEL_TSID 	= 0x02,
};

typedef uint32_t atsctime_t;

struct atsc_text {
	uint8_t number_strings;
	/* struct atsc_text_string strings[] */
};

struct atsc_text_string {
	iso639lang_t language_code;
	uint8_t number_segments;
	/* struct atsc_text_string_segment segments[] */
};

struct atsc_text_string_segment {
	uint8_t compression_type;
	uint8_t mode;
	uint8_t number_bytes;
	/* uint8_t string_bytes[] */
};

extern int atsc_text_validate(uint8_t *buf, int len);

extern time_t atsctime_to_unixtime(atsctime_t atsc);
extern atsctime_t unixtime_to_atsctime(time_t atsc);

#ifdef __cplusplus
}
#endif

#endif
