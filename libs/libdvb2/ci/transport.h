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

#ifndef _DVB_CI_TRANSPORT_H_
#define _DVB_CI_TRANSPORT_H_

struct dvb_ci_tpdu;

struct dvb_ci_connection {
	struct list_entry list;

	struct dvb_ci * ci;
	struct dvb_ci_slot * slot;

	int nsid; /* next session id */

	int id;
	enum dvb_ci_connection_state state;
	unsigned int polled : 1;

	struct timespec laststatus;

	struct list_entry outqueue;
	struct list_entry inqueue;

	struct list_entry data;
	struct list_entry sessions;

	int (*send) (struct dvb_ci_connection *, struct dvb_ci_tpdu *);
	int (*close) (struct dvb_ci_connection *);
};

enum dvb_ci_tct {
	dvb_ci_tct_status	= 0x80,
	dvb_ci_tct_receive	= 0x81,
	dvb_ci_tct_create	= 0x82,
	dvb_ci_tct_create_reply	= 0x83,
	dvb_ci_tct_delete	= 0x84,
	dvb_ci_tct_delete_reply	= 0x85,
	dvb_ci_tct_request	= 0x86,
	dvb_ci_tct_new		= 0x87,
	dvb_ci_tct_error	= 0x88,
	dvb_ci_tct_data_more	= 0xA1,
	dvb_ci_tct_data_last	= 0xA0,
};

struct dvb_ci_tpdu {
	struct list_entry list;
	struct dvb_ci_connection * conn;

	uint8_t tag;
	int len;
	uint8_t * data;
};

int dvb_ci_create_tpdu(struct dvb_ci_connection * conn,
		       uint8_t tag, uint8_t data,
		       struct dvb_ci_tpdu ** ptr);

int dvb_ci_create_connection(struct dvb_ci_slot * slot,
			     struct dvb_ci_connection ** connptr);

int dvb_ci_recv_tpdu(struct dvb_ci_connection * connection,
		     struct dvb_ci_tpdu * tpdu);

int dvb_ci_send_tpdu(struct dvb_ci_connection * connection,
		     struct dvb_ci_tpdu * tpdu);

int dvb_ci_send_tpdu_data(struct dvb_ci_connection * conn,
		          int len, const uint8_t * data);

#endif

