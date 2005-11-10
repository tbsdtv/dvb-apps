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

#ifndef _UCSI_DVB_TRANSPORT_STREAM_DESCRIPTOR
#define _UCSI_DVB_TRANSPORT_STREAM_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/common.h>

struct dvb_transport_stream_descriptor {
	struct descriptor d;

	/* uint8_t data[] */
} packed;

static inline struct dvb_transport_stream_descriptor*
	dvb_transport_stream_descriptor_parse(struct descriptor* d)
{
	return (struct dvb_transport_stream_descriptor*) d;
}

static inline uint8_t *
	dvb_transport_stream_descriptor_data(struct dvb_transport_stream_descriptor *d)
{
	return (uint8_t *) d + sizeof(struct dvb_transport_stream_descriptor);
}

static inline int
	dvb_transport_stream_descriptor_data_length(struct dvb_transport_stream_descriptor *d)
{
	return d->d.len;
}

#endif
