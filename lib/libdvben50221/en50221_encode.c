/*
	en50221 encoder An implementation for libdvb
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
#include "en50221_encode.h"

uint16_t en50221_encode_header(struct ca_msg * ca_msg,
			       struct en50221_pmt_object * pmt, uint16_t pos)
{
	ca_msg->msg[pos + 0] = pmt->ca_pmt_list_mgmt;
	ca_msg->msg[pos + 1] = pmt->program_number >> 8;
	ca_msg->msg[pos + 2] = pmt->program_number;
	ca_msg->msg[pos + 3] = pmt->reserved_1 << 6 |
			       pmt->version_number << 1 |
			       pmt->current_next;
	ca_msg->msg[pos + 4] = pmt->reserved_2 << 4 |
			     ((pmt->program_info_length >> 8) & 0x0f);
	ca_msg->msg[pos + 5] = pmt->program_info_length;

	pos += 6;
	printf("%s: CA PMT List Mgmt=[%d], Program Number=[%d], Program info length=[%d]\n", __FUNCTION__,
		pmt->ca_pmt_list_mgmt, pmt->program_number, pmt->program_info_length);

	return pos;
}

uint16_t en50221_encode_descriptor(struct ca_msg * ca_msg,
				   struct descriptor * desc, uint16_t pos)
{
	uint8_t i;
	uint16_t temp = 0, private_bytes = 0;
	temp = pos;

	ca_msg->msg[pos + 0] = desc->descriptor_tag;
	ca_msg->msg[pos + 1] = desc->descriptor_length;
	ca_msg->msg[pos + 2] = desc->ca.ca_system_id >> 8;
	ca_msg->msg[pos + 3] = desc->ca.ca_system_id;
	ca_msg->msg[pos + 4] = desc->ca.reserved << 5 |
			     ((desc->ca.ca_pid >> 8) & 0x1f);
	ca_msg->msg[pos + 5] = desc->ca.ca_pid;

	pos += 6;
	// parse private data
	printf("%s: Tag=[%02x], length=[%02x], CA system id=[%02x], CA PID=[%02x]\n", __FUNCTION__,
		desc->descriptor_tag, desc->descriptor_length, desc->ca.ca_system_id, desc->ca.ca_pid);

	if (desc->descriptor_length) {
		private_bytes = desc->descriptor_length - 4;

		printf("%s: Private Bytes=[%d] [ ", __FUNCTION__, private_bytes);
		for (i = 0; i < private_bytes; i++) {
			ca_msg->msg[pos + i] = desc->ca.p_private_data_byte[i];
			printf("%02x ", ca_msg->msg[pos + i]);
		}
		printf("]\n");
	}
	pos = pos + i;

	return pos;
}


uint16_t encode_ca_pmt_command(struct ca_msg * ca_msg, void * encode_object,
			       uint16_t pos, uint8_t scrambling)
{
	if (scrambling == PROGRAM_SCRAMBLED) {
		struct en50221_pmt_object * program_encode_object;
		program_encode_object = encode_object;
		printf("%s: Encoding SCRAMBLING @ PROGRAM Level, Command=[%02x]\n",
			__FUNCTION__, program_encode_object->ca_pmt_cmd_id);
		ca_msg->msg[pos + 0] = program_encode_object->ca_pmt_cmd_id;

	} else if (scrambling == STREAMS_SCRAMBLED) {
		struct en50221_stream * streams_encode_object;
		streams_encode_object = encode_object;
		printf("%s: Encoding SCRAMBLING @ STREAMS Level, Command=[%02x]\n",
			__FUNCTION__, streams_encode_object->ca_pmt_cmd_id);
		ca_msg->msg[pos + 0] = streams_encode_object->ca_pmt_cmd_id;
	}

	return pos + 1;
}

uint16_t en50221_encode_streams(struct ca_msg * ca_msg,
				struct en50221_stream * stream, uint16_t pos)
{
	int i, es_info_len_pos, es_info_len;

	ca_msg->msg[pos + 0] = stream->stream_type;
	ca_msg->msg[pos + 1] = stream->reserved_1 << 5 |
			     ((stream->elementary_pid >> 8) & 0x1f);
	ca_msg->msg[pos + 2] = stream->elementary_pid & 0xff;

	es_info_len_pos = pos + 3;
	pos += 5;

	printf("%s: Stream type=[%02x], ES PID=[%02x], ES Info length=[%02x]\n", __FUNCTION__,
		stream->stream_type, stream->elementary_pid,
		stream->es_info_length);

	if(stream->streams_desc_count) {
		pos = encode_ca_pmt_command(ca_msg, stream, pos, STREAMS_SCRAMBLED);
	}

	for (i = 0; i < stream->streams_desc_count; i++) {
		struct descriptor * stream_desc = &stream->p_descriptors[i];
		pos = en50221_encode_descriptor(ca_msg, stream_desc, pos);
	}

	es_info_len = pos - (es_info_len_pos + 2);

	ca_msg->msg[es_info_len_pos+0] = stream->reserved_2 << 4 |
				       ((es_info_len >> 8) & 0x0f);
	ca_msg->msg[es_info_len_pos+1] = es_info_len & 0xff;

	return pos;
}
