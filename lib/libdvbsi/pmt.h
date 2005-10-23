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

#ifndef __PMT_H__
#define __PMT_H__

#include <stdlib.h>
#include <stdint.h>


#define MAX_STREAMS		20
#define MAX_DESCRIPTORS		20

#define PROGRAM_SCRAMBLED	0x01
#define STREAMS_SCRAMBLED	0x02
#define SCRAMBLING_GIMMICK	0x03
#define SCRAMBLING_BOTH		0x04


struct streams {
	unsigned stream_type: 8;
	unsigned reserved_1: 3;
	unsigned elementary_pid: 13;
	unsigned reserved_2: 4;
	unsigned es_info_length: 12;

	uint8_t streams_desc_count;
	struct descriptor *p_descriptors;
};

struct pmt {
	uint16_t header_length;
	unsigned table_id: 8;
	unsigned section_syntax : 1;
	unsigned reserved_1: 2;
	unsigned section_length: 12;
	unsigned program_number: 16;
	unsigned reserved_2: 2;
	unsigned version_number: 5;
	unsigned current_next: 1;
	unsigned section_number: 8;
	unsigned last_section: 8;
	unsigned reserved_3: 3;
	unsigned pcr_pid: 13;
	unsigned reserved_4: 4;
	unsigned program_info_length: 12;

	uint8_t program_desc_count;
	struct descriptor *p_descriptors;

	uint8_t stream_count;
	struct streams *p_streams;
};

uint16_t parse_pmt(struct pmt *p_pmt, uint8_t *buf, uint32_t pmt_pid, int bytes);

#endif
