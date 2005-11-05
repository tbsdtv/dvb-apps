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

static int compare_en50221_descriptor_object(struct descriptor* p_descriptor1,
					     struct descriptor* p_descriptor2)
{
	int i;

	// Check basic fields
	if (p_descriptor1->descriptor_tag != TAG_CA_DESCRIPTOR ||
	   p_descriptor2->descriptor_tag != p_descriptor2->descriptor_tag ||
	   p_descriptor1->descriptor_length != p_descriptor2->descriptor_length ||
	   p_descriptor1->ca.ca_system_id != p_descriptor2->ca.ca_system_id ||
	   p_descriptor1->ca.reserved != p_descriptor2->ca.reserved ||
	   p_descriptor1->ca.ca_pid != p_descriptor2->ca.ca_pid) {
		return 0;
	}

	// Check private data
	for (i = 0; i < p_descriptor1->descriptor_length - 4; i++) {
		if (p_descriptor1->ca.p_private_data_byte[i] != p_descriptor2->ca.p_private_data_byte[i]) {
			return 0;
		}
	}

	// Everything looked ok
	return 1;
}

static uint16_t copy_en50221_descriptor_object(struct descriptor *p_en50221_descriptor, struct descriptor *p_descriptor)
{
	uint8_t i;
	uint16_t object_length = 0;
	uint16_t private_bytes = 0;

	p_en50221_descriptor->descriptor_tag = p_descriptor->descriptor_tag;
	p_en50221_descriptor->descriptor_length = p_descriptor->descriptor_length;

	if (p_descriptor->descriptor_tag != TAG_CA_DESCRIPTOR) {
		printf("ERROR::Trying to copy a CA descriptor with incorrect tag [%d]. Bailing out.\n",
		       p_descriptor->descriptor_tag);
		return 16;
	}
	p_en50221_descriptor->ca.ca_system_id = p_descriptor->ca.ca_system_id;
	p_en50221_descriptor->ca.reserved = p_descriptor->ca.reserved;
	p_en50221_descriptor->ca.ca_pid = p_descriptor->ca.ca_pid;

	printf("\t%s: Tag=[%02x], Length=[%02x], CA system ID=[%02x], CA PID=[%02x]\n", __FUNCTION__,
		p_en50221_descriptor->descriptor_tag, p_en50221_descriptor->descriptor_length,
		p_en50221_descriptor->ca.ca_system_id, p_en50221_descriptor->ca.ca_pid);

	object_length += (8 + 8 + 16 + 3 + 13);

	private_bytes = p_descriptor->descriptor_length - 4;

	printf("%s: Private bytes=[%d] [ ", __FUNCTION__, private_bytes);
	uint8_t *p_en50221_priv = (uint8_t *) malloc(sizeof (uint8_t) * private_bytes);
	if (p_en50221_priv != NULL) {

		for (i = 0; i < private_bytes; i++) {
			p_en50221_priv[i] = p_descriptor->ca.p_private_data_byte[i];
			printf("%02x ", p_en50221_priv[i]);
		}
		printf("]\n");

		object_length += (private_bytes * 8);
		p_en50221_descriptor->ca.p_private_data_byte = p_en50221_priv;
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

	p_en50221_stream->stream_type = p_streams->stream_type;
	p_en50221_stream->reserved_1 = p_streams->reserved_1;
	p_en50221_stream->elementary_pid = p_streams->elementary_pid;
	p_en50221_stream->reserved_2 = p_streams->reserved_2;
	p_en50221_stream->es_info_length = p_streams->es_info_length;
	p_en50221_stream->streams_desc_count = 0;

	printf("%s: Stream Type=[%d], Elementary PID=[%d], ES length=[%d], Number of descriptors=[%d]\n", __FUNCTION__,
		p_streams->stream_type,  p_streams->elementary_pid, p_streams->es_info_length, p_streams->streams_desc_count);

	object_length += (8 + 3 + 13 + 4 + 12);
	// Now each stream has descriptors
	struct descriptor *p_en50221_streams_desc =
		(struct descriptor *) malloc(sizeof (struct descriptor) * p_streams->streams_desc_count);
	if (p_en50221_streams_desc != NULL) {

		for (descriptor_count = 0; descriptor_count < p_streams->streams_desc_count; descriptor_count++) {
			struct descriptor *p_en50221_stream_desc =
				&p_en50221_streams_desc[p_en50221_stream->streams_desc_count];
			struct descriptor *p_stream_desc = &p_streams->p_descriptors[descriptor_count];

			if (p_stream_desc && p_stream_desc->descriptor_tag == TAG_CA_DESCRIPTOR) {	// Copy only CA descriptors
				object_length += copy_en50221_descriptor_object(p_en50221_stream_desc, p_stream_desc);
				p_en50221_stream->streams_desc_count++;
			}
		}
	}
	else {
		printf("%s: Memory allocation failed.\n", __FUNCTION__);
		return 0;
	}

	p_en50221_stream->p_descriptors = p_en50221_streams_desc;

	return object_length;
}

static void try_move_ca_descriptors(struct service_info *p_si)
{
	int i,j,k,l, movable = 1, ca_descriptors = 0, num_ca1, num_ca2, found_match, found;
	struct streams *p_stream1, *p_stream2;
	struct descriptor *p_desc1, *p_desc2, *p_new_descriptors;

	// First, check how many CA descriptors that we have on stream level
	for (i = 0; i < p_si->p_pmt->stream_count; i++) {
		p_stream1 = &p_si->p_pmt->p_streams[i];
		for (j = 0; j < p_stream1->streams_desc_count; j++) {
			p_desc1 = &p_stream1->p_descriptors[j];
			if (p_desc1->descriptor_tag == TAG_CA_DESCRIPTOR) {
				ca_descriptors++;
			}
		}
	}

	// Check that we have CA descriptors on stream level only
	if (p_si->p_pmt->program_desc_count != 0 || ca_descriptors == 0) {
		movable = 0;
	}

	// Check that all streams with CA descriptors have exactly the same CA descriptors
	for (i = 0; i < p_si->p_pmt->stream_count; i++) {
		p_stream1 = &p_si->p_pmt->p_streams[i];

		// Get number of CA descriptors for stream1
		num_ca1 = 0;
		for (j = 0; j < p_stream1->streams_desc_count; j++) {
			p_desc1 = &p_stream1->p_descriptors[j];
			if (p_desc1->descriptor_tag == TAG_CA_DESCRIPTOR) {
				num_ca1++;
			}
		}
		for (k = 0; k < p_si->p_pmt->stream_count; k++) {
			p_stream2 = &p_si->p_pmt->p_streams[k];

			// Get number of CA descriptors for stream2
			num_ca2 = 0;
			for (j = 0; j < p_stream2->streams_desc_count; j++) {
				p_desc2 = &p_stream2->p_descriptors[j];
				if (p_desc2->descriptor_tag == TAG_CA_DESCRIPTOR) {
					num_ca2++;
				}
			}

			// If they have the same number of CA descriptors, check that they are EXACTLY the same
			if (num_ca1 == num_ca2) {
				for (j = 0; j < p_stream1->streams_desc_count; j++) {
					p_desc1 = &p_stream1->p_descriptors[j];
					if (p_desc1->descriptor_tag == TAG_CA_DESCRIPTOR) {
						found_match = 0;
						for (l = 0; l < p_stream2->streams_desc_count; l++) {
							p_desc2 = &p_stream2->p_descriptors[l];
							if (compare_en50221_descriptor_object(p_desc1, p_desc2)) {
								found_match = 1;
							}
						}
						if (found_match == 0) {
							movable = 0;
						}
					}
				}
			} else if (num_ca1 && num_ca2) {
				// If the number of CAs are not the same (and neither are zero) we can't move
				movable = 0;
			}
		}
	}

	if (movable) {
		printf("%s: Moving all (%d) common CA descriptors from stream to programme level\n", __FUNCTION__, ca_descriptors);

		// Allocate space for the new descriptors, at most <ca_descriptors> number
		p_new_descriptors = (struct descriptor *) malloc(sizeof(struct descriptor) * (ca_descriptors));

		p_si->p_pmt->p_descriptors = p_new_descriptors;

		// Copy stream level descriptor to programme level
		for (i = 0; i < p_si->p_pmt->stream_count; i++) {
			p_stream1 = &p_si->p_pmt->p_streams[i];
			for (j = 0; j < p_stream1->streams_desc_count; j++) {
				p_desc1 = &p_stream1->p_descriptors[j];

				// Copy only CA descriptors
				if (p_desc1->descriptor_tag == TAG_CA_DESCRIPTOR) {
					// Check that this descriptor has not already been copied
					found = 0;
					for (k = 0; k < p_si->p_pmt->program_desc_count; k++) {
						p_desc2 = &p_si->p_pmt->p_descriptors[k];
						if (compare_en50221_descriptor_object(p_desc1, p_desc2)) {
							found++;
						}
					}
					if (found == 0) {
						printf("%s: Adding common CA descriptor to programme level\n", __FUNCTION__);

						struct descriptor *p_dest_desc =
							&p_si->p_pmt->p_descriptors[p_si->p_pmt->program_desc_count++];
						copy_en50221_descriptor_object(p_dest_desc, p_desc1);
					}
				}
			}
		}

		// Now remove the copied descriptors from stream level
		for (i = 0; i < p_si->p_pmt->stream_count; i++) {
			p_stream1 = &p_si->p_pmt->p_streams[i];
			for (j = 0; j < p_stream1->streams_desc_count; j++) {
				p_desc1 = &p_stream1->p_descriptors[j];
				for (k = 0; k < p_si->p_pmt->program_desc_count; k++) {
					p_desc2 = &p_si->p_pmt->p_descriptors[k];
					if (compare_en50221_descriptor_object(p_desc1, p_desc2)) {
						// Remove from stream level
						printf("%s: Removing common CA descriptor from stream level\n", __FUNCTION__);
						for (l = j; l < p_stream1->streams_desc_count - 1; l++) {
							copy_en50221_descriptor_object(&p_stream1->p_descriptors[j],
										       &p_stream1->p_descriptors[j + 1]);
						}
						p_stream1->streams_desc_count--;
						p_stream1->es_info_length -= p_desc2->descriptor_length;
					}
				}
			}
		}
	}
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
	// Program info length need not be set
	// It is calculated later
	//p_en50221_pmt_object->program_info_length = p_si->p_pmt->program_info_length;

	printf("\n%s: Copying EN50221 Header\n", __FUNCTION__);
	printf("%s: Program Number=[%d]\n", __FUNCTION__, p_en50221_pmt_object->program_number);

	p_en50221_pmt_object->program_desc_count = p_si->p_pmt->program_desc_count;
	printf("%s: Program level descriptor count=[%d]\n", __FUNCTION__, p_en50221_pmt_object->program_desc_count);

	object_length += (16 + 2 + 5 + 1 + 4 + 12);

	//	Program descriptor
	struct descriptor *p_prog_desc = NULL;
	struct descriptor *p_en50221_descriptor = NULL;
	struct descriptor *p_descriptor_objects = NULL;

	p_descriptor_objects = (struct descriptor *) malloc(sizeof (struct descriptor) * p_en50221_pmt_object->program_desc_count);
	if (p_descriptor_objects == NULL) {
		printf("%s: Memory allocation failed\n", __FUNCTION__);
		return 0;
	}

	p_prog_desc = (struct descriptor*)p_si->p_pmt->p_descriptors;
	for (i = 0; i < p_si->p_pmt->program_desc_count; ++i, ++p_prog_desc) {
		printf("%s: CA descriptor=[%02x] found, @ [%p], descriptor length=[%02x]\n", __FUNCTION__,
				p_prog_desc->descriptor_tag, &p_prog_desc, p_prog_desc->descriptor_length);

		if (p_prog_desc->descriptor_tag == TAG_CA_DESCRIPTOR) {	// Copy only CA descriptors
			p_en50221_descriptor = (struct descriptor *) &p_descriptor_objects[ca_descriptors];
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
	uint16_t i = 0;

	printf("%s: Setting PMT Command\n", __FUNCTION__);
	if (p_en50221_pmt_object->program_desc_count > 0) {
		printf("%s: CA descriptor(s) found @ PROGRAM Level, Setting CA PMT command=[%02x]\n",
		       __FUNCTION__, pmt_command);
			p_en50221_pmt_object->ca_pmt_cmd_id = pmt_command;
			object_length += 8;
		}

	for (i = 0; i < p_en50221_pmt_object->stream_count; i++) {
		if (p_en50221_pmt_object->p_en50221_streams[i].streams_desc_count > 0) {
			printf("%s: CA descriptor(s) found @ STREAMS Level, Setting CA PMT command=[%02x]\n",
			       __FUNCTION__, pmt_command);
			p_en50221_pmt_object->p_en50221_streams[i].ca_pmt_cmd_id = pmt_command;
			object_length += 8;
		}
	}

	return object_length;
}


uint16_t do_en50221_pmt_object(struct en50221_pmt_object *p_en50221_pmt_object, struct service_info *p_si,
			       uint8_t pmt_list_mgmt, uint8_t pmt_command, int move_to_programme)
{
	uint16_t object_length = 0;
	uint32_t asn_1_words = 0;
	uint8_t i;

	if (move_to_programme) {
		try_move_ca_descriptors(p_si);
	}

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

void debug_parse_message(struct ca_msg *p_ca_msg, uint16_t length)
{
	const char* type;
	uint16_t pos = 0;
	uint8_t* ptr = p_ca_msg->msg;
	struct descriptor desc;
	int list_management;
	int program_info_length;
	int es_info_length;
	char message[2048];
	char temp[100];
	uint16_t i;

	// Check which kind of message we have
	uint32_t type_id = (ptr[pos + 0] << 16) + (ptr[pos + 1] << 8) +
		ptr[pos + 2];
	pos += 3;

	switch (type_id) {
	case CA_APP_INFO_ENQUIRY:
		type = "CA_APP_INFO_ENQUIRY";
		break;
	case CA_APP_INFO:
		type = "CA_APP_INFO";
		break;
	case CA_ENTER_MENU:
		type = "CA_ENTER_MENU";
		break;
	case CA_INFO_ENQUIRY:
		type = "CA_INFO_ENQUIRY";
		break;
	case CA_INFO:
		type = "CA_INFO";
		break;
	case CA_PMT:
		type = "CA_PMT";
		break;
	case CA_PMT_REPLY:
		type = "fine CA_PMT_REPLY";
		break;
	case CA_CLOSE_MMI:
		type = "CA_CLOSE_MMI";
		break;
	case CA_DISPLAY_CONTROL:
		type = "CA_DISPLAY_CONTROL";
		break;
	case CA_DISPLAY_REPLY:
		type = "CA_DISPLAY_REPLY";
		break;
	case CA_TEXT_LAST:
		type = "CA_TEXT_LAST";
		break;
	case CA_TEXT_MORE:
		type = "CA_TEXT_MORE";
		break;
	case CA_KEYPAD_CONTROL:
		type = "CA_KEYPAD_CONTROL";
		break;
	case CA_KEYPRESS:
		type = "CA_KEYPRESS";
		break;
	case CA_ENQUIRY:
		type = "CA_ENQUIRY";
		break;
	case CA_ANSWER:
		type = "CA_ANSWER";
		break;
	case CA_MENU_LAST:
		type = "CA_MENU_LAST";
		break;
	case CA_MENU_MORE:
		type = "CA_MENU_MORE";
		break;
	case CA_MENU_ANSWER:
		type = "CA_MENU_ANSWER";
		break;
	case CA_LIST_LAST:
		type = "CA_LIST_LAST";
		break;
	case CA_LIST_MORE:
		type = "CA_LIST_MORE";
		break;
	default:
		type = "Unknown";
	}

	sprintf(message, "%s: %s={", __FUNCTION__, type);

	sprintf(temp, "Length=%d", ptr[pos++]);
	strcat(message, temp);
	list_management = ptr[pos++];
	sprintf(temp, ":CA_PMT_ListManagement=%d", list_management);
	strcat(message, temp);
	sprintf(temp, ":ProgramNumber=0x%02x%02x=%d", ptr[pos + 0],
		ptr[pos + 1], (ptr[pos + 0] << 8) + ptr[pos + 1]);
	strcat(message, temp);
	pos += 2;
	sprintf(temp, ":VersionNumber=%d", (ptr[pos++] >> 1) & 0x1f);
	strcat(message, temp);
	program_info_length = ((ptr[pos + 0] << 8) & 0x0f) + ptr[pos + 1];
	pos += 2;
	sprintf(temp, ":Program={");
	strcat(message, temp);
	int end_program = pos + program_info_length;
	if (program_info_length) {
		switch (ptr[pos++]) {
		case 1:
			sprintf(temp, "Command=OK_Descrambling");
			strcat(message, temp);
			break;
		case 2:
			sprintf(temp, "Command=OK_MMI");
			strcat(message, temp);
			break;
		case 3:
			sprintf(temp, "Command=Query");
			strcat(message, temp);
			break;
		case 4:
			sprintf(temp, "Command=NotSelected");
			strcat(message, temp);
			break;
		default:
			sprintf(temp, "Command=Unknown");
			strcat(message, temp);
		}

		while (pos < end_program) {
			pos = parse_ca_descriptor(&desc, ptr, pos);
			sprintf(temp, ":CA={System=%d:PID=%d:PrivateData={", desc.ca.ca_system_id,
				desc.ca.ca_pid);
			strcat(message, temp);
			for (i = 0; i < desc.descriptor_length - 4; i++) {
				if (i == 0)
					sprintf(temp, "%02x", desc.ca.p_private_data_byte[i]);
				else
					sprintf(temp, ",%02x", desc.ca.p_private_data_byte[i]);
				strcat(message, temp);
			}
			sprintf(temp, "}}");
			strcat(message, temp);
		}
	}
	sprintf(temp, "}");
	strcat(message, temp);

	// Do streams
	while (pos < length) {
		sprintf(temp, ":Stream={Type=");
		strcat(message, temp);
		switch (ptr[pos++]) {
		case 2:
			sprintf(temp, "Video");
			break;
		case 3:
			sprintf(temp, "Audio");
			break;
		case 5:
			sprintf(temp, "Private");
			break;
		case 6:
			sprintf(temp, "PES_Packets");
			break;
		default:
			sprintf(temp, "Unknown");
		}
		strcat(message, temp);
		sprintf(temp, ":StreamPID=0x%02x%02x=%d", (ptr[pos + 0] & 0x3f),
		       ptr[pos + 1], ((ptr[pos + 0] & 0x3f) << 8) +
		       ptr[pos + 1]);
		strcat(message, temp);
		pos += 2;
		es_info_length = ((ptr[pos + 0] & 0x0f) << 8) +
			ptr[pos + 1];
		pos += 2;

		sprintf(temp, ":ES_Info={");
		strcat(message, temp);
		if (es_info_length) {
			switch (ptr[pos++]) {
			case 1:
				sprintf(temp, "Command=OK_Descrambling");
				strcat(message, temp);
				break;
			case 2:
				sprintf(temp, "Command=OK_MMI");
				strcat(message, temp);
				break;
			case 3:
				sprintf(temp, "Command=Query");
				strcat(message, temp);
				break;
			case 4:
				sprintf(temp, "Command=NotSelected");
				strcat(message, temp);
				break;
			default:
				sprintf(temp, "Command=Unknown");
				strcat(message, temp);
			}
			pos = parse_ca_descriptor(&desc, ptr, pos);
			sprintf(temp, ":CA={System=%d:PID=%d:PrivateData={", desc.ca.ca_system_id,
				desc.ca.ca_pid);
			strcat(message, temp);
			for (i = 0; i < desc.descriptor_length - 4; i++) {
				if (i == 0)
					sprintf(temp, "%02x", desc.ca.p_private_data_byte[i]);
				else
					sprintf(temp, ",%02x", desc.ca.p_private_data_byte[i]);
				strcat(message, temp);
			}
			sprintf(temp, "}}");
			strcat(message, temp);
		}
		sprintf(temp, "}}");
		strcat(message, temp);
	}

	sprintf(temp, "}\n");
	strcat(message, temp);

	printf(message);
}

uint16_t write_en50221_pmt_object(struct en50221_pmt_object *p_en50221_pmt_object, char *ca_dev)
{
	uint32_t object_length = 0, temp = 0;
	struct ca_msg *p_ca_msg;
	uint8_t i, words = 0;
	uint16_t pos = 0;
	struct descriptor *p_desc = NULL;
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

	// Calculate program_info_length
	if (p_en50221_pmt_object->program_desc_count) {
		// Set length to one
		p_en50221_pmt_object->program_info_length = 1;
		// Add length of all CA descriptors
		for (i = 0; i < p_en50221_pmt_object->program_desc_count; i++) {
			p_desc = (struct descriptor *) &p_en50221_pmt_object->p_en50221_prog_desc[i];
			if (p_desc->descriptor_length) {
				p_en50221_pmt_object->program_info_length += p_desc->descriptor_length + 2;
			}
		}
	} else {
		p_en50221_pmt_object->program_info_length = 0;
	}

	pos = en50221_encode_header(p_ca_msg, p_en50221_pmt_object, pos);
	printf("%s: EN50221 header encoded\n", __FUNCTION__);

	if (p_en50221_pmt_object->program_desc_count) {
		// Put ca_pmt_cmd_id only once
		pos = encode_ca_pmt_command(p_ca_msg, p_en50221_pmt_object, pos, PROGRAM_SCRAMBLED);
	} else {
	}

	for (i = 0; i < p_en50221_pmt_object->program_desc_count; i++) {
		printf("%s: Program level descriptor @ [%p], count=[%d], tag=[%02x], length=[%02x]\n", __FUNCTION__,
				&p_en50221_pmt_object->p_en50221_prog_desc, p_en50221_pmt_object->program_desc_count,
				p_en50221_pmt_object->p_en50221_prog_desc[i].descriptor_tag,
				p_en50221_pmt_object->p_en50221_prog_desc[i].descriptor_length);
		p_desc = (struct descriptor *) &p_en50221_pmt_object->p_en50221_prog_desc[i];

		if (p_desc->descriptor_length) {
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

	// Uncommenting the following line will parse the ca_pmt and print the parsed data
	//debug_parse_message(p_ca_msg, pos);

	write_to_slot(p_ca_msg, ca_dev);

	return 0;
}
