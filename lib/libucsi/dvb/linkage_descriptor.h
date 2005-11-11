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

#ifndef _UCSI_DVB_LINKAGE_DESCRIPTOR
#define _UCSI_DVB_LINKAGE_DESCRIPTOR 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <ucsi/descriptor.h>
#include <ucsi/endianops.h>

/**
 * dvb_linkage_descriptor structure.
 */
struct dvb_linkage_descriptor {
	struct descriptor d;

	uint16_t transport_stream_id;
	uint16_t original_network_id;
	uint16_t service_id;
	uint8_t linkage_type;
	/* uint8_t data[] */
} packed;

/**
 * Data for a linkage_type of 0x08.
 */
struct dvb_linkage_data_08 {
  EBIT3(uint8_t hand_over_type		: 4;  ,
	uint8_t reserved		: 3;  ,
	uint8_t origin_type		: 1;  );
	uint16_t id;
	/* uint8_t data[] */
} packed;

/**
 * Data for an linkage_type of 0x0b (IP/MAC Notification Table).
 */
struct dvb_linkage_data_0b {
	uint8_t platform_id_data_length;
	/* struct platform_id ids[] */
} packed;

/**
 * Entries in the ids field of a dvb_linkage_data_0b.
 */
struct dvb_platform_id {
  EBIT2(uint32_t platform_id			: 24; ,
	uint32_t platform_name_loop_length	: 8;  );
	/* struct platform_name names[] */
} packed;

/**
 * Entries in the names field of a dvb_platform_id.
 */
struct dvb_platform_name {
	uint8_t iso_639_language_code[3];
	uint8_t platform_name_length;
	/* uint8_t text[] */
} packed;

/**
 * Data for a linkage_type of 0x0c (IP/MAC Notification Table).
 */
struct dvb_linkage_data_0c {
	uint8_t table_type;
	/* bouquet_id if table_type == 0x02 */
} packed;


/**
 * Process a dvb_linkage_descriptor.
 *
 * @param d Generic descriptor pointer.
 * @return dvb_linkage_descriptor pointer, or NULL on error.
 */
static inline struct dvb_linkage_descriptor*
	dvb_linkage_descriptor_codec(struct descriptor* d)
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

	pos += sizeof(struct dvb_linkage_descriptor) - 2;

	if (p->linkage_type == 0x08) {
		if ((len - pos) < sizeof(struct dvb_linkage_data_08))
			return NULL;
		bswap16(buf+pos+1);
	} else if (p->linkage_type == 0x0b) {
		int pos2=0;
		struct dvb_linkage_data_0b *l_0b = (struct dvb_linkage_data_0b *) buf + pos;

		if ((len - pos) < sizeof(struct dvb_linkage_data_0b))
			return NULL;

		pos += sizeof(struct dvb_linkage_data_0b);
		if ((len - pos) < l_0b->platform_id_data_length)
			return NULL;

		while (pos2 < l_0b->platform_id_data_length) {
			struct dvb_platform_id *p_id = (struct dvb_platform_id *) (buf + pos + pos2);
			bswap32(buf+pos+pos2);
			pos2 += sizeof(struct dvb_platform_id) + p_id->platform_name_loop_length;
		}

		pos += pos2;
	} else if (p->linkage_type == 0x0c) {
		struct dvb_linkage_data_0c *l_0c = (struct dvb_linkage_data_0c *) buf + pos;

		if ((len - pos) < sizeof(struct dvb_linkage_data_0c))
			return NULL;

		if (l_0c->table_type == 0x02)
			bswap16(buf+pos+1);
	}

	return (struct dvb_linkage_descriptor*) d;
}

/**
 * Accessor for the data field of a dvb_linkage_descriptor.
 *
 * @param d dvb_linkage_descriptor pointer.
 * @return Pointer to the data field.
 */
static inline uint8_t *
	dvb_linkage_descriptor_data(struct dvb_linkage_descriptor *d)
{
	return (uint8_t *) d + sizeof(struct dvb_linkage_descriptor);
}

/**
 * Determine the length of the data field of a dvb_linkage_descriptor.
 *
 * @param d dvb_linkage_descriptor pointer.
 * @return Length of the field in bytes.
 */
static inline int
	dvb_linkage_descriptor_data_length(struct dvb_linkage_descriptor *d)
{
	return d->d.len - 7;
}

/**
 * Accessor for the data field of a dvb_linkage_data_08.
 *
 * @param d dvb_linkage_data_08 pointer.
 * @return Pointer to the data field.
 */
static inline uint8_t *
	dvb_linkage_data_08_data(struct dvb_linkage_data_08 *d)
{
	return (uint8_t *) d + sizeof(struct dvb_linkage_data_08);
}

/**
 * Determine the length of the data field of a dvb_linkage_data_08.
 *
 * @param d dvb_linkage_data_08 pointer.
 * @return Length of the field in bytes.
 */
static inline int
	dvb_linkage_data_08_data_length(struct dvb_linkage_descriptor *d)
{
	return dvb_linkage_descriptor_data_length(d) - sizeof(struct dvb_linkage_data_08);
}

/**
 * Iterator for the platform_id field of a dvb_linkage_data_0b.
 *
 * @param linkage dvb_linkage_data_0b pointer.
 * @param pos Variable containing a pointer to the current dvb_platform_id.
 */
#define dvb_dvb_linkage_data_0b_platform_id_for_each(linkage, pos) \
	for ((pos) = dvb_platform_id_first(linkage); \
	     (pos); \
	     (pos) = dvb_platform_id_next(linkage, pos))

/**
 * Iterator for the platform_name field of a dvb_platform_id.
 *
 * @param platid dvb_platform_id pointer.
 * @param pos Variable containing a pointer to the current dvb_platform_name.
 */
#define dvb_platform_id_platform_name_for_each(platid, pos) \
	for ((pos) = dvb_platform_name_first(platid); \
	     (pos); \
	     (pos) = dvb_platform_name_next(platid, pos))

/**
 * Accessor for the text field of a dvb_platform_name.
 *
 * @param p dvb_platform_name pointer.
 * @return Pointer to the field.
 */
static inline uint8_t *
	dvb_platform_name_text(struct dvb_platform_name *p)
{
	return (uint8_t *) p + sizeof(struct dvb_platform_name);
}

/**
 * Accessor for the bouquet_id field of a dvb_linkage_data_0c if table_id == 0x02.
 *
 * @param l_0c dvb_linkage_data_0c pointer.
 * @return The bouquet field, or -1 on error.
 */
static inline int
	dvb_linkage_data_0c_bouquet_id(struct dvb_linkage_data_0c *l_0c)
{
	uint8_t *b;

	if (l_0c->table_type != 0x02)
		return -1;

	b = (uint8_t *) l_0c + 1;
	return (int) (uint16_t) *b;
}







/******************************** PRIVATE CODE ********************************/
static inline struct dvb_platform_id *
	dvb_platform_id_first(struct dvb_linkage_data_0b *d)
{
	if (d->platform_id_data_length == 0)
		return NULL;

	return (struct dvb_platform_id *) ((uint8_t *) d + sizeof(struct dvb_linkage_data_0b));
}

static inline struct dvb_platform_id *
	dvb_platform_id_next(struct dvb_linkage_data_0b *d,
				    struct dvb_platform_id *pos)
{
	uint8_t *end = (uint8_t *) d + d->platform_id_data_length;
	uint8_t *next =	(uint8_t *) pos +
			sizeof(struct dvb_platform_id) +
			pos->platform_name_loop_length;

	if (next >= end)
		return NULL;

	return (struct dvb_platform_id *) next;
}

static inline struct dvb_platform_name *
	dvb_platform_name_first(struct dvb_platform_id *p)
{
	if (p->platform_name_loop_length == 0)
		return NULL;

	return (struct dvb_platform_name *) ((uint8_t *) p + sizeof(struct dvb_platform_id));
}

static inline struct dvb_platform_name *
	dvb_platform_name_next(struct dvb_platform_id *p,
				    struct dvb_platform_name *pos)
{
	uint8_t *end = (uint8_t *) p + p->platform_name_loop_length;
	uint8_t *next =	(uint8_t *) pos +
			sizeof(struct dvb_platform_name) +
			pos->platform_name_length;

	if (next >= end)
		return NULL;

	return (struct dvb_platform_name *) next;
}

#ifdef __cplusplus
}
#endif

#endif
