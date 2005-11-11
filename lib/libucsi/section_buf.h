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

#ifndef _UCSI_SECTION_BUF_H
#define _UCSI_SECTION_BUF_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

#define DVB_MAX_SECTION_BYTES 4096

/**
 * Buffer used to keep track of  section fragments. You should allocate an
 * area of memory of size (sizeof(section_buf) + <maxsectionsize>), and pass that area
 * to section_buf_init() to set it up.
 */
struct section_buf {
	uint32_t max;
	uint32_t count;
	uint32_t len;
	uint8_t header:1;
	/* uint8_t data[] */
};

/**
 * Initialise a section_buf structure.
 *
 * @param section The section_buf to initialise.
 * @param max Maximum number of bytes in section (must be > 3)
 * @return 0 on success, nonzero on error.
 */
extern int section_buf_init(struct section_buf *section, int max);

/**
 * Add a data fragment to a section_buf.
 *
 * @param section section_buf to add to.
 * @param frag Pointer to data fragment.
 * @param len Number of bytes of data.
 * @return 0 If section was already complete. >0 indicates how many bytes were
 * consumed.
 * -ERANGE indicates that the section is larger than section->max.
 * Note in this case, section->len is still updated, so you know how much data
 * needs to be skipped.
 */
extern int section_buf_add(struct section_buf *section, uint8_t* frag, int len);

/**
 * Add a transport packet PSI payload to a section_buf. This takes into account
 * the extra byte present in PDU_START flagged packets.
 *
 * @param section section_buf to add to.
 * @param payload Pointer to packet payload data.
 * @param len Number of bytes of data.
 * @param pdu_start True if the payload_unit_start_indicator flag was set in the
 * TS packet.
 * @return 0 If section was already complete. >0 indicates how many bytes were
 * consumed.
 * -ERANGE indicates that the section is larger than section->max.
 * Note in this case, section->len is still updated, so you know how much data
 * needs to be skipped. Or else the combination of pointer_field/pdu_start
 * meant either the complete data was never received, or too much data was
 * received.
 * -EINVAL indicates the pointer_field was completely invalid (too large).
 */
extern int section_buf_add_transport_payload(struct section_buf *section,
					     uint8_t* payload, int len,
					     int pdu_start);

/**
 * Get the number of bytes left to be received in a section_buf.
 *
 * @param section The section_buf concerned.
 * @return The number of bytes.
 */
static inline int section_buf_remaining(struct section_buf *section)
{
	return section->len - section->count;
}

/**
 * Return a pointer to the start of the data in the section_buf.
 *
 * @param section The section_buf concerned.
 * @return The data.
 */
static inline uint8_t* section_buf_data(struct section_buf *section)
{
	return (uint8_t*) section + sizeof(struct section_buf);
}

#ifdef __cplusplus
}
#endif

#endif
