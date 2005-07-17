/*
 * section and descriptor parser
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef _SI_DVB_LINKAGE_DESCRIPTOR
#define _SI_DVB_LINKAGE_DESCRIPTOR 1

#include <si/descriptor.h>
#include <si/common.h>

struct dvb_linkage_descriptor {
	struct descriptor d;

	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint16_t service_id;
	uint8_t linkage_type;
	/* uint8_t data[] */
} packed;

struct dvb_linkage_data_08 {
  EBIT3(uint8_t hand_over_type 		: 4;  ,
	uint8_t reserved		: 3;  ,
	uint8_t origin_type		: 1;  );
	uint16_t id;
	/* uint8_t data[] */
} packed;

static inline struct dvb_linkage_descriptor*
	dvb_linkage_descriptor_parse(struct descriptor* d)
{
	int pos = 0;
	uint8_t* buf = (uint8_t*) d + 2;
	int len = d->len;
	struct dvb_linkage_descriptor *p =
		(struct dvb_linkage_descriptor*) d;

	if (len < (sizeof(struct dvb_linkage_descriptor) - 2))
		return NULL;

	bswap16(buf);
	bswap16(buf+2);
	bswap16(buf+4);

	pos += sizeof(struct dvb_linkage_descriptor);

	if (p->linkage_type == 0x08) {
		if ((len - pos) < sizeof(struct dvb_linkage_data_08))
			return NULL;
		bswap16(buf+pos+1);
	}

	return (struct dvb_linkage_descriptor*) d;
}

static inline uint8_t *
	dvb_linkage_descriptor_data(struct dvb_linkage_descriptor *d)
{
	return (uint8_t *) d + sizeof(struct dvb_linkage_descriptor);
}

static inline int
	dvb_linkage_descriptor_data_length(struct dvb_linkage_descriptor *d)
{
	return d->d.len - 7;
}

static inline uint8_t *
	dvb_linkage_data_08_data(struct dvb_linkage_data_08 *d)
{
	return (uint8_t *) d + sizeof(struct dvb_linkage_data_08);
}

static inline int
	dvb_linkage_data_08_data_length(struct dvb_linkage_descriptor *d)
{
	return dvb_linkage_descriptor_data_length(d) - sizeof(struct dvb_linkage_data_08);
}

#endif
