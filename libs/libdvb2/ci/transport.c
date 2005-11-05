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

static int send_tpdu_generic(struct dvb_ci_connection * connection,
			     struct dvb_ci_tpdu * tpdu);
static int generic_close(struct dvb_ci_connection * conn);

int dvb_ci_create_tpdu(struct dvb_ci_connection * conn,
		       uint8_t tag, uint8_t data,
		       struct dvb_ci_tpdu ** ptr)
{
	struct dvb_ci_tpdu * tpdu = malloc(sizeof(struct dvb_ci_tpdu));

	if (tpdu == NULL)
		return -ENOMEM;

	tpdu->conn = conn;
	tpdu->tag = tag;

	switch (tag) {
		case dvb_ci_tct_create:
		case dvb_ci_tct_create_reply:
		case dvb_ci_tct_delete:
		case dvb_ci_tct_delete_reply:
		case dvb_ci_tct_request:
		case dvb_ci_tct_receive:
			tpdu->len = 1;
			tpdu->data = malloc(1);
			if (tpdu->data == NULL) {
				free(tpdu);
				return -ENOMEM;
			}
			tpdu->data[0] = conn->id;
			break;

		case dvb_ci_tct_new:
		case dvb_ci_tct_error:
		case dvb_ci_tct_status:
			tpdu->len = 2;
			tpdu->data = malloc(2);
			if (tpdu->data == NULL) {
				free(tpdu);
				return -ENOMEM;
			}
			tpdu->data[0] = conn->id;
			tpdu->data[1] = data;
			break;

		case dvb_ci_tct_data_more:
		case dvb_ci_tct_data_last:
			tpdu->len = 0;
			tpdu->data = NULL;
			break;
	}

	*ptr = tpdu;
	return 0;
}


int dvb_ci_create_connection(struct dvb_ci_slot * slot,
			     struct dvb_ci_connection ** connptr)
{
	struct dvb_ci_tpdu * tpdu;
	struct dvb_ci_connection * conn;

	assert(slot->send_tpdu);

	conn = malloc(sizeof(struct dvb_ci_connection));
	if (conn == NULL)
		return -ENOMEM;

	memset(conn, 0, sizeof(struct dvb_ci_connection));

	LIST_ENTRY_INIT(&conn->outqueue);
	LIST_ENTRY_INIT(&conn->inqueue);
	LIST_ENTRY_INIT(&conn->data);
	LIST_ENTRY_INIT(&conn->sessions);

	conn->ci = slot->ci;
	conn->slot = slot;
	conn->id = ++slot->ncid;
	conn->state = dvb_ci_tpconn_create;
	conn->send = send_tpdu_generic;
	conn->close = generic_close;

	if (dvb_ci_create_tpdu(conn, dvb_ci_tct_create, conn->id, &tpdu)) {
		free(conn);
		return -ENOMEM;
	}

	slot->send_tpdu(slot, tpdu);
	free(tpdu->data);
	free(tpdu);

	list_append_entry(&slot->connections, &conn->list);

	if (connptr)
		*connptr = conn;

	return 0;
}

static void flush_connection(struct dvb_ci_connection * conn)
{
	struct list_entry * pos, * next;
	struct dvb_ci_tpdu * tpdu;

	for (pos = conn->outqueue.next; pos; pos = next) {
		next = pos->next;
		tpdu = list_get_entry(pos, struct dvb_ci_tpdu, list);
		list_remove_entry(&conn->outqueue, pos);
		free(tpdu);
	}

	for (pos = conn->inqueue.next; pos; pos = next) {
		next = pos->next;
		tpdu = list_get_entry(pos, struct dvb_ci_tpdu, list);
		list_remove_entry(&conn->inqueue, pos);
		free(tpdu);
	}

	for (pos = conn->data.next; pos; pos = next) {
		next = pos->next;
		tpdu = list_get_entry(pos, struct dvb_ci_tpdu, list);
		list_remove_entry(&conn->data, pos);
		free(tpdu);
	}
}

static int generic_close(struct dvb_ci_connection * conn)
{
	struct list_entry * pos, * next;
	struct dvb_ci_session * session;

	flush_connection(conn);

	for (pos = conn->sessions.next; pos; pos = next) {
		next = pos->next;
		session = list_get_entry(pos, struct dvb_ci_session, list);
		dvb_ci_close_session(session);
	}

	list_remove_entry(&conn->slot->connections, &conn->list);
	free(conn);
	return 0;
}

static int handle_tpdu_create_reply(struct dvb_ci_tpdu * tpdu)
{
	struct dvb_ci_tpdu * out;
	tpdu->conn->state = dvb_ci_tpconn_active;

	if (dvb_ci_create_tpdu(tpdu->conn, dvb_ci_tct_receive,
			       tpdu->conn->id, &out)) {
		return -ENOMEM;
	}

	dvb_ci_send_tpdu(tpdu->conn, out);
	return 0;
}

static int handle_tpdu_status(struct dvb_ci_tpdu * tpdu)
{
	struct dvb_ci_connection * conn = tpdu->conn;
	struct dvb_ci_tpdu * out;

	clock_gettime(CLOCK_MONOTONIC, &conn->laststatus);

	if (tpdu->data[1] & 0x80) {
		if (dvb_ci_create_tpdu(conn, dvb_ci_tct_receive, conn->id, &out)) {
			return -ENOMEM;
		}

		dvb_ci_send_tpdu(conn, out);
	}

	return 0;
}

static struct dvb_ci_session * find_session(struct dvb_ci_connection * conn,
					    struct dvb_ci_spdu * spdu)
{
	struct dvb_ci_session * session;
	struct list_entry * pos;
	uint16_t id;

	if (spdu->tag == dvb_ci_sot_open_request)
		return NULL;

	id = spdu->data[spdu->len - 2] << 8 |
	     spdu->data[spdu->len - 1];

	for (pos = conn->sessions.next; pos != NULL; pos = pos->next) {
		session = list_get_entry(pos, struct dvb_ci_session, list);

		if (session->id == id)
			return session;
	}

	return NULL;
}

static int dvb_ci_tpdu_parse_data(struct dvb_ci_connection * conn,
		                  uint8_t * buf, int buflen)
{
	struct dvb_ci_session * session;
	struct dvb_ci_spdu * spdu;
	int pos = 0;

	uint8_t tag = buf[pos++];
	int len = decode_asn_len(buf, &pos);

	if (dvb_ci_create_spdu(tag, len, buf + pos, &spdu))
		return -ENOMEM;

	pos += len;

	session = find_session(conn, spdu);

	dprintf("[%.2i:%.2i:%.2i] <<<< spdu: %s\n",
		conn->slot->num, conn->id,
		session ? session->id : 0, sot_name(spdu->tag));

	while (pos < buflen) {
		struct dvb_ci_apdu * apdu;

		apdu = malloc(sizeof(struct dvb_ci_apdu));
		if (apdu == NULL)
			return -ENOMEM;

		apdu->tag  = buf[pos++] << 16;
		apdu->tag |= buf[pos++] << 8;
		apdu->tag |= buf[pos++] << 0;
		apdu->len = decode_asn_len(buf, &pos);

		apdu->data = malloc(apdu->len);
		if (apdu->data == NULL)
			return -ENOMEM;

		memcpy(apdu->data, buf + pos, apdu->len);
		pos += apdu->len;
		list_append_entry(&spdu->apdu, &apdu->list);

		dprintf("[%.2i:%.2i:%.2i] <<<< apdu: %s\n",
			session->conn->slot->num, session->conn->id,
			session->id, aot_name(apdu->tag));
	}

	assert(pos == buflen);

	if (spdu->tag == dvb_ci_sot_open_request) {
		dvb_ci_create_session(conn, spdu);
	} else {
		dvb_ci_recv_spdu(session, spdu);
	}

	return 0;
}

static int handle_tpdu_more_last(struct dvb_ci_tpdu * tpdu)
{
	struct dvb_ci_connection * conn = tpdu->conn;
	struct list_entry * pos, * next;
	uint8_t * data;
	int len = 0, dpos = 0;

	list_append_entry(&conn->data, &tpdu->list);

	if (tpdu->tag == dvb_ci_tct_data_more)
		return 0;

	for (pos = conn->data.next; pos != NULL; pos = pos->next) {
		tpdu = list_get_entry(pos, struct dvb_ci_tpdu, list);

		assert(conn->id == tpdu->data[0]);
		len += tpdu->len - 1; // len - tcid
	}

	data = malloc(len);
	if (data == NULL)
		return -ENOMEM;

	for (pos = conn->data.next; pos != NULL; pos = next) {
		next = pos->next;
		tpdu = list_get_entry(pos, struct dvb_ci_tpdu, list);

		memcpy(data + dpos, tpdu->data + 1, tpdu->len - 1);
		dpos += tpdu->len - 1;
		list_remove_entry(&conn->data, pos);
		free(tpdu);
	}

#if 0
	fprintf(stderr, "< < < < TPDU [%.2i] [%4i]", tpdu->conn->slot->num, len);
	for (int i = 0; i < len; ++i)
		fprintf(stderr, " 0x%.2x", data[i]);
	fprintf(stderr, "\n");
#endif

	dvb_ci_tpdu_parse_data(conn, data, len);
	free(data);
	return 0;
}

int dvb_ci_recv_tpdu(struct dvb_ci_connection * conn,
		     struct dvb_ci_tpdu * tpdu)
{
	int ret = 0;

	dprintf("[%.2i:%.2i:  ] <<<< tpdu: %s\n",
		conn->slot->num, conn->id, tct_name(tpdu->tag));

	tpdu->conn->slot->state = dvb_ci_slot_state_idle;
	clock_gettime(CLOCK_MONOTONIC, &tpdu->conn->slot->lastread);

	switch (tpdu->tag) {
		case dvb_ci_tct_create_reply:
			ret = handle_tpdu_create_reply(tpdu);
			break;

		case dvb_ci_tct_status:
			ret = handle_tpdu_status(tpdu);
			break;

		case dvb_ci_tct_data_more:
		case dvb_ci_tct_data_last:
			ret = handle_tpdu_more_last(tpdu);
			if (ret == 0)
				return 0;
			break;

		case dvb_ci_tct_create:
		case dvb_ci_tct_delete:
		case dvb_ci_tct_delete_reply:
		case dvb_ci_tct_receive:
		case dvb_ci_tct_request:
		case dvb_ci_tct_new:
		case dvb_ci_tct_error:
		default:
			fprintf(stderr, "unknown tag 0x%.2x\n", tpdu->tag);
	}

	free(tpdu->data);
	free(tpdu);
	return 0;
}

int dvb_ci_send_tpdu(struct dvb_ci_connection * connection,
		     struct dvb_ci_tpdu * tpdu)
{
	return connection->send(connection, tpdu);
}

static int send_tpdu_generic(struct dvb_ci_connection * connection,
			     struct dvb_ci_tpdu * tpdu)
{
	list_append_entry(&connection->outqueue, &tpdu->list);
	return 0;
}

int dvb_ci_send_tpdu_data(struct dvb_ci_connection * conn,
		          int len, const uint8_t * data)
{
	struct dvb_ci_tpdu * tpdu;
	int cur, written = 0;

	while (written < len) {
		tpdu = malloc(sizeof(struct dvb_ci_tpdu));

		if (tpdu == NULL)
			return -ENOMEM;

		cur = len;
		if (cur > 256 - 4)
			cur = 256 - 4;

		tpdu->conn = conn;
		tpdu->len = cur + 1;
		tpdu->data = malloc(cur + 1);

		if (tpdu->data == NULL) {
			free(tpdu);
			return -ENOMEM;
		}

		tpdu->data[0] = conn->id;
		memcpy(tpdu->data + 1, data + written, cur);

		if (written + cur < len)
			tpdu->tag = dvb_ci_tct_data_more;
		else
			tpdu->tag = dvb_ci_tct_data_last;

		dvb_ci_send_tpdu(conn, tpdu);

		dprintf("[%.2i:%.2i:  ] >>>> tpdu: %s\n",
			conn->slot->num, conn->id, tct_name(tpdu->tag));

#if 0
		fprintf(stderr, "> > > > TPDU [%.2i] [%4i]", conn->slot->num, cur);
		for (int i = 0; i < cur; ++i)
			fprintf(stderr, " 0x%.2x", data[i]);
		fprintf(stderr, "\n");
#endif

		written += cur;
	}

	return 0;
}

