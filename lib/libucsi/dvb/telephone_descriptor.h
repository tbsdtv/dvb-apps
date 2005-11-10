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

#ifndef _UCSI_DVB_TELEPHONE_DESCRIPTOR
#define _UCSI_DVB_TELEPHONE_DESCRIPTOR 1

#include <ucsi/descriptor.h>
#include <ucsi/common.h>

struct dvb_telephone_descriptor {
	struct descriptor d;

  EBIT3(uint8_t reserved_1			: 2; ,
	uint8_t foreign_availability		: 1; ,
	uint8_t connection_type			: 5; );
  EBIT4(uint8_t reserved_2			: 1; ,
	uint8_t country_prefix_length		: 2; ,
	uint8_t international_area_code_length	: 3; ,
	uint8_t operator_code_length		: 2; );
  EBIT3(uint8_t reserved_3			: 1; ,
	uint8_t national_area_code_length	: 3; ,
	uint8_t core_number_length		: 4; );
	/* uint8_t country_prefix[] */
	/* uint8_t international_area_code[] */
	/* uint8_t operator_code[] */
	/* uint8_t national_area_code[] */
	/* uint8_t core_number[] */
} packed;


static inline struct dvb_telephone_descriptor*
	dvb_telephone_descriptor_parse(struct descriptor* d)
{
	struct dvb_telephone_descriptor* p =
		(struct dvb_telephone_descriptor*) d;
	int pos = sizeof(struct dvb_telephone_descriptor) - 2;
	int len = d->len;

	if (pos > len)
		return NULL;

	pos +=  p->country_prefix_length +
		p->international_area_code_length +
		p->operator_code_length +
		p->national_area_code_length +
		p->core_number_length;

	if (pos != len)
		return NULL;

	return p;
}

static inline uint8_t*
	dvb_telephone_descriptor_country_prefix(struct dvb_telephone_descriptor* d)
{
	return (uint8_t*) d + sizeof(struct dvb_telephone_descriptor);
}

static inline uint8_t*
	dvb_telephone_descriptor_international_area_code(struct dvb_telephone_descriptor* d)
{
	return dvb_telephone_descriptor_country_prefix(d) + d->country_prefix_length;
}

static inline uint8_t*
	dvb_telephone_descriptor_operator_code(struct dvb_telephone_descriptor* d)
{
	return dvb_telephone_descriptor_international_area_code(d) + d->international_area_code_length;
}

static inline uint8_t*
	dvb_telephone_descriptor_national_area_code(struct dvb_telephone_descriptor* d)
{
	return dvb_telephone_descriptor_operator_code(d) + d->operator_code_length;
}

static inline uint8_t*
	dvb_telephone_descriptor_core_number(struct dvb_telephone_descriptor* d)
{
	return dvb_telephone_descriptor_national_area_code(d) + d->national_area_code_length;
}

#endif
