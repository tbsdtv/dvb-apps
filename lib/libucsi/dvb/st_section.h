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

#ifndef _UCSI_DVB_ST_SECTION_H
#define _UCSI_DVB_ST_SECTION_H 1

#include <ucsi/section.h>

struct dvb_st_section {
	struct section head;

	/* uint8_t data[] */
};

struct dvb_st_section *dvb_st_section_parse(struct section *);

static inline uint8_t*
	dvb_st_section_data(struct dvb_st_section* st)
{
	return (uint8_t*) st + sizeof(struct dvb_st_section);
}

static inline int
	dvb_st_section_data_length(struct dvb_st_section* st)
{
	return st->head.length;
}


#endif
