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

#ifndef _UCSI_DVB_CELL_FREQUENCY_LINK_DESCRIPTOR
#define _UCSI_DVB_CELL_FREQUENCY_LINK_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/common.h>

struct dvb_cell_frequency_link_descriptor {
	struct descriptor d;

	/* struct dvb_cell_frequency_link_cell cells[] */
} packed;

struct dvb_cell_frequency_link_cell {
	uint16_t cell_id;
	uint32_t frequency;
	uint8_t subcell_loop_info_length;
	/* struct dvb_cell_frequency_link_subcell subcells[] */
} packed;

struct dvb_cell_frequency_link_cell_subcell {
	uint8_t cell_id_extension;
	uint32_t transposer_frequency;
} packed;

static inline struct dvb_cell_frequency_link_descriptor*
	dvb_cell_frequency_link_descriptor_parse(struct descriptor* d)
{
	int pos = 0;
	int pos2 = 0;
	uint8_t* buf = (uint8_t*) d + 2;
	int len = d->len;

	while(pos < len) {
		struct dvb_cell_frequency_link_cell *e =
			(struct dvb_cell_frequency_link_cell*) (buf+pos);

		if ((pos + sizeof(struct dvb_cell_frequency_link_cell)) > len)
			return NULL;

		bswap16(buf+pos);
		bswap32(buf+pos+2);

		pos += sizeof(struct dvb_cell_frequency_link_cell);

		if ((pos + e->subcell_loop_info_length) > len)
			return NULL;

		if (e->subcell_loop_info_length % sizeof(struct dvb_cell_frequency_link_cell_subcell))
			return NULL;

		pos2 = 0;
		while(pos2 < e->subcell_loop_info_length) {
			bswap32(buf+pos+pos2+1);

			pos2 += sizeof(struct dvb_cell_frequency_link_cell_subcell);
		}

		pos += e->subcell_loop_info_length;
	}

	return (struct dvb_cell_frequency_link_descriptor*) d;
}

#define dvb_cell_frequency_link_descriptor_cells_for_each(d, pos) \
	for ((pos) = dvb_cell_frequency_link_descriptor_cells_first(d); \
	     (pos); \
	     (pos) = dvb_cell_frequency_link_descriptor_cells_next(d, pos))

#define dvb_cell_frequency_link_cell_subcells_for_each(cell, pos) \
	for ((pos) = dvb_cell_frequency_link_cell_subcells_first(cell); \
	     (pos); \
	     (pos) = dvb_cell_frequency_link_cell_subcells_next(cell, pos))










/******************************** PRIVATE CODE ********************************/
static inline struct dvb_cell_frequency_link_cell*
	dvb_cell_frequency_link_descriptor_cells_first(struct dvb_cell_frequency_link_descriptor *d)
{
	if (d->d.len == 0)
		return NULL;

	return (struct dvb_cell_frequency_link_cell *)
		(uint8_t*) d + sizeof(struct dvb_cell_frequency_link_descriptor);
}

static inline struct dvb_cell_frequency_link_cell*
	dvb_cell_frequency_link_descriptor_cells_next(struct dvb_cell_frequency_link_descriptor *d,
						      struct dvb_cell_frequency_link_cell *pos)
{
	uint8_t *end = (uint8_t*) d + 2 + d->d.len;
	uint8_t *next =	(uint8_t *) pos +
			sizeof(struct dvb_cell_frequency_link_cell) +
			pos->subcell_loop_info_length;

	if (next >= end)
		return NULL;

	return (struct dvb_cell_frequency_link_cell *) next;
}

static inline struct dvb_cell_frequency_link_cell_subcell*
	dvb_cell_frequency_cell_subcells_first(struct dvb_cell_frequency_link_cell *d)
{
	if (d->subcell_loop_info_length == 0)
		return NULL;

	return (struct dvb_cell_frequency_link_cell_subcell*)
		(uint8_t*) d + sizeof(struct dvb_cell_frequency_link_cell);
}

static inline struct dvb_cell_frequency_link_cell_subcell*
	dvb_cell_frequency_cell_subcells_next(struct dvb_cell_frequency_link_cell *cell,
					      struct dvb_cell_frequency_link_cell_subcell *pos)
{
	uint8_t *end = (uint8_t*) cell + cell->subcell_loop_info_length;
	uint8_t *next = (uint8_t*) pos +
			sizeof(struct dvb_cell_frequency_link_cell_subcell);

	if (next >= end)
		return NULL;

	return (struct dvb_cell_frequency_link_cell_subcell *) next;
}

#endif
