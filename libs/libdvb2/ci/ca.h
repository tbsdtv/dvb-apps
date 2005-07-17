/*
 * libdvb2 - dvb helper and wrapper library
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
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

#ifndef _DVB_CA_H_
#define _DVB_CA_H_

#include <stdint.h>
#include <si/mpeg/pmt_section.h>

struct dvb_ca;
struct dvb_ca_program;
struct dvb_ca_stream;

struct ca_pmt {
	uint8_t ca_pmt_list_management;
	uint16_t program_number;
	uint8_t reserved_1		: 2;
	uint8_t version_number		: 5;
	uint8_t current_next_indicator	: 1;
	uint16_t reserved_2		: 4;
	uint16_t program_info_length	:12;

	uint8_t ca_pmt_cmd_id;
	/* program_info_length descriptors */
	/* struct ca_pmt_stream [] */
};

struct ca_pmt_stream {
	uint8_t stream_type;
	uint16_t reserved_1	: 3;
	uint16_t pid		:13;
	uint16_t reserved_2	: 4;
	uint16_t es_info_length :12;

	uint8_t ca_pmt_cmd_id;
	/* es_info_length descriptors */
};

struct ca_pmt_reply {
	uint16_t program_number;
	uint8_t reserved_1		: 2;
	uint8_t version_number		: 5;
	uint8_t current_next_indicator	: 1;
	uint8_t ca_enable_flag		: 1;
	uint8_t ca_enable		: 7;

	/* struct ca_pmt_reply_stream [] */
};

struct ca_pmt_reply_stream {
	uint16_t reserved_1		: 3;
	uint16_t pid			:13;
	uint8_t ca_enable_flag		: 1;
	uint8_t ca_enable		: 7;
};

/* Reworked interface */
/**
 * Open a new conditional access handler for the ci context given.
 */
int dvb_create_ca(struct dvb_ca ** ptr);

/**
 * Free the ca context given, and any associated resources.
 */
int dvb_free_ca(struct dvb_ca *);

/**
 * Add a new session that can be used for conditional access management.
 */
int dvb_ca_add_session(struct dvb_ca *, struct dvb_ci_session *);

/**
 * Remove a previously added ci session.
 */
int dvb_ca_remove_session(struct dvb_ca *, struct dvb_ci_session *);

/**
 * Create a new ca program context, where streams can be (de)selected
 */
int dvb_ca_create_program(struct dvb_ca *, struct dvb_ca_program ** ptr);

/**
 * Free the previously created ca program context.
 */
int dvb_ca_free_program(struct dvb_ca_program *);

/**
 * Select the stream with given pid for decoding, if the
 * stream already was selected this function does nothing.
 */
int dvb_ca_select_stream(struct dvb_ca_program *, int pid);

/**
 * Deselect the stream with given pid, if the stream was
 * not selected this function does nothing.
 */
int dvb_ca_deselect_stream(struct dvb_ca_program *, int pid);

/**
 * Check if the stream with given pid is currently selected,
 * returns true (non-zero) if it is.
 */
int dvb_ca_stream_is_selected(struct dvb_ca_program *, int pid);

/**
 * Updates the given section, and (re)sends new conditional
 * access management messages to the modules if anything changed.
 */
int dvb_ca_update_section(struct dvb_ca_program *, 
			  const struct mpeg_pmt_section * section);

#endif

