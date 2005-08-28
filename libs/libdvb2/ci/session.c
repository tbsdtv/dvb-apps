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

#include <ci/ci.h>
#include <ci/internal.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

static int send_spdu_generic(struct dvb_ci_session * session,
			     struct dvb_ci_spdu * spdu);

uint32_t dvb_ci_get_session_resource_id(const struct dvb_ci_session * session)
{
	return session->resid;
}

int dvb_ci_create_spdu(uint8_t tag, int len, const uint8_t * data,
		       struct dvb_ci_spdu ** ptr)
{
	struct dvb_ci_spdu * spdu;

	spdu = malloc(sizeof(struct dvb_ci_spdu));
	if (spdu == NULL)
		return -ENOMEM;

	LIST_ENTRY_INIT(&spdu->apdu);
	spdu->tag = tag;
	spdu->len = len;
	spdu->data = malloc(spdu->len);

	if (spdu->data == NULL) {
		free(spdu);
		return -ENOMEM;
	}

	if (data != NULL)
		memcpy(spdu->data, data, spdu->len);

	*ptr = spdu;
	return 0;
}

int dvb_ci_create_session(struct dvb_ci_connection * conn,
			  struct dvb_ci_spdu * spdu)
{
	struct dvb_ci_session * session;
	struct dvb_ci_spdu * ret;

	session = malloc(sizeof(struct dvb_ci_session));
	if (session == NULL)
		return -ENOMEM;

	memset(session, 0, sizeof(struct dvb_ci_session));
	session->ci   = conn->ci;
	session->slot = conn->slot;
	session->conn = conn;

	session->id = ++conn->nsid; /* XXX */
	session->resid = spdu->data[0] << 24 | spdu->data[1] << 16 |
			 spdu->data[2] << 8  | spdu->data[3];
	session->send = send_spdu_generic;

	list_append_entry(&conn->sessions, &session->list);

	if (dvb_ci_create_spdu(dvb_ci_sot_open_response, 7, NULL, &ret)) {
		free(session);
		return -ENOMEM;
	}

	ret->data[0] = 0; // status ok
	ret->data[1] = session->resid >> 24;
	ret->data[2] = session->resid >> 16;
	ret->data[3] = session->resid >> 8;
	ret->data[4] = session->resid;
	ret->data[5] = session->id >> 8;
	ret->data[6] = session->id;

	dvb_ci_send_spdu(session, ret);

	dvb_ci_new_session(conn->slot->ci, session);
	return 0;
}

int dvb_ci_close_session(struct dvb_ci_session * session)
{
	dvb_ci_remove_session(session->ci, session);
	list_remove_entry(&session->conn->sessions, &session->list);
	free(session);
	return 0;
}

static int handle_spdu_session_number(struct dvb_ci_session * session,
				      struct dvb_ci_spdu * spdu)
{
	struct dvb_ci_apdu * apdu;
	struct list_entry * pos;

	for(pos = spdu->apdu.next; pos != NULL; pos = pos->next) {
		apdu = list_get_entry(pos, struct dvb_ci_apdu, list);

		fprintf(stderr, "[%.2i:%.2i:%.2i] <<<< apdu: %s\n",
			session->conn->slot->num, session->conn->id,
			session->id, aot_name(apdu->tag));

		dvb_ci_recv_apdu(session, apdu);
	}

	return 0;
}

int dvb_ci_recv_spdu(struct dvb_ci_session * session,
		     struct dvb_ci_spdu * spdu)
{
	if (spdu->tag != dvb_ci_sot_open_request)
		assert(session);

	switch (spdu->tag) {

		case dvb_ci_sot_number:
			handle_spdu_session_number(session, spdu);
			break;
#if 0
		case dvb_ci_sot_create:
		case dvb_ci_sot_close_request:

		case dvb_ci_sot_open_response:
		case dvb_ci_sot_create_response:
		case dvb_ci_sot_close_response:
#endif
		default:
			fprintf(stderr, "unknown session tag 0x%x len %i\n",
				spdu->tag, spdu->len);
	}

	return 0;
}

int dvb_ci_send_spdu(struct dvb_ci_session * session,
		     struct dvb_ci_spdu * spdu)
{
	return session->send(session, spdu);
}

static int send_spdu_generic(struct dvb_ci_session * session,
			     struct dvb_ci_spdu * spdu)
{
	struct dvb_ci_apdu * apdu;
	struct list_entry * lpos;
	uint8_t * buf;
	int len = spdu->len + 2;
	int pos = 0;

	dprintf("[%.2i:%.2i:%.2i] >>>> spdu: %s\n",
	      session->conn->slot->num, session->conn->id,
	      session ? session->id : 0, sot_name(spdu->tag));

	for(lpos = spdu->apdu.next; lpos != NULL; lpos = lpos->next) {
		apdu = list_get_entry(lpos, struct dvb_ci_apdu, list);

		assert(apdu->len < 0x80);
		len += apdu->len + 3 + 1;
	}

	buf = malloc(len);
	if (buf == NULL)
		return -ENOMEM;

	buf[pos++] = spdu->tag;
	buf[pos++] = spdu->len;

	memcpy(buf + pos, spdu->data, spdu->len);
	pos += spdu->len;

	for(lpos = spdu->apdu.next; lpos != NULL; lpos = lpos->next) {
		apdu = list_get_entry(lpos, struct dvb_ci_apdu, list);

		buf[pos++] = apdu->tag >> 16;
		buf[pos++] = apdu->tag >> 8;
		buf[pos++] = apdu->tag;
		encode_asn_len(apdu->len, buf, &pos);
		if (apdu->len) {
			memcpy(buf + pos, apdu->data, apdu->len);
			pos += apdu->len;
		}

		fprintf(stderr, "[%.2i:%.2i:%.2i] >>>> apdu: %s\n",
			session->conn->slot->num, session->conn->id,
			session->id, aot_name(apdu->tag));
	}

	return dvb_ci_send_tpdu_data(session->conn, len, buf);
}

