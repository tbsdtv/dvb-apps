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

#ifndef _UCSI_MPEG_SECTION_H
#define _UCSI_MPEG_SECTION_H 1

#include <ucsi/mpeg/cat_section.h>
#include <ucsi/mpeg/odsmt_section.h>
#include <ucsi/mpeg/pat_section.h>
#include <ucsi/mpeg/pmt_section.h>
#include <ucsi/mpeg/tsdt_section.h>

/**
 * Enumeration of MPEG section tags.
 */
enum mpeg_section_tag {
	sct_mpeg_program_association			= 0x00,
	sct_mpeg_conditional_access			= 0x01,
	sct_mpeg_program_map				= 0x02,
	sct_mpeg_transport_stream_description		= 0x03,
	sct_mpeg_iso14496_scene_description		= 0x04,
	sct_mpeg_iso14496_object_description		= 0x05,
};

#endif
