/*
 * section and descriptor parser
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
 * Copyright (C) 2005 Patrick Boettcher (pb@linuxtv.org)
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
#ifndef _UCSI_DVB_INT_SECTION_H
#define _UCSI_DVB_INT_SECTION_H

#include <ucsi/section.h>

struct dvb_int_section {
	struct section head;

	uint8_t  action_type;
	uint8_t  platform_id_hash;

  EBIT3(uint8_t  reserved1		: 2; ,
	uint8_t  version_number		: 5; ,
	uint8_t  current_next_indicator	: 1; );
	uint8_t  section_number;
	uint8_t  last_section_number;
  EBIT2(uint32_t platform_id				:24;  ,
	uint32_t processing_order			: 8;  );
  EBIT2(uint16_t reserved2				: 4;  ,
	uint16_t platform_descriptor_loop_length	:12;  );
	/* platform_descriptor_loop[] */
	/* loop of target_loop+operational_loop */
} packed;

struct dvb_int_section_target_loop {
  EBIT2(uint16_t reserved3			: 4;  ,
	uint16_t target_descriptor_loop_length	:12;  );
	/* target_descriptor_loop[] */
} packed;

struct dvb_int_section_operational_loop {
  EBIT2(uint16_t reserved4				: 4;  ,
	uint16_t operational_descriptor_loop_length	:12;  );
	/* operational_descriptor_loop[] */
} packed;

extern struct dvb_int_section * dvb_int_section_parse(struct section_ext *);

#define dvb_platform_descriptor_for_each(first, pos) \
	for ((pos) = dvb_platform_descriptor_first(first); \
	     (pos); \
	     (pos) = dvb_platform_descriptor_next(first, pos))

#define dvb_target_operational_loop_for_each(first,pos) \
	for ((pos) = dvb_target_operational_loop_first(first); \
	     (pos); \
	     (pos) = dvb_target_operational_loop_next(first, pos))

static inline struct dvb_int_section_operational_loop *
	dvb_int_section_operational_loop_get(struct dvb_int_section_target_loop *tl)
{
	return (struct dvb_int_section_operational_loop *)
		((uint8_t *) tl + sizeof(struct dvb_int_section_target_loop) + tl->target_descriptor_loop_length);
}

#define dvb_target_descriptor_for_each(first, pos) \
	for ((pos) = dvb_target_descriptor_first(first); \
	     (pos); \
	     (pos) = dvb_target_descriptor_next(first, pos))

#define dvb_operational_descriptor_for_each(first, pos) \
	for ((pos) = dvb_operational_descriptor_first(first); \
	     (pos); \
	     (pos) = dvb_operational_descriptor_next(first, pos))


/******************************** PRIVATE CODE ********************************/
static inline struct descriptor *
	dvb_platform_descriptor_first(struct dvb_int_section *in)
{
	if (in->platform_descriptor_loop_length == 0)
		return NULL;

	return (struct descriptor *)
		((uint8_t *) in + sizeof(struct dvb_int_section));
}

static inline struct descriptor *
	dvb_platform_descriptor_next(struct dvb_int_section *in,
		struct descriptor* pos)
{
	return next_descriptor((uint8_t*) in + sizeof(struct dvb_int_section),
		in->platform_descriptor_loop_length,
		pos);
}

static inline struct dvb_int_section_target_loop *
	dvb_target_operational_loop_first(struct dvb_int_section *in)
{
	if (sizeof(struct dvb_int_section) + in->platform_descriptor_loop_length >= section_ext_length((struct section_ext *) in))
		return NULL;

	return (struct dvb_int_section_target_loop *)
		((uint8_t *) in + sizeof(struct dvb_int_section) + in->platform_descriptor_loop_length);
}

static inline struct dvb_int_section_target_loop *
	dvb_target_operational_loop_next(struct dvb_int_section *in,
			struct dvb_int_section_target_loop *pos)
{
	struct dvb_int_section_operational_loop *ol = dvb_int_section_operational_loop_get(pos);
	struct dvb_int_section_target_loop *next =
		(struct dvb_int_section_target_loop *) ( (uint8_t *) pos +
			sizeof(struct dvb_int_section_target_loop) + pos->target_descriptor_loop_length +
			sizeof(struct dvb_int_section_operational_loop) + ol->operational_descriptor_loop_length);
	struct dvb_int_section_target_loop *end =
		(struct dvb_int_section_target_loop *) ((uint8_t *) in + section_ext_length((struct section_ext *) in) );

	if (next >= end)
		return 0;
	return next;
}

static inline struct descriptor *
	dvb_target_descriptor_first(struct dvb_int_section_target_loop *tl)
{
	if (tl->target_descriptor_loop_length == 0)
		return NULL;

	return (struct descriptor *)
		((uint8_t *) tl + sizeof(struct dvb_int_section_target_loop));
}

static inline struct descriptor *
	dvb_target_descriptor_next(struct dvb_int_section_target_loop *tl,
		struct descriptor* pos)
{
	return next_descriptor((uint8_t*) tl + sizeof(struct dvb_int_section_target_loop),
		tl->target_descriptor_loop_length,
		pos);
}

static inline struct descriptor *
	dvb_operational_descriptor_first(struct dvb_int_section_operational_loop *ol)
{
	if (ol->operational_descriptor_loop_length == 0)
		return NULL;

	return (struct descriptor *)
		((uint8_t *) ol + sizeof(struct dvb_int_section_operational_loop));
}

static inline struct descriptor *
	dvb_operational_descriptor_next(struct dvb_int_section_operational_loop *ol,
		struct descriptor* pos)
{
	return next_descriptor((uint8_t*) ol + sizeof(struct dvb_int_section_operational_loop),
		ol->operational_descriptor_loop_length,
		pos);
}

#endif
