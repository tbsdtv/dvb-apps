/*
 * section and descriptor parser
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
 * Copyright (C) 2005 Andrew de Quincey (adq_atsc@lidskialf.net)
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

#ifndef _UCSI_ATSC_AC3_DESCRIPTOR
#define _UCSI_ATSC_AC3_DESCRIPTOR 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <libucsi/descriptor.h>
#include <libucsi/endianops.h>

/**
 * atsc_ac3_descriptor structure.
 */
struct atsc_ac3_descriptor {
	struct descriptor d;

  EBIT2(uint8_t sample_rate_code        : 3; ,
	uint8_t bsid			: 5; );
  EBIT2(uint8_t bit_rate_code	        : 6; ,
	uint8_t surround_mode		: 2; );
  EBIT3(uint8_t bsmod		        : 6; ,
	uint8_t num_channels		: 4; ,
	uint8_t full_svc		: 1; );
	uint8_t langcod;
	/* uint8_t langcod2 if num_channels == 0 */
	/* struct atsc_ac3_descriptor_part2 part2 */
} __ucsi_packed;

struct atsc_ac3_descriptor_part2 {

	union {
		struct {
  		  EBIT3(uint8_t mainid		        : 3; ,
			uint8_t priority		: 2; ,
			uint8_t reserved		: 2; );
		} bsmodflags __ucsi_packed; /* if bsmod < 2 */

		uint8_t asvcflags; /* if bsmod >= 2 */
	} flags __ucsi_packed;

  EBIT2(uint8_t textlen			: 7; ,
	uint8_t text_code		: 1; );
	/* uint8_t text[] */
	/* struct atsc_ac3_descriptor_part3 part3 */
} __ucsi_packed;

struct atsc_ac3_descriptor_part3 {

  EBIT3(uint8_t language_flag		: 1; ,
	uint8_t language_flag_2		: 1; ,
	uint8_t reserved		: 6; );
	/* iso639lang_t language if (language_flag) */
	/* iso639lang_t language_2 if (language_flag_2) */
	/* uint8_t additional_info[] */
} __ucsi_packed;

/**
 * Process an atsc_ac3_descriptor.
 *
 * @param d Generic descriptor structure.
 * @return atsc_ac3_descriptor pointer, or NULL on error.
 */
static inline struct atsc_ac3_descriptor*
	atsc_ac3_descriptor_codec(struct descriptor* d)
{
	int pos = 0;
	uint8_t *buf = ((uint8_t*) d) + 2;
	struct atsc_ac3_descriptor *ac3 = (struct atsc_ac3_descriptor*) d;

	if (d->len < (pos+4))
		return NULL;
	pos += 4;

	if (ac3->num_channels == 0) {
		if (d->len < (pos + 1))
			return NULL;
		pos++;
	}

	if (d->len < (pos+2))
		return NULL;
	struct atsc_ac3_descriptor_part2 *part2 = (struct atsc_ac3_descriptor_part2*) (buf+pos);
	pos+=2;

	if (d->len < (pos+part2->textlen))
		return NULL;
	pos+=part2->textlen;

	if (d->len < (pos+1))
		return NULL;
	struct atsc_ac3_descriptor_part3 *part3 = (struct atsc_ac3_descriptor_part3*) (buf+pos);
	pos++;

	if (part3->language_flag) {
		if (d->len < (pos + 3))
			return NULL;
		pos+3;
	}
	if (part3->language_flag_2) {
		if (d->len < (pos + 3))
			return NULL;
		pos+3;
	}

	return (struct atsc_ac3_descriptor*) d;
}

/**
 * Accessor for the langcod2 field of an atsc_ac3_descriptor.
 *
 * @param d atsc_ac3_descriptor pointer.
 * @return The value, or -1 if not present
 */
static inline int atsc_ac3_descriptor_langcod2(struct atsc_ac3_descriptor *d)
{
	if (d->num_channels)
		return -1;

	int pos = sizeof(struct atsc_ac3_descriptor);
	return *(((uint8_t*) d) + pos);
}

/**
 * Accessor for the part2 field of an atsc_ac3_descriptor.
 *
 * @param d atsc_ac3_descriptor pointer.
 * @return struct atsc_ac3_descriptor_part2 pointer.
 */
static inline struct atsc_ac3_descriptor_part2 *
	atsc_ac3_descriptor_part2(struct atsc_ac3_descriptor *d)
{
	int pos = sizeof(struct atsc_ac3_descriptor);
	if (d->num_channels == 0)
		pos++;

	return (struct atsc_ac3_descriptor_part2 *) (((uint8_t*) d) + pos);
}

/**
 * Accessor for the text field of an atsc_ac3_descriptor_part2.
 *
 * @param part2 atsc_ac3_descriptor_part2 pointer.
 * @return Pointer to the text field
 */
static inline uint8_t *atsc_ac3_descriptor_part2_text(struct atsc_ac3_descriptor_part2 *part2)
{
	int pos = sizeof(struct atsc_ac3_descriptor_part2);

	return (((uint8_t*) part2) + pos);
}

/**
 * Accessor for the part3 field of an atsc_ac3_descriptor.
 *
 * @param part2 atsc_ac3_descriptor_part2 pointer.
 * @return struct atsc_ac3_descriptor_part3 pointer.
 */
static inline struct atsc_ac3_descriptor_part3 *
		atsc_ac3_descriptor_part3(struct atsc_ac3_descriptor_part2 *part2)
{
	int pos = sizeof(struct atsc_ac3_descriptor_part2);
	pos += part2->textlen;

	return (struct atsc_ac3_descriptor_part3 *) (((uint8_t*) part2) + pos);
}

/**
 * Accessor for the language field of an atsc_ac3_descriptor_part3.
 *
 * @param part3 atsc_ac3_descriptor_part3 pointer.
 * @return Pointer to the language field, or NULL if not present.
 */
static inline iso639lang_t *atsc_ac3_descriptor_part3_language(struct atsc_ac3_descriptor_part3 *part3)
{
	int pos = sizeof(struct atsc_ac3_descriptor_part3);

	if (!part3->language_flag)
		return NULL;

	return (iso639lang_t *) (((uint8_t*) part3) + pos);
}

/**
 * Accessor for the language_2 field of an atsc_ac3_descriptor_part3.
 *
 * @param part3 atsc_ac3_descriptor_part3 pointer.
 * @return Pointer to the language_2 field, or NULL if not present.
 */
static inline iso639lang_t *atsc_ac3_descriptor_part3_language_2(struct atsc_ac3_descriptor_part3 *part3)
{
	int pos = sizeof(struct atsc_ac3_descriptor_part3);

	if (part3->language_flag)
		pos += 3;
 	if (!part3->language_flag_2)
		return NULL;

	return (iso639lang_t *) (((uint8_t*) part3) + pos);
}

/**
 * Retrieve pointer to additional_info field of a atsc_ac3_descriptor.
 *
 * @param part3 atsc_ac3_descriptor_part3 pointer.
 * @return Pointer to additional_info field.
 */
static inline uint8_t *atsc_ac3_descriptor_additional_info(struct atsc_ac3_descriptor_part3 *part3)
{
	int pos = sizeof(struct atsc_ac3_descriptor_part3);

	if (part3->language_flag) {
		pos+=3;
	}
	if (part3->language_flag_2) {
		pos+=3;
	}

	return ((uint8_t *) part3) + pos;
}

/**
 * Determine length of additional_info field of a atsc_ac3_descriptor.
 *
 * @param part3 atsc_ac3_descriptor_part3 pointer.
 * @return Length of field in bytes.
 */
static inline int atsc_ac3_descriptor_additional_info_length(struct atsc_ac3_descriptor* d,
							     struct atsc_ac3_descriptor_part3 *part3)
{
	uint8_t *end = ((uint8_t*) d) + d->d.len;

	int pos = ((int) end - (int) part3) + sizeof(struct atsc_ac3_descriptor_part3);

	if (part3->language_flag) {
		pos+=3;
	}
	if (part3->language_flag_2) {
		pos+=3;
	}

	return pos;
}

#ifdef __cplusplus
}
#endif

#endif
