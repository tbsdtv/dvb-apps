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

#include <stdio.h>
#include <unistd.h>

#include "pat.h"

static uint16_t parse_descriptor(struct program_descriptor *p_program_descriptor,
								uint8_t *buf, uint16_t pos)
{
	p_program_descriptor->program_number = (buf[pos + 0] << 8) | buf[pos + 1];
	p_program_descriptor->reserved = buf[pos + 2] >> 5;
	p_program_descriptor->pid = ((buf[pos + 2] & 0x1f) << 8) | buf[pos + 3];

//	printf("%s: Program Number=[%d], ", __FUNCTION__,
//		p_program_descriptor->program_number);

//	if (p_program_descriptor->program_number == 0)
//		printf("%s: Network PID=[%d]\n", __FUNCTION__, p_program_descriptor->pid);
//	else
//		printf("%s: PMT PID=[%d]\n", __FUNCTION__, p_program_descriptor->pid);

	return 4;
}

uint16_t parse_pat(struct pat *p_pat, uint8_t *buf, uint32_t pat_pid, int bytes)
{
	uint16_t pos = 0, length = 0, program_count = 0;
	printf("%s: ----------------->parse PAT section\n", __FUNCTION__);

	/*	PAT Header	*/
	p_pat->table_id = buf[pos + 0];
	p_pat->section_syntax =	(buf[pos + 1] & 0x80) >> 7;
	p_pat->reserved_1 = (buf[pos + 1] & 0xf0) >> 4;
	p_pat->section_length =	((buf[pos + 1] & 0x0f) << 8) | buf[pos + 2];
	p_pat->transport_stream_id = (buf[pos + 3] << 8) | buf[pos + 4];
	p_pat->reserved_2 = buf[pos + 5] >> 6;
	p_pat->version_number =	(buf[pos + 5] >> 1) & 0x1f;
	p_pat->current_next = buf[pos + 5] & 0x01;
	p_pat->section_number = buf[pos + 6];
	p_pat->last_section = buf[pos + 7];

	p_pat->pat_header_length = pos + 8;

	printf("%s: PAT => Section Length=[%d], TS ID=[%d]\n", __FUNCTION__,
		p_pat->section_length, p_pat->transport_stream_id);

	while ((length < (p_pat->section_length - 1)) && length < bytes) {
		length += parse_descriptor(&p_pat->p_program_descriptor[p_pat->program_count], buf, length);
		p_pat->program_count++;
		program_count = p_pat->program_count;
	}

	return 0;
}
