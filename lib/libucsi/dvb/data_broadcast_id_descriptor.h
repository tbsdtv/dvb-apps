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

#ifndef _UCSI_DVB_DATA_BROADCAST_ID_DESCRIPTOR
#define _UCSI_DVB_DATA_BROADCAST_ID_DESCRIPTOR 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <ucsi/descriptor.h>
#include <ucsi/endianops.h>

/**
 * dvb_data_broadcast_id_descriptor structure.
 */
struct dvb_data_broadcast_id_descriptor {
	struct descriptor d;

	uint16_t data_broadcast_id;
	/* uint8_t id_selector_byte[] */
} packed;

/**
 * Process a dvb_data_broadcast_id_descriptor.
 *
 * @param d Generic descriptor structure.
 * @return dvb_data_broadcast_id_descriptor pointer, or NULL on error.
 */
static inline struct dvb_data_broadcast_id_descriptor*
	dvb_data_broadcast_id_descriptor_codec(struct descriptor* d)
{
	if (d->len < (sizeof(struct dvb_data_broadcast_id_descriptor) - 2))
		return NULL;

	bswap16((uint8_t*) d + 2);

	return (struct dvb_data_broadcast_id_descriptor*) d;
}

/**
 * Accessor for the selector_byte field of a dvb_data_broadcast_id_descriptor.
 *
 * @param d dvb_data_broadcast_id_descriptor pointer.
 * @return Pointer to the field.
 */
static inline uint8_t *
	dvb_data_broadcast_id_descriptor_id_selector_byte(struct dvb_data_broadcast_id_descriptor *d)
{
	return (uint8_t *) d + sizeof(struct dvb_data_broadcast_id_descriptor);
}

/**
 * Determine the length of the selector_byte field of a dvb_data_broadcast_id_descriptor.
 *
 * @param d dvb_data_broadcast_id_descriptor pointer.
 * @return Length of the field in bytes.
 */
static inline int
	dvb_data_broadcast_id_descriptor_id_selector_byte_length(struct dvb_data_broadcast_id_descriptor *d)
{
	return d->d.len - 2;
}

#ifdef __cplusplus
}
#endif

#endif
