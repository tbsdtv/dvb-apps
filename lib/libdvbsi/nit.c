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
#include "nit.h"


uint16_t parse_nit_header(struct nit *p_nit, uint8_t *buf, uint16_t pos)
{
	p_nit->table_id = buf[pos + 0];
	p_nit->section_synatx_indicator = buf[pos + 1] >> 7;
	p_nit->reserved_1 = (buf[pos + 1] >> 6) & 0x01;
	p_nit->reserved_2 = (buf[pos + 1]] >> 4) & 0x03;
	p_nit->section_length = ((p_nit->section_length | (buf[pos + 1] & 0x0f)) << 8) | buf[pos + 2];
	p_nit->network_id = ((p_nit->network_id | buf[pos + 3]) << 8) | buf[pos + 4];
	p_nit->reserved_3 = buf[pos + 5] >> 6;
	p_nit->version_number = (buf[pos + 5] >> 1) & 0x1f;
	p_nit->current_next_indicator = buf[pos + 5] >> 7;
	p_nit->section_number = buf[pos + 6];
	p_nit->last_section_number = buf[pos + 7];
	p_nit->reserved_4 = buf[pos + 8] >> 4;
	p_nit->network_descriptors_length = ((p_nit->network_descriptors_length | (buf[pos + 8] & 0x0f)) << 8) | buf[pos + 9];

	pos += 10;

	return pos;
}


uint16_t parse_nit_loop(struct nit_loop *p_nit_loop, uint8_t *buf, uint16_t pos)
{

	return pos;
}



uint16_t parse_nit(struct nit *p_nit, uint8_t *buf)
{
	uint16_t pos = 0;
	struct
	struct nit_loop *p_nit_loop;

	pos = parse_nit_header(p_nit, buf);
	p_nit->reserved_5 = buf[pos + 0] >> 4;
	p_nit->network_descriptors_length = ((p_nit->network_descriptors_length | (buf[pos + 0] & 0x0f)) << 8) | buf[pos + 1];

	pos += 2;

	p_network_descrtiptor = (struct network_descriptor *) malloc(sizeof (struct network_descriptor) * MAX_NETWORK_DESCRIPTORS);
	if (p_network_descriptor == NULL) {
		printf("%s: Memory allocation failed\n", __FUNCTION__);
		exit(-1);
	}
	pos = parse_descriptor();

	p_nit_loop = (struct nit_loop *) malloc(sizeof (struct nit_loop) * MAX_NIT_LOOPS);

	if (p_nit_loop == NULL) {
		printf("%s: Memory allocation failed\n", __FUNCTION__);
		exit(-1);
	}

	for (i = 0; i < )
	pos = parse_nit_loop()


	return 0;
}
