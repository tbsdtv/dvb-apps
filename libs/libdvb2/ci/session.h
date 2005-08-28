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

#ifndef _DVB_CI_SESSION_H_
#define _DVB_CI_SESSION_H_

struct dvb_ci_slot;
struct dvb_ci_spdu;

enum dvb_ci_connection_state {
	dvb_ci_tpconn_idle,
	dvb_ci_tpconn_create,
	dvb_ci_tpconn_active,
	dvb_ci_tpconn_delete,
};

struct dvb_ci_session {
	struct list_entry list;

	struct dvb_ci * ci;
	struct dvb_ci_slot * slot;
	struct dvb_ci_connection * conn;

	uint16_t id;
	uint32_t resid;

	struct dvb_ci_application * app;

	int (*send) (struct dvb_ci_session *, struct dvb_ci_spdu *);
};

enum dvb_ci_sot {
	dvb_ci_sot_open_request		= 0x91,
	dvb_ci_sot_open_response	= 0x92,
	dvb_ci_sot_create		= 0x93,
	dvb_ci_sot_create_response	= 0x94,
	dvb_ci_sot_close_request	= 0x95,
	dvb_ci_sot_close_response	= 0x96,
	dvb_ci_sot_number		= 0x90,
};

struct dvb_ci_spdu {
	struct list_entry list;

	uint8_t tag;
	int len;
	uint8_t * data;

	struct list_entry apdu;
};

int dvb_ci_create_spdu(uint8_t tag, int len, const uint8_t * data,
		       struct dvb_ci_spdu ** ptr);

int dvb_ci_create_session(struct dvb_ci_connection * connection,
			  struct dvb_ci_spdu * spdu);

int dvb_ci_close_session(struct dvb_ci_session *);

int dvb_ci_recv_spdu(struct dvb_ci_session * session,
		     struct dvb_ci_spdu * spdu);

int dvb_ci_send_spdu(struct dvb_ci_session * session,
		     struct dvb_ci_spdu * spdu);

#endif

