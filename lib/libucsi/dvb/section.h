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

#ifndef _UCSI_DVB_SECTION_H
#define _UCSI_DVB_SECTION_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <ucsi/dvb/bat_section.h>
#include <ucsi/dvb/dit_section.h>
#include <ucsi/dvb/eit_section.h>
#include <ucsi/dvb/nit_section.h>
#include <ucsi/dvb/rst_section.h>
#include <ucsi/dvb/sdt_section.h>
#include <ucsi/dvb/sit_section.h>
#include <ucsi/dvb/st_section.h>
#include <ucsi/dvb/tdt_section.h>
#include <ucsi/dvb/tot_section.h>
#include <ucsi/dvb/int_section.h>

/**
 * Enumeration of DVB section tags.
 */
enum dvb_section_tag {
	stag_dvb_network_information_actual			= 0x40,
	stag_dvb_network_information_other			= 0x41,

	stag_dvb_service_description_actual			= 0x42,
	stag_dvb_service_description_other			= 0x46,

	stag_dvb_bouquet_association				= 0x4a,

	stag_dvb_ip_mac_notification				= 0x4c,

	stag_dvb_event_information_actual			= 0x4e,
	stag_dvb_event_information_other			= 0x4f,
	stag_dvb_event_information_actual_schedule		= 0x50,
	stag_dvb_event_information_other_schdule		= 0x60,
};

#ifdef __cplusplus
}
#endif

#endif
