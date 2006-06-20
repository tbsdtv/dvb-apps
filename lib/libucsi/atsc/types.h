/*
 * section and descriptor parser
 *
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

#ifndef _UCSI_ATSC_TYPES_H
#define _UCSI_ATSC_TYPES_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <time.h>
#include <libucsi/types.h>

enum atsc_vct_modulation {
	ATSC_VCT_MODULATION_ANALOG 	= 0x01,
	ATSC_VCT_MODULATION_SCTE_MODE1 	= 0x02,
	ATSC_VCT_MODULATION_SCTE_MODE2 	= 0x03,
	ATSC_VCT_MODULATION_8VSB 	= 0x04,
	ATSC_VCT_MODULATION_16VSB 	= 0x05,
};

enum atsc_vct_service_type {
	ATSC_VCT_SERVICE_TYPE_ANALOG 	= 0x01,
	ATSC_VCT_SERVICE_TYPE_TV 	= 0x02,
	ATSC_VCT_SERVICE_TYPE_AUDIO 	= 0x03,
	ATSC_VCT_SERVICE_TYPE_DATA 	= 0x04,
};

enum atsc_etm_location {
	ATSC_VCT_ETM_NONE	 	= 0x00,
	ATSC_VCT_ETM_IN_THIS_PTC 	= 0x01,
	ATSC_VCT_ETM_IN_CHANNEL_TSID 	= 0x02,
};

typedef uint32_t atsctime_t;

struct atsc_text {
	uint8_t number_strings;
	/* struct atsc_text_string strings[] */
};

struct atsc_text_string {
	iso639lang_t language_code;
	uint8_t number_segments;
	/* struct atsc_text_string_segment segments[] */
};

struct atsc_text_string_segment {
	uint8_t compression_type;
	uint8_t mode;
	uint8_t number_bytes;
	/* uint8_t bytes[] */
};

/**
 * Iterator for strings field of an atsc_text structure.
 *
 * @param txt atsc_text pointer.
 * @param pos Variable holding a pointer to the current atsc_text_string.
 * @param idx Iterator variable.
 */
#define atsc_text_strings_for_each(txt, pos, idx) \
	for ((pos) = atsc_text_strings_first(txt), idx=0; \
	     (pos); \
	     (pos) = atsc_text_strings_next(txt, pos, idx), idx++)

/**
 * Iterator for segments field of an atsc_text_string structure.
 *
 * @param str atsc_text_string pointer.
 * @param pos Variable holding a pointer to the current atsc_text_string_segment.
 * @param idx Iterator variable.
 */
#define atsc_text_string_segments_for_each(txt, pos, idx) \
	for ((pos) = atsc_text_string_segments_first(txt), idx=0; \
	     (pos); \
	     (pos) = atsc_text_string_segments_next(txt, pos, idx), idx++)

/**
 * Accessor for the bytes field of an atsc_text_string_segment.
 *
 * @param seg atsc_text_string_segment pointer.
 * @return Pointer to the bytes.
 */
static inline uint8_t*
	atsc_text_string_segment_bytes(struct atsc_text_string *d)
{
	return ((uint8_t*) d) + sizeof(struct atsc_text_string_segment);
}

/**
 * Parse a buffer containing an atsc_text structure.
 *
 * @param buf Start of the atsc_text structure.
 * @param len Length in bytes of the buffer.
 * @return atsc_text pointer if valid, NULL if not.
 */
extern struct atsc_text *atsc_text_parse(uint8_t *buf, int len);

/**
 * Decompress a huffman encoded program title string segment.
 *
 * @param buf Pointer to the segment buffer.
 * @param buflen Length of the segment buffer.
 * @param dest Destination for the malloc()ed decompressed string.
 * @param destlen Destination for the length of the decompressed string.
 * @return 0 on success, nonzero on failure.
 */
extern int atsc_text_decode_program_title_segment(uint8_t *buf, size_t buflen,
					     uint8_t **dest, size_t *destlen);

/**
 * Decompress a huffman encoded program description string segment.
 *
 * @param buf Pointer to the segment buffer.
 * @param buflen Length of the segment buffer.
 * @param dest Destination for the malloc()ed decompressed string.
 * @param destlen Destination for the length of the decompressed string.
 * @return 0 on success, nonzero on failure.
 */
extern int atsc_text_decode_program_description_segment(uint8_t *buf, size_t buflen,
					           uint8_t **dest, size_t *destlen);

extern time_t atsctime_to_unixtime(atsctime_t atsc);
extern atsctime_t unixtime_to_atsctime(time_t atsc);







/******************************** PRIVATE CODE ********************************/
static inline struct atsc_text_string*
	atsc_text_strings_first(struct atsc_text *txt)
{
	if (txt->number_strings == 0)
		return NULL;

	return (struct atsc_text_string *)
		((uint8_t*) txt + sizeof(struct atsc_text));
}

static inline struct atsc_text_string*
	atsc_text_strings_next(struct atsc_text *txt, struct atsc_text_string *pos, int idx)
{
	int i;
	uint8_t *buf;

	if (idx >= txt->number_strings)
		return NULL;

	buf = ((uint8_t*) pos) + sizeof(struct atsc_text_string);
	for(i=0; i < pos->number_segments; i++) {
		struct atsc_text_string_segment *seg =
			(struct atsc_text_string_segment *) buf;

		buf += sizeof(struct atsc_text_string_segment);
		buf += seg->number_bytes;
	}

	return (struct atsc_text_string *) pos;
}

static inline struct atsc_text_string_segment*
	atsc_text_string_segments_first(struct atsc_text_string *str)
{
	if (str->number_segments == 0)
		return NULL;

	return (struct atsc_text_string_segment *)
		((uint8_t*) str + sizeof(struct atsc_text_string));
}

static inline struct atsc_text_string_segment*
	atsc_text_string_segments_next(struct atsc_text_string *str,
				       struct atsc_text_string_segment *pos, int idx)
{
	if (idx >= str->number_segments)
		return NULL;

	return (struct atsc_text_string_segment *)
		(((uint8_t*) pos) + sizeof(struct atsc_text_string_segment) + pos->number_bytes);
}

#ifdef __cplusplus
}
#endif

#endif
