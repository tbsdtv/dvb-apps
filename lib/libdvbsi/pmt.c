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
#include "pmt.h"
#include "descriptor.h"


static int parse_pmt_header(struct pmt *p_pmt, uint8_t *buf)
{
	uint16_t pos = 0;

	p_pmt->header_length = 0;
	pos = p_pmt->header_length;
	p_pmt->table_id = buf[pos + 0];
	p_pmt->section_syntax = buf[pos + 1] & 0x80;
	p_pmt->reserved_1 = (buf[pos + 1] >> 4) & 0x03;
	p_pmt->section_length = 0;
	p_pmt->section_length =
		((p_pmt->section_length | (buf[pos + 1] & 0x0f)) << 8) | buf[pos + 2];

	p_pmt->program_number = 0;
	p_pmt->program_number =
		((p_pmt->program_number | buf[pos + 3]) << 8) | buf[pos + 4];

	p_pmt->reserved_2 = buf[pos + 5] >> 6;
	p_pmt->version_number = (buf[pos + 5] >> 1) & 0x1f;
	p_pmt->current_next = buf[pos + 5] & 0x01;

	p_pmt->section_number = buf[pos + 6];
	p_pmt->last_section = buf[pos + 7];
	p_pmt->reserved_3 = buf[pos + 8] >> 5;

	p_pmt->pcr_pid = (((p_pmt->pcr_pid | buf[pos + 8]) & 0x1f) << 8) | buf[pos + 9];

	p_pmt->reserved_4 = buf[pos + 10] >> 4;
	p_pmt->program_info_length =
		(((p_pmt->program_info_length | buf[pos + 10]) & 0x0f) << 8) | buf[pos + 11];

	printf("%s: Table ID=[%d], Section Length=[%d], Program Number=[%d], "
		"Section Number=[%d], PCR PID=[%d], Program info length=[%d]\n",
		__FUNCTION__, p_pmt->table_id, p_pmt->section_length,p_pmt->program_number,
		p_pmt->section_number,p_pmt->pcr_pid, p_pmt->program_info_length);

	pos += 12;
	p_pmt->header_length = pos;

	return pos;
}

static uint16_t parse_streams(struct streams *p_streams, uint8_t *buf,
					uint16_t stream_count, uint16_t pos, int bytes)
{
	uint16_t i;
	struct descriptor_info descr_info;

	p_streams->stream_type = buf[pos + 0];
	p_streams->reserved_1 = buf[pos + 1] >> 5;
	p_streams->elementary_pid = ((p_streams->elementary_pid | (buf[pos + 1] & 0x1f)) << 8) | buf[pos + 2];
	p_streams->reserved_2 = buf[pos + 3] >> 4;
	p_streams->es_info_length = 0;
	p_streams->es_info_length = ((p_streams->es_info_length | (buf[pos + 3] & 0x0f)) << 8) | buf[pos + 4];

	printf("\n\t%s: Elements=[", __FUNCTION__);
	for (i = 0; i < (p_streams->es_info_length + 5); i++)
		printf(" %02x", buf[pos + i]);
	printf("]\n");

	printf("\t%s: Stream=[%d], Stream Type=[%d], Elementary PID=[%d], ES info length=[%d]\n",
				__FUNCTION__, stream_count, p_streams->stream_type,
				p_streams->elementary_pid, p_streams->es_info_length);


	pos += 5;
	parse_descriptor_info(&descr_info, buf, pos);
	p_streams->p_descriptor = allocate_descriptor_storage(&descr_info);
	if (p_streams->es_info_length) {
		while (pos < (bytes - 4)) {
			pos = parse_descriptor(p_streams->p_descriptor, &descr_info, buf, pos);
		}
	}

	return pos;
}

uint16_t parse_pmt(struct pmt *p_pmt, uint8_t *buf, uint32_t pmt_pid, int bytes)
{
	uint16_t pos = 0;
	uint16_t i, count = 0;;

	struct descriptor_info descr_info;
	p_pmt->program_desc_count = 0;

	printf("%s: PMT Words=[ ", __FUNCTION__);
	for (i = 0; i < bytes; i++)
		printf("%02x ", buf[i]);
	printf("]\n");

	printf("\n%s: ----------->parse PMT section, PMT PID=[%d], bytes=[%d]\n", __FUNCTION__, pmt_pid, bytes);
	pos = parse_pmt_header(p_pmt, buf);	// PMT Header (12 bytes)
	printf("%s: Program info length=[%d]\n", __FUNCTION__, p_pmt->program_info_length);

	parse_descriptor_info(&descr_info, buf, pos);
	p_pmt->p_descriptor = allocate_descriptor_storage(&descr_info);

	for (count = 0; count < p_pmt->program_info_length; count++) {
		pos = parse_descriptor(p_pmt->p_descriptor, &descr_info, buf, pos);
		count += pos, p_pmt->program_desc_count++;
		printf("%s: Count=[%d], Position=[%d], Program descriptor count=[%d]\n", __FUNCTION__, count, pos, p_pmt->program_info_length);
	}

	p_pmt->p_streams = (struct streams *) malloc(sizeof (struct streams) * MAX_STREAMS);
	while (pos < (bytes - 4)) {	// we should not go more than the section length
		pos = parse_streams(&p_pmt->p_streams[p_pmt->stream_count], buf, p_pmt->stream_count, pos, bytes);
		p_pmt->stream_count++;
	}

	return 0;
}
