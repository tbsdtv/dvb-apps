/*
	libdvbsi, A SI parser implementation for libdvb
	an implementation for the High Level Common Interface

	Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as
	published by the Free Software Foundation; either version 2.1 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#ifndef __PAT_H__
#define __PAT_H__

#include <stdlib.h>
#include <stdint.h>


#define MAX_SECTION_SIZE	8192
#define MAX_PROGRAMS		200

struct program_descriptor {
	unsigned program_number: 16;
	unsigned reserved: 3;
	unsigned pid: 13;
};

struct pat {
	uint16_t pat_header_length;
	unsigned table_id: 8;
	unsigned section_syntax: 1;
	unsigned reserved_1: 2;
	unsigned section_length: 12;
	unsigned transport_stream_id: 16;
	unsigned reserved_2: 2;
	unsigned version_number: 5;
	unsigned current_next: 1;
	unsigned section_number: 8;
	unsigned last_section: 8;

	uint16_t program_count;
	struct program_descriptor *p_program_descriptor;
};

uint16_t parse_pat(struct pat *p_pat, uint8_t *buf, uint32_t pat_pid, int bytes);


#endif
