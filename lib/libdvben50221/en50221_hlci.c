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
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/dvb/ca.h>
#include "pmt.h"
#include "en50221_hlci.h"
#include "asn_1.h"
#include "en50221_encode.h"
#include "descriptor.h"

static uint16_t copy_en50221_descriptor_object(struct ca_descriptor *p_en50221_descriptor, struct ca_descriptor *p_descriptor)
{
	uint8_t i;
	uint16_t object_length = 0;
	uint16_t private_bytes = 0;

	p_en50221_descriptor->descriptor_tag = p_descriptor->descriptor_tag;
	p_en50221_descriptor->descriptor_length = p_descriptor->descriptor_length;
	p_en50221_descriptor->ca_system_id = p_descriptor->ca_system_id;
	p_en50221_descriptor->reserved = p_descriptor->reserved;
	p_en50221_descriptor->ca_pid = p_descriptor->ca_pid;

	printf("\t%s: Tag=[%02x], Length=[%02x], CA system ID=[%02x], CA PID=[%02x]\n", __FUNCTION__,
		p_en50221_descriptor->descriptor_tag, p_en50221_descriptor->descriptor_length,
		p_en50221_descriptor->ca_system_id, p_en50221_descriptor->ca_pid);

	object_length += (8 + 8 + 16 + 3 + 13);

	private_bytes = p_descriptor->descriptor_length - 4;

	printf("%s: Private bytes=[%d] [ ", __FUNCTION__, private_bytes);
	uint8_t *p_en50221_priv = (uint8_t *) malloc(sizeof (uint8_t) * private_bytes);
	if (p_en50221_priv != NULL) {

		for (i = 0; i < private_bytes; i++) {
			p_en50221_priv[i] = p_descriptor->p_private_data_byte[i];
			printf("%02x ", p_en50221_priv[i]);
		}
		printf("]\n");

		object_length += (private_bytes * 8);
		p_en50221_descriptor->p_private_data_byte = p_en50221_priv;
	}
	else {
		printf("%s: Memory Allocation failed.\n", __FUNCTION__);
		return 0;
	}

	return object_length;
}

static uint16_t copy_en50221_stream_object(struct en50221_stream *p_en50221_stream, struct streams *p_streams)
{
	uint16_t descriptor_count = 0, object_length = 0;
	struct ca_descriptor *p_descriptor = NULL;

	p_en50221_stream->stream_type = p_streams->stream_type;
	p_en50221_stream->reserved_1 = p_streams->reserved_1;
	p_en50221_stream->elementary_pid = p_streams->elementary_pid;
	p_en50221_stream->reserved_2 = p_streams->reserved_2;
	p_en50221_stream->es_info_length = p_streams->es_info_length;
	p_en50221_stream->streams_desc_count = p_streams->streams_desc_count;

	printf("%s: Stream Type=[%d], Elementary PID=[%d], ES length=[%d], Number of descriptors=[%d]\n", __FUNCTION__,
		p_streams->stream_type,  p_streams->elementary_pid, p_streams->es_info_length, p_streams->streams_desc_count);

	object_length += (8 + 3 + 13 + 4 + 12);
	// Now each stream has descriptors
	struct ca_descriptor *p_en50221_streams_desc =
			(struct ca_descriptor *) malloc(sizeof (struct ca_descriptor) * p_streams->streams_desc_count);
	if (p_en50221_streams_desc != NULL) {

		for (descriptor_count = 0; descriptor_count < p_streams->streams_desc_count; descriptor_count++) {
			struct ca_descriptor *p_en50221_stream_desc = &p_en50221_streams_desc[descriptor_count];
			p_descriptor = p_streams->p_descriptor;
			struct ca_descriptor *p_stream_desc = &p_descriptor[descriptor_count];

			if (p_stream_desc->descriptor_tag == 0x09)	// Copy only CA descriptors
				object_length += copy_en50221_descriptor_object(p_en50221_stream_desc, p_stream_desc);
		}
	}
	else {
		printf("%s: Memory allocation failed.\n", __FUNCTION__);
		return 0;
	}

	return object_length;
}


static uint16_t copy_en50221_pmt_object(struct service_info *p_si, struct en50221_pmt_object *p_en50221_pmt_object)
{
	int i, stream_count = 0, ca_descriptors = 0, object_length = 0;

	p_en50221_pmt_object->p_en50221_prog_desc = NULL;

	//	Program header
	p_en50221_pmt_object->program_number = p_si->p_pmt->program_number;
	p_en50221_pmt_object->reserved_1 = p_si->p_pmt->reserved_1;
	p_en50221_pmt_object->version_number = p_si->p_pmt->version_number;
	p_en50221_pmt_object->current_next = p_si->p_pmt->current_next;
	p_en50221_pmt_object->reserved_2 = p_si->p_pmt->reserved_2;
	p_en50221_pmt_object->program_info_length = p_si->p_pmt->program_info_length;

	printf("\n%s: Copying EN50221 Header\n", __FUNCTION__);
	printf("%s: Program Number=[%d], Program info length=[%d]\n", __FUNCTION__,
			p_en50221_pmt_object->program_number, p_en50221_pmt_object->program_info_length);

	p_en50221_pmt_object->program_desc_count = p_si->p_pmt->program_desc_count;
	printf("%s: Program level descriptor count=[%d]\n", __FUNCTION__, p_en50221_pmt_object->program_desc_count);

	object_length += (16 + 2 + 5 + 1 + 4 + 12);

	//	Program descriptor
	struct ca_descriptor *p_prog_desc = NULL;
	struct ca_descriptor *p_en50221_descriptor = NULL;
	struct ca_descriptor *p_descriptor_objects = NULL;

	p_descriptor_objects = (struct ca_descriptor *) malloc(sizeof (struct ca_descriptor) * p_en50221_pmt_object->program_desc_count);
	if (p_descriptor_objects == NULL) {
		printf("%s: Memory allocation failed\n", __FUNCTION__);
		return 0;
	}

	p_prog_desc = (struct ca_descriptor*)p_si->p_pmt->p_descriptor;
	for (i = 0; i < p_si->p_pmt->program_desc_count; ++i, ++p_prog_desc) {
		printf("%s: CA descriptor=[%02x] found, @ [%p], descriptor length=[%02x]\n", __FUNCTION__,
				p_prog_desc->descriptor_tag, &p_prog_desc, p_prog_desc->descriptor_length);

		if (p_prog_desc->descriptor_tag == 0x09) {	// Copy only CA descriptors
			p_en50221_descriptor = (struct ca_descriptor *) &p_descriptor_objects[ca_descriptors];
			object_length += copy_en50221_descriptor_object(p_en50221_descriptor, p_prog_desc);
			printf("%s: [%d] CA descriptor copied\n", __FUNCTION__, ca_descriptors);
			ca_descriptors++;

		}
	}
	p_en50221_pmt_object->p_en50221_prog_desc = p_descriptor_objects;
	p_en50221_pmt_object->stream_count = p_si->p_pmt->stream_count;

	//	Streams
	struct en50221_stream *p_streams = (struct en50221_stream *) malloc(sizeof (struct en50221_stream) * p_si->p_pmt->stream_count);

	if (p_streams != NULL) {
		while (stream_count < p_si->p_pmt->stream_count) {
			struct en50221_stream *p_en50221_stream = (struct en50221_stream *) &p_streams[stream_count];
			struct streams *p_streams = (struct streams *) &p_si->p_pmt->p_streams[stream_count];
			p_streams->p_descriptor = NULL;
			object_length += copy_en50221_stream_object(p_en50221_stream, p_streams);
			stream_count++;
		}
		p_en50221_pmt_object->p_en50221_streams = p_streams;
	}
	else {
		printf("%s: Memory allocation failed.\n", __FUNCTION__);
		return 0;
	}

	return object_length;
}


uint16_t set_pmt_command(struct en50221_pmt_object *p_en50221_pmt_object, uint8_t pmt_command)
{
	uint16_t object_length = 0;
	uint16_t i = 0, j = 0;

	printf("%s: Setting PMT Command\n", __FUNCTION__);
	for (i = 0; i < p_en50221_pmt_object->program_desc_count; i++) {
		if (p_en50221_pmt_object->p_en50221_prog_desc[i].descriptor_tag == 0x09) {
			printf("%s: CA descriptor found @ PROGRAM Level, Setting CA PMT command=[%02x]\n", __FUNCTION__, pmt_command);
			p_en50221_pmt_object->ca_pmt_cmd_id = pmt_command;
			object_length += 8;
		}
	}

	for (i = 0; i < p_en50221_pmt_object->stream_count; i++) {
		for (j = 0; j < p_en50221_pmt_object->p_en50221_streams[i].streams_desc_count; j++) {
			if (p_en50221_pmt_object->p_en50221_streams[i].p_descriptor->descriptor_tag == 0x09) {
				printf("%s: CA descriptor found @ STREAMS Level, Setting CA PMT command=[%02x]\n", __FUNCTION__, pmt_command);
				p_en50221_pmt_object->p_en50221_streams[i].ca_pmt_cmd_id = pmt_command;
				object_length += 8;
			}
		}
	}


	return object_length;
}


uint16_t do_en50221_pmt_object(struct en50221_pmt_object *p_en50221_pmt_object, struct service_info *p_si, uint8_t pmt_list_mgmt, uint8_t pmt_command)
{
	uint16_t object_length = 0;
	uint32_t asn_1_words = 0;
	uint8_t i;

	object_length += copy_en50221_pmt_object(p_si, p_en50221_pmt_object);
	p_en50221_pmt_object->ca_pmt_tag = CA_PMT;	// this is ok, as we are doing ca pmt only

	p_en50221_pmt_object->ca_pmt_list_mgmt = pmt_list_mgmt;
	object_length += 8;
	printf("%s: CA PMT List Management=[%02x]\n",__FUNCTION__, p_en50221_pmt_object->ca_pmt_list_mgmt);

	// check whether scrambling @program/stream level
	object_length += set_pmt_command(p_en50221_pmt_object, pmt_command);
	printf("%s: Object length=[%d], Total length=[%d]\n", __FUNCTION__, object_length, (object_length / 8));
	p_en50221_pmt_object->asn_1_length = asn_1_encode((object_length / 8), &asn_1_words);	// convert to bytes

	printf("%s: ASN.1 words=[%u], Length Array=[ ", __FUNCTION__, asn_1_words);
	for (i = 0; i < asn_1_words; i++)
		printf(" %d ", p_en50221_pmt_object->asn_1_length[i]);
	printf("]\n");


	return 0;
}

uint16_t write_to_slot(struct ca_msg *p_ca_msg, char *ca_dev)
{
	int ca_fd = 0;

	if ((ca_fd = open(ca_dev, O_RDONLY)) < 0) {
		printf("%s: Cannot open CA slot.\n", __FUNCTION__);
		return -1;
	}
	if (ioctl(ca_fd, CA_SEND_MSG, p_ca_msg)) {
		printf("%s: Cannot send CA message.\n", __FUNCTION__);
		return -1;
	}
	close(ca_fd);

	return 0;
}

uint16_t debug_message(struct ca_msg *p_ca_msg, uint16_t pos)
{
	uint16_t i;

	printf("%s: CA MESSAGE=[ ", __FUNCTION__);
	for (i = 0; i < pos; i++)
		printf("%02x ", p_ca_msg->msg[i]);
	printf(" ]\n");

	return 0;
}

uint16_t write_en50221_pmt_object(struct en50221_pmt_object *p_en50221_pmt_object, char *ca_dev)
{
	uint32_t object_length = 0, temp = 0;
	struct ca_msg *p_ca_msg;
	uint8_t i, words = 0;
	uint16_t pos = 0;
	struct ca_descriptor *p_desc = NULL;
	struct en50221_stream *p_en50221_stream = NULL;

	p_ca_msg = (struct ca_msg *) malloc(sizeof (struct ca_msg));
	if (p_ca_msg == NULL)
		return 0; // -ENOMEM

	memset(p_ca_msg, 0, sizeof(struct ca_msg));

	// get length of the object
	object_length = asn_1_decode(p_en50221_pmt_object->asn_1_length);

	// write tag
	temp = p_en50221_pmt_object->ca_pmt_tag;
	for (i = 3; i > 0; i--) {
		p_ca_msg->msg[i - 1] = temp & 0xff;
		temp = temp >> 8;
		pos++;
	}

	// write length
	temp = object_length;
	while (temp) {
		temp = temp >> 8;
		p_ca_msg->msg[pos] = p_en50221_pmt_object->asn_1_length[words];
		words++, pos++;
	}
	pos = en50221_encode_header(p_ca_msg, p_en50221_pmt_object, pos);
	printf("%s: EN50221 header encoded\n", __FUNCTION__);

	for (i = 0; i < p_en50221_pmt_object->program_desc_count; i++) {
		printf("%s: Program level descriptor @ [%p], count=[%d], tag=[%02x], length=[%02x]\n", __FUNCTION__,
				&p_en50221_pmt_object->p_en50221_prog_desc, p_en50221_pmt_object->program_desc_count,
				p_en50221_pmt_object->p_en50221_prog_desc[i].descriptor_tag,
				p_en50221_pmt_object->p_en50221_prog_desc[i].descriptor_length);
		p_desc = (struct ca_descriptor *) &p_en50221_pmt_object->p_en50221_prog_desc[i];

		if (p_desc->descriptor_length) {
			// where is the ca_pmt_cmd_id ?
			pos = encode_ca_pmt_command(p_ca_msg, p_en50221_pmt_object, pos, PROGRAM_SCRAMBLED);
			pos = en50221_encode_descriptor(p_ca_msg, p_desc, pos);
		} else
			printf("%s: Descriptor length=[%d]\n", __FUNCTION__, p_desc->descriptor_length);
	}

	for (i = 0; i < p_en50221_pmt_object->stream_count; i++) {
		p_en50221_stream = (struct en50221_stream *) &p_en50221_pmt_object->p_en50221_streams[i];
		pos = en50221_encode_streams(p_ca_msg, p_en50221_stream, pos);
	}

	if (object_length > 253) {
		printf("%s: Hey, we need to chain APDU's !\n", __FUNCTION__);
		// do chaining of APDU's here
		// breakup size = 255 - (L_APDU_TAG/M_APDU_TAG + length_field)
		// L_APDU_TAG = 0x80, M_APDU_TAG = 0x00
	}

	debug_message(p_ca_msg, pos);
	write_to_slot(p_ca_msg, ca_dev);

	return 0;
}
