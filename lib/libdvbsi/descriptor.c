/*
	A SI parser implementation for libdvb
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
#include <stdint.h>
#include "descriptor.h"

uint8_t parse_descriptor_info(struct descriptor_info *p_descr_info, uint8_t *buf, uint16_t pos)
{
	p_descr_info->descriptor_tag = buf[pos + 0];
	p_descr_info->descriptor_length = buf[pos + 1];

	return 0;
}


uint16_t parse_ca_descriptor(struct ca_descriptor *p_descriptor, uint8_t *buf, uint16_t pos)
{
	uint8_t loops = 0;
	uint8_t *p_private_data = NULL;

	p_descriptor->descriptor_tag = buf[pos + 0];
	p_descriptor->descriptor_length = buf[pos + 1];
	printf("INFO:: Parsing descriptor: %s, Tag=[%02x], Length=[%d]\n", __FUNCTION__, p_descriptor->descriptor_tag, p_descriptor->descriptor_length);
	p_descriptor->ca_system_id = ((p_descriptor->ca_system_id | buf[pos + 2]) << 8) | buf[pos + 3];
	p_descriptor->reserved = buf[pos + 4] >> 5;
	p_descriptor->ca_pid = ((p_descriptor->ca_pid | (buf[pos + 4] & 0x1f)) << 8) | buf[pos + 5];
	p_private_data = (uint8_t *) malloc(sizeof (uint8_t) * (p_descriptor->descriptor_length - 4));

	printf("%s: Tag=[%02x], Length=[%02x], CA System=[%02x], CA PID=[%02x]\n", __FUNCTION__,
		p_descriptor->descriptor_tag, p_descriptor->descriptor_length, p_descriptor->ca_system_id, p_descriptor->ca_pid);
	printf("%s: CA Private Data=[ ", __FUNCTION__);
	while (loops < (p_descriptor->descriptor_length - 4)) {
		p_private_data[loops] = buf[pos + 6 + loops];
		printf("%02x ", p_private_data[loops]);
		loops++;
	}
	printf("]\n");
	p_descriptor->p_private_data_byte = p_private_data;

	pos += loops + 6;
	printf("%s: Pos=[%d]\n", __FUNCTION__, pos);

	return pos;
}


void *allocate_descriptor_storage(struct descriptor_info *p_descriptor_info)
{
	void *p_descriptor = NULL;
	switch (p_descriptor_info->descriptor_tag) {
		case 0x09:
			p_descriptor = (struct ca_descriptor *) malloc(sizeof (struct ca_descriptor) * MAX_DESCRIPTOR);
			break;
	}

	return p_descriptor;
}

uint16_t parse_descriptor(void *p_descr, struct descriptor_info *p_descriptor_info, uint8_t *buf, uint16_t pos)
{
	uint16_t descriptor_loop = 0;

	parse_descriptor_info(p_descriptor_info, buf, pos);
	switch (p_descriptor_info->descriptor_tag) {
		case 0x09:	/*	CA descriptor		*/
			{
				struct ca_descriptor *p_descriptor = (struct ca_descriptor *) p_descr;
				pos = parse_ca_descriptor(&p_descriptor[descriptor_loop], buf, pos);
				p_descr = p_descriptor;
			}
			break;
		default:	/*	parse an unknown	*/
				pos += p_descriptor_info->descriptor_length;
				break;
	}

	return pos;
}
