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

#ifndef _SI_DVB_SECTION_H
#define _SI_DVB_SECTION_H 1

#include <si/dvb/bat_section.h>
#include <si/dvb/dit_section.h>
#include <si/dvb/eit_section.h>
#include <si/dvb/nit_section.h>
#include <si/dvb/rst_section.h>
#include <si/dvb/sdt_section.h>
#include <si/dvb/sit_section.h>
#include <si/dvb/st_section.h>
#include <si/dvb/tdt_section.h>
#include <si/dvb/tot_section.h>

enum dvb_section_tag {
	sct_network_information_actual			= 0x40,
	sct_network_information_other			= 0x41,

	sct_service_description_actual			= 0x42,
	sct_service_description_other			= 0x46,

	sct_bouquet_association				= 0x4a,

	sct_event_information_actual			= 0x4e,
	sct_event_information_other			= 0x4f,
	sct_event_information_actual_schedule		= 0x50,
	sct_event_information_other_schdule		= 0x60,
};

#endif
