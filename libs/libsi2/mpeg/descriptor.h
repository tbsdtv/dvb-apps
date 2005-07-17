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

#ifndef _SI_MPEG_DESCRIPTOR_H
#define _SI_MPEG_DESCRIPTOR_H 1

#include <si/mpeg/mpeg4_audio_descriptor.h>
#include <si/mpeg/mpeg4_video_descriptor.h>
#include <si/mpeg/audio_stream_descriptor.h>
#include <si/mpeg/ca_descriptor.h>
#include <si/mpeg/copyright_descriptor.h>
#include <si/mpeg/data_stream_alignment_descriptor.h>
#include <si/mpeg/external_es_id_descriptor.h>
#include <si/mpeg/fmc_descriptor.h>
#include <si/mpeg/fmxbuffer_size_descriptor.h>
#include <si/mpeg/hierarchy_descriptor.h>
#include <si/mpeg/ibp_descriptor.h>
#include <si/mpeg/iod_descriptor.h>
#include <si/mpeg/iso_639_language_descriptor.h>
#include <si/mpeg/maximum_bitrate_descriptor.h>
#include <si/mpeg/multiplex_buffer_descriptor.h>
#include <si/mpeg/multiplex_buffer_utilization_descriptor.h>
#include <si/mpeg/muxcode_descriptor.h>
#include <si/mpeg/private_data_indicator_descriptor.h>
#include <si/mpeg/registration_descriptor.h>
#include <si/mpeg/sl_descriptor.h>
#include <si/mpeg/smoothing_buffer_descriptor.h>
#include <si/mpeg/std_descriptor.h>
#include <si/mpeg/system_clock_descriptor.h>
#include <si/mpeg/target_background_grid_descriptor.h>
#include <si/mpeg/video_stream_descriptor.h>
#include <si/mpeg/video_window_descriptor.h>
#include <si/common.h>

enum mpeg_descriptor_tag {
	dtag_mpeg_video_stream			= 0x02,
	dtag_mpeg_audio_stream			= 0x03,
	dtag_mpeg_hierarchy			= 0x04,
	dtag_mpeg_registration			= 0x05,
	dtag_mpeg_data_stream_alignment		= 0x06,
	dtag_mpeg_target_background_grid	= 0x07,
	dtag_mpeg_video_window			= 0x08,
	dtag_mpeg_ca				= 0x09,
	dtag_mpeg_iso_639_language		= 0x0a,
	dtag_mpeg_system_clock			= 0x0b,
	dtag_mpeg_multiplex_buffer_utilization	= 0x0c,
	dtag_mpeg_copyright			= 0x0d,
	dtag_mpeg_maximum_bitrate		= 0x0e,
	dtag_mpeg_private_data_indicator	= 0x0f,
	dtag_mpeg_smoothing_buffer		= 0x10,
	dtag_mpeg_std				= 0x11,
	dtag_mpeg_ibp				= 0x12,
	dtag_mpeg_4_video			= 0x1b,
	dtag_mpeg_4_audio			= 0x1c,
	dtag_mpeg_iod				= 0x1d,
	dtag_mpeg_sl				= 0x1e,
	dtag_mpeg_fmc				= 0x1f,
	dtag_mpeg_external_es_id		= 0x20,
	dtag_mpeg_muxcode			= 0x21,
	dtag_mpeg_fmxbuffer_size		= 0x22,
	dtag_mpeg_multiplex_buffer		= 0x23,
};

#endif
