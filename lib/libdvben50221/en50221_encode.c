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

uint16_t en50221_encode_header(struct ca_msg *p_ca_msg, struct en50221_pmt_object *p_en50221_pmt_object, uint16_t pos)
{
	uint8_t i, program_desc_count = 0;

	p_ca_msg->msg[pos + 0] = p_en50221_pmt_object->ca_pmt_list_mgmt;		// list management
	p_ca_msg->msg[pos + 1] = (p_en50221_pmt_object->program_number >> 8) & 0xff;	// prog no MSB
	p_ca_msg->msg[pos + 2] = p_en50221_pmt_object->program_number & 0xff;	// prog no LSB
	p_ca_msg->msg[pos + 3] = (p_ca_msg->msg[pos + 3] | p_en50221_pmt_object->reserved_1)  << 6;
	p_ca_msg->msg[pos + 3] = (p_ca_msg->msg[pos + 3] | p_en50221_pmt_object->version_number) << 2;
	p_ca_msg->msg[pos + 3] = p_ca_msg->msg[pos + 3] | p_en50221_pmt_object->current_next;
	p_ca_msg->msg[pos + 4] = (p_ca_msg->msg[pos + 4] | p_en50221_pmt_object->reserved_2) << 4;
	p_ca_msg->msg[pos + 4] = (p_ca_msg->msg[pos + 4] | (p_en50221_pmt_object->program_info_length >> 8) & 0x0f);
	p_ca_msg->msg[pos + 5] = p_ca_msg->msg[pos + 5] | (p_en50221_pmt_object->program_info_length & 0xff);

	pos += 6;
	printf("%s: CA PMT List Mgmt=[%d], Program Number=[%d], Program info length=[%d]\n", __FUNCTION__,
		p_en50221_pmt_object->ca_pmt_list_mgmt, p_en50221_pmt_object->program_number, p_en50221_pmt_object->program_info_length);

	return pos;
}

uint16_t en50221_encode_descriptor(struct ca_msg *p_ca_msg, struct ca_descriptor *p_desc, uint16_t pos)
{
	uint8_t i;
	uint16_t temp = 0, private_bytes = 0;
	temp = pos;

	p_ca_msg->msg[pos + 0] = p_desc->descriptor_tag;
	p_ca_msg->msg[pos + 1] = p_desc->descriptor_length;
	p_ca_msg->msg[pos + 2] = (p_desc->ca_system_id >> 8) & 0xff;
	p_ca_msg->msg[pos + 3] = p_desc->ca_system_id & 0xff;
	p_ca_msg->msg[pos + 4] = ((p_ca_msg->msg[pos + 4] | p_desc->reserved) << 5) | ((p_desc->ca_pid >> 8) & 0x07);
	p_ca_msg->msg[pos + 5] = p_ca_msg->msg[pos + 5] | (p_desc->ca_pid & 0xff);

	pos += 6;
	// parse private data
	printf("%s: Tag=[%02x], length=[%02x], CA system id=[%02x], CA PID=[%02x]\n", __FUNCTION__,
		p_desc->descriptor_tag, p_desc->descriptor_length, p_desc->ca_system_id, p_desc->ca_pid);

	if (p_desc->descriptor_length) {
		private_bytes = p_desc->descriptor_length - 4;

		printf("%s: Private Bytes=[%d] [ ", __FUNCTION__, private_bytes);
		for (i = 0; i < private_bytes; i++) {
			p_ca_msg->msg[pos + i] = p_desc->p_private_data_byte[i];
			printf("%02x ", p_ca_msg->msg[pos + i]);
		}
		printf("]\n");
	}
	pos = pos + i;

	return pos;
}


uint16_t encode_ca_pmt_command(struct ca_msg *p_ca_msg, void *p_encode_object, uint16_t pos, uint8_t scrambling)
{
	if (scrambling == PROGRAM_SCRAMBLED) {
		struct en50221_pmt_object *p_program_encode_object = (struct en50221_pmt_object *) p_encode_object;
		printf("%s: Encoding SCRAMBLING @ PROGRAM Level, Command=[%02x]\n", __FUNCTION__, p_program_encode_object->ca_pmt_cmd_id);
		p_ca_msg->msg[pos + 0] = p_program_encode_object->ca_pmt_cmd_id;

	} else if (scrambling == STREAMS_SCRAMBLED) {
		struct en50221_stream *p_streams_encode_object = (struct en50221_stream *) p_encode_object;
		printf("%s: Encoding SCRAMBLING @ STREAMS Level, Command=[%02x]\n", __FUNCTION__, p_streams_encode_object->ca_pmt_cmd_id);
		p_ca_msg->msg[pos + 0] = p_streams_encode_object->ca_pmt_cmd_id;
	}

	return pos + 1;
}

uint16_t en50221_encode_streams(struct ca_msg *p_ca_msg, struct en50221_stream *p_stream, uint16_t pos)
{
	uint8_t i, stream_desc_count = 0;
	p_ca_msg->msg[pos + 0] = p_stream->stream_type;

	p_ca_msg->msg[pos + 1] = ((p_ca_msg->msg[pos + 1] | p_stream->reserved_1) << 5) | ((p_stream->elementary_pid >> 8) & 0x1f);
	p_ca_msg->msg[pos + 2] = p_ca_msg->msg[pos + 2] | (p_stream->elementary_pid & 0xff);
	p_ca_msg->msg[pos + 3] = ((p_ca_msg->msg[pos + 3] | p_stream->reserved_2) << 4) | ((p_stream->es_info_length >> 8) & 0x0f);
	p_ca_msg->msg[pos + 4] = p_ca_msg->msg[pos + 4] | (p_stream->es_info_length & 0xff);

	printf("%s: Stream type=[%02x], ES PID=[%02x], ES Info length=[%02x]\n", __FUNCTION__,
		p_stream->stream_type, p_stream->elementary_pid, p_stream->es_info_length);
	pos += 5;
	// parse descriptor
	if (p_stream->es_info_length) {
		// where is ca_pmt_cmd_id ?
		for (i = 0; i < p_stream->streams_desc_count; i++) {
			struct ca_descriptor *p_stream_desc = (struct ca_descriptor *) &p_stream->p_descriptor[i];
			pos = encode_ca_pmt_command(p_ca_msg, p_stream, pos, STREAMS_SCRAMBLED);
			pos = en50221_encode_descriptor(p_ca_msg, p_stream_desc, pos);
			stream_desc_count++;
			if (stream_desc_count != p_stream->streams_desc_count) {
				printf("%s: Error ! stream_desc_count=[%d], p_stream->streams_desc_count=[%d]\n",
					__FUNCTION__, stream_desc_count, p_stream->streams_desc_count);
				exit(-1);
			}
		}
	}

	return pos;
}
