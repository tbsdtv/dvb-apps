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

#ifndef _UCSI_MPEG_ODSMT_SECTION_H
#define _UCSI_MPEG_ODSMT_SECTION_H 1

#include <ucsi/section.h>

struct mpeg_odsmt_section {
	struct section_ext head;

	uint8_t stream_count;
	/* stream_count==0 => struct mpeg_odsmt_stream_single streams
	   stream_count>0  => struct mpeg_odsmt_stream_multi streams[] */
	/* uint8_t object_descriptors[] */
} packed;

struct mpeg_odsmt_stream {
	union {
		struct mpeg_odsmt_stream_single {
			uint16_t esid;
			uint8_t es_info_length;
			/* struct descriptor descriptors[] */
		} single packed;

		struct mpeg_odsmt_stream_multi {
			uint16_t esid;
			uint8_t fmc;
			uint8_t es_info_length;
			/* struct descriptor descriptors[] */
		} multi packed;
	} u packed;
} packed;

extern struct mpeg_odsmt_section *mpeg_odsmt_section_parse(struct section_ext *);

#define mpeg_odsmt_section_streams_for_each(osdmt, pos, index) \
	for (index=0, (pos) = mpeg_odsmt_section_streams_first(odsmt); \
	     (pos); \
	     (pos) = mpeg_odsmt_section_streams_next(odsmt, pos, ++index))

#define mpeg_odsmt_stream_descriptors_for_each(osdmt, stream, pos) \
	for ((pos) = mpeg_odsmt_stream_descriptors_first(odsmt, stream); \
	     (pos); \
	     (pos) = mpeg_odsmt_stream_descriptors_next(odsmt, stream, pos))

static inline uint8_t*
	mpeg_odsmt_section_object_descriptors(struct mpeg_odsmt_section * odsmt,
					      uint32_t* len);










/******************************** PRIVATE CODE ********************************/
static inline struct mpeg_odsmt_stream *
	mpeg_odsmt_section_streams_first(struct mpeg_odsmt_section *odsmt)
{
	int pos = sizeof(struct mpeg_odsmt_section);

	if (pos >= section_ext_length(&odsmt->head))
		return NULL;

	return (struct mpeg_odsmt_stream *) ((uint8_t *) odsmt + pos);
}

static inline struct mpeg_odsmt_stream *
	mpeg_odsmt_section_streams_next(struct mpeg_odsmt_section *odsmt,
					struct mpeg_odsmt_stream *pos,
				        int index)
{
	uint8_t *end = (uint8_t*) odsmt + section_ext_length(&odsmt->head);
	uint8_t *next;

	if (index > odsmt->stream_count)
		return NULL;

	next = (uint8_t *) pos + sizeof(struct mpeg_odsmt_stream_multi) +
		pos->u.multi.es_info_length;

	if (next >= end)
		return NULL;

	return (struct mpeg_odsmt_stream *) next;
}

static inline struct descriptor *
	mpeg_odsmt_stream_descriptors_first(struct mpeg_odsmt_section *odsmt,
				      	    struct mpeg_odsmt_stream *stream)
{
	if (odsmt->stream_count == 0) {
		if (stream->u.single.es_info_length == 0)
			return NULL;

		return (struct descriptor *)
			((uint8_t*) stream + sizeof(struct mpeg_odsmt_stream_single));
	} else {
		if (stream->u.multi.es_info_length == 0)
			return NULL;

		return (struct descriptor *)
			((uint8_t*) stream + sizeof(struct mpeg_odsmt_stream_multi));
	}
}

static inline struct descriptor *
	mpeg_odsmt_stream_descriptors_next(struct mpeg_odsmt_section *odsmt,
					   struct mpeg_odsmt_stream *stream,
					   struct descriptor* pos)
{
	if (odsmt->stream_count == 0) {
		return next_descriptor((uint8_t *) stream + sizeof(struct mpeg_odsmt_stream_single),
				       stream->u.single.es_info_length,
				       pos);
	} else {
		return next_descriptor((uint8_t *) stream + sizeof(struct mpeg_odsmt_stream_multi),
				       stream->u.multi.es_info_length,
				       pos);
	}
}

static inline uint8_t*
	mpeg_odsmt_section_object_descriptors(struct mpeg_odsmt_section * odsmt,
					      uint32_t* len)
{
	struct mpeg_odsmt_stream* pos;
	int size = sizeof(struct mpeg_odsmt_section);
	int index;

	mpeg_odsmt_section_streams_for_each(odsmt, pos, index) {
		if (odsmt->stream_count == 0)
			size += sizeof(struct mpeg_odsmt_stream_single) +
					pos->u.single.es_info_length;
		else
			size += sizeof(struct mpeg_odsmt_stream_multi) +
					pos->u.multi.es_info_length;
	}

	*len = section_ext_length(&odsmt->head) - size;
	return (uint8_t*) odsmt + size;
}

#endif
