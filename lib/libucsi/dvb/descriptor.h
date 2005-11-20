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

#ifndef _UCSI_DVB_DESCRIPTOR_H
#define _UCSI_DVB_DESCRIPTOR_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <ucsi/dvb/ac3_descriptor.h>
#include <ucsi/dvb/ancillary_data_descriptor.h>
#include <ucsi/dvb/announcement_support_descriptor.h>
#include <ucsi/dvb/bouquet_name_descriptor.h>
#include <ucsi/dvb/ca_identifier_descriptor.h>
#include <ucsi/dvb/ca_system_descriptor.h>
#include <ucsi/dvb/cable_delivery_descriptor.h>
#include <ucsi/dvb/cell_frequency_link_descriptor.h>
#include <ucsi/dvb/cell_list_descriptor.h>
#include <ucsi/dvb/component_descriptor.h>
#include <ucsi/dvb/content_descriptor.h>
#include <ucsi/dvb/country_availability_descriptor.h>
#include <ucsi/dvb/data_broadcast_descriptor.h>
#include <ucsi/dvb/data_broadcast_id_descriptor.h>
#include <ucsi/dvb/dsng_descriptor.h>
#include <ucsi/dvb/extended_event_descriptor.h>
#include <ucsi/dvb/frequency_list_descriptor.h>
#include <ucsi/dvb/linkage_descriptor.h>
#include <ucsi/dvb/local_time_offset_descriptor.h>
#include <ucsi/dvb/mosaic_descriptor.h>
#include <ucsi/dvb/multilingual_bouquet_name_descriptor.h>
#include <ucsi/dvb/multilingual_component_descriptor.h>
#include <ucsi/dvb/multilingual_network_name_descriptor.h>
#include <ucsi/dvb/multilingual_service_name_descriptor.h>
#include <ucsi/dvb/network_name_descriptor.h>
#include <ucsi/dvb/nvod_reference_descriptor.h>
#include <ucsi/dvb/parental_rating_descriptor.h>
#include <ucsi/dvb/partial_transport_stream_descriptor.h>
#include <ucsi/dvb/pdc_descriptor.h>
#include <ucsi/dvb/private_data_specifier_descriptor.h>
#include <ucsi/dvb/satellite_delivery_descriptor.h>
#include <ucsi/dvb/service_descriptor.h>
#include <ucsi/dvb/service_list_descriptor.h>
#include <ucsi/dvb/service_move_descriptor.h>
#include <ucsi/dvb/short_event_descriptor.h>
#include <ucsi/dvb/short_smoothing_buffer_descriptor.h>
#include <ucsi/dvb/stream_identifier_descriptor.h>
#include <ucsi/dvb/stuffing_descriptor.h>
#include <ucsi/dvb/subtitling_descriptor.h>
#include <ucsi/dvb/telephone_descriptor.h>
#include <ucsi/dvb/teletext_descriptor.h>
#include <ucsi/dvb/terrestrial_delivery_descriptor.h>
#include <ucsi/dvb/time_shifted_event_descriptor.h>
#include <ucsi/dvb/time_shifted_service_descriptor.h>
#include <ucsi/dvb/transport_stream_descriptor.h>
#include <ucsi/dvb/vbi_data_descriptor.h>
#include <ucsi/dvb/vbi_teletext_descriptor.h>
#include <ucsi/endianops.h>

/**
 * Enumeration of DVB descriptor tags.
 */
enum dvb_descriptor_tag {
	dtag_dvb_network_name			= 0x40,
	dtag_dvb_service_list			= 0x41,
	dtag_dvb_stuffing			= 0x42,
	dtag_dvb_satellite_delivery_system	= 0x43,
	dtag_dvb_cable_delivery_system		= 0x44,
	dtag_dvb_vbi_data			= 0x45,
	dtag_dvb_vbi_teletext			= 0x46,
	dtag_dvb_bouquet_name			= 0x47,
	dtag_dvb_service			= 0x48,
	dtag_dvb_country_availability		= 0x49,
	dtag_dvb_linkage			= 0x4a,
	dtag_dvb_nvod_reference			= 0x4b,
	dtag_dvb_time_shifted_service		= 0x4c,
	dtag_dvb_short_event			= 0x4d,
	dtag_dvb_extended_event			= 0x4e,
	dtag_dvb_time_shifted_event		= 0x4f,
	dtag_dvb_component			= 0x50,
	dtag_dvb_mosaic				= 0x51,
	dtag_dvb_stream_identifier		= 0x52,
	dtag_dvb_ca_identifier			= 0x53,
	dtag_dvb_content			= 0x54,
	dtag_dvb_parental_rating		= 0x55,
	dtag_dvb_teletext			= 0x56,
	dtag_dvb_telephone			= 0x57,
	dtag_dvb_local_time_offset		= 0x58,
	dtag_dvb_subtitling			= 0x59,
	dtag_dvb_terrestial_delivery_system	= 0x5a,
	dtag_dvb_multilingual_network_name	= 0x5b,
	dtag_dvb_multilingual_bouquet_name	= 0x5c,
	dtag_dvb_multilingual_service_name	= 0x5d,
	dtag_dvb_multilingual_component		= 0x5e,
	dtag_dvb_private_data_specifier		= 0x5f,
	dtag_dvb_service_move			= 0x60,
	dtag_dvb_short_smoothing_buffer		= 0x61,
	dtag_dvb_frequency_list			= 0x62,
	dtag_dvb_partial_transport_stream	= 0x63,
	dtag_dvb_data_broadcast			= 0x64,
	dtag_dvb_data_broadcast_id		= 0x66,
	dtag_dvb_transport_stream		= 0x67,
	dtag_dvb_dsng				= 0x68,
	dtag_dvb_pdc				= 0x69,
	dtag_dvb_ac3				= 0x6a,
	dtag_dvb_ancillary_data			= 0x6b,
	dtag_dvb_cell_list			= 0x6c,
	dtag_dvb_cell_frequency_link		= 0x6d,
	dtag_dvb_announcement_support		= 0x6e,
	dtag_dvb_application_signalling		= 0x6f,
	dtag_dvb_adaption_field_data		= 0x70,
	dtag_dvb_service_identifier		= 0x71,
	dtag_dvb_service_availability		= 0x72,
} packed;

#ifdef __cplusplus
}
#endif

#endif
