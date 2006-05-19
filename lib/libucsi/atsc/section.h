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

#include <libucsi/section.h>

#ifndef _UCSI_ATSC_SECTION_H
#define _UCSI_ATSC_SECTION_H 1

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Enumeration of ATSC section tags.
 */
enum atsc_section_tag {
	stag_atsc_master_guide					= 0xc7,
	stag_atsc_terrestrial_virtual_channel			= 0xc8,
	stag_atsc_cable_virtual_channel				= 0xc9,
	stag_atsc_rating_region					= 0xca,
	stag_atsc_event_informationen				= 0xcb,
	stag_atsc_extended_text					= 0xcc,
	stag_atsc_system_time					= 0xcd,
	stag_atsc_directed_channel_change			= 0xd3,
	stag_atsc_directed_channel_change_selection_code	= 0xd4,
};

/**
 * ATSC specific PSIP section structure.
 */
struct section_psip {
	struct section_ext		section_ext;
	uint8_t				protocol_version;
} __ucsi_packed;

/**
 * Decode a PSIP section structure.
 *
 * @param section_ext Pointer to the processed section_ext structure.
 * @return Pointer to the parsed section_psip structure, or NULL if invalid.
 */
static inline struct section_psip *section_psip_decode(struct section_ext *section_ext)
{
	size_t len = section_ext_length(section_ext);
	if (len < sizeof(struct section_psip)) {
		return NULL;
	}

	return (struct section_psip *) section_ext;
}

#ifdef __cplusplus
}
#endif

#endif
