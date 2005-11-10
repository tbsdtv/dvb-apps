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

#ifndef _UCSI_ATSC_SECTION_H
#define _UCSI_ATSC_SECTION_H 1

/**
 * Enumeration of ATSC section tags.
 */
enum atsc_section_tag {
/* 0xC0-0xC6   [ATSC coordinated values which are defined in other standards.] */
	sct_master_guide				= 0xc7,
	sct_terrestrial_virtual_channel			= 0xc8,
	sct_cable_virtual_channel			= 0xc9,
	sct_rating_region				= 0xca,
	sct_event_informationen				= 0xcb,
	sct_extended_text				= 0xcc,
	sct_system_time					= 0xcd,

	sct_data_event					= 0xce,
	sct_data_service				= 0xcf,
	sct_network_resources				= 0xd1,
	sct_long_term_serivce				= 0xd2,
/* identical to DVB/ISO ?
	0x3F        DSM-CC Addressable Section Table
	0x3B        DSM-CC Section Table
	0x3C        DSM-CC Section Table */

	/* 0xCE-0xD2   [ATSC coordinated values which are defined in other standards.] */
	sct_directed_channel_change			= 0xd3,
	sct_directed_channel_change_selection_code	= 0xd4,

	/* 0xD5-0xDF   [ATSC coordinated values which are defined in other standards.] */
	sct_aggregate_event_information			= 0xd6,
	sct_aggregate_extended_text			= 0xd7,
	sct_satellie_virtual_channel			= 0xda,

	/* 0xE0-0xE5   [Used in other systems] */
	/* 0xE6-0xFE   [Reserved for future ATSC use] */
};

#endif
