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

#include <dvb/internal.h>
#include <dvb/frontend.h>
#include <dvb/notifier.h>
#include <dvb/plugin.h>
#include <ci/ci.h>
#include <ci/internal.h>

#include <linux/dvb/ca.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>

static int ci_probe(struct dvb_frontend *);
static int ci_init(struct dvb_ci *);
static int ci_exit(struct dvb_ci *);

static int ci_slot_close(struct dvb_ci_slot *);

static void ll_notifier(struct dvb_notifier * notifier,
			const struct pollfd * pfd);

static struct dvb_ci_plugin ci_private = {
	probe:	ci_probe,
	init:	ci_init,
	exit:	ci_exit,
};

DECLARE_PLUGIN("ci_linklayer", dvb_ci_plugin_type, &ci_private);

struct linklayer {
	int refcount;
	int fd;

	ca_caps_t caps;
	struct dvb_notifier * notifier;

	int nb_slots;
	struct dvb_ci_slot ** slots;

	int nsid; /* next session id */
};

static struct dvb_ci_slot * slot_by_id(struct linklayer * ll, int num);
static struct dvb_ci_connection * dvb_ci_connection_by_id(struct dvb_ci_slot * slot, int id);


static int linklayer_send(struct linklayer * ll, struct dvb_ci_tpdu * tpdu)
{
	uint8_t buf[2048];
	int pos = 0, written = 0;

	buf[pos++] = tpdu->conn->slot->num;
	buf[pos++] = tpdu->conn->id;
	buf[pos++] = tpdu->tag;
	encode_asn_len(tpdu->len, buf, &pos);

	memcpy(buf + pos, tpdu->data, tpdu->len);
	pos += tpdu->len;

	while (written < pos) {
		int ret = write(ll->fd, buf + written, pos - written);
		if (ret < 0 && errno == EAGAIN)
			continue;
		if (ret < 0) {
			fprintf(stderr, "write error\n");
			return ret;
		}
		written += ret;
		if (written < ret)
			fprintf(stderr, "warning...\n");
	}

#if 0
	if (tpdu->tag != dvb_ci_tct_receive) {
		int i;
#if 0
		fprintf(stderr, "> > > > slot %i cid %i tag 0x%.2x len %i (%s)\n",
			tpdu->conn->slot->num, tpdu->conn->id,
			tpdu->tag, tpdu->len, tp_tag_name(tpdu->tag));
#endif

		fprintf(stderr, "> > > > SEND [%.2i] [%4i]", tpdu->conn->slot->num, written);
		for (i = 0; i < written; ++i)
			fprintf(stderr, " 0x%.2x", buf[i]);
		fprintf(stderr, "\n");
	}
#endif

	return written;
}

static int slot_send_tpdu(struct dvb_ci_slot * slot, struct dvb_ci_tpdu * tpdu)
{
	struct linklayer * ll = slot->opaque;
	return linklayer_send(ll, tpdu);
}

static int linklayer_recv(struct linklayer * ll)
{
	struct dvb_ci_connection * conn;
	struct dvb_ci_slot * slot;
	uint8_t buf[4096];
	int pos = 0, ret;

	ret = read(ll->fd, buf, 4096);

	if (ret < 0 && errno == EAGAIN) {
		return -EAGAIN;
	} else if (ret < 0) {
		fprintf(stderr, "READ error\n");
		return -errno;
	}

	if (ret == 0)
		return 0;

	slot = slot_by_id(ll, buf[pos++]);
	assert(slot);
	conn = dvb_ci_connection_by_id(slot, buf[pos++]);
	assert(conn);

	while (pos < ret) {
		struct dvb_ci_tpdu * tpdu = malloc(sizeof(struct dvb_ci_tpdu));
		if (tpdu == NULL)
			return -ENOMEM;

		tpdu->conn = conn;
		tpdu->tag = buf[pos++];
		tpdu->len = decode_asn_len(buf, &pos);
		tpdu->data = malloc(tpdu->len);
		memcpy(tpdu->data, &buf[pos], tpdu->len);
		pos += tpdu->len;

		dvb_ci_recv_tpdu(conn, tpdu);
	}

	assert(pos == ret);

#if 0
	if (ret > 0) {
		int i;
		fprintf(stderr, "< < < < RECV [%.2i] [%4i]", slot->num, ret);
		for (i = 0; i < ret; ++i)
			fprintf(stderr, " 0x%.2x", buf[i]);
		fprintf(stderr, "\n");
	}
#endif
	return ret;
}

static int ci_probe(struct dvb_frontend * frontend)
{
	ca_caps_t caps;
	int fd;

	// XXX - use adapter to get the device name?
	if ((fd = open("/dev/dvb/adapter0/ca0", O_RDWR | O_NONBLOCK)) < 0)
		return -errno;

	if (ioctl(fd, CA_GET_CAP, &caps) < 0) {
		close(fd);
		return -errno;
	}

	if (caps.slot_num <= 0)
		return -ENODEV;

	close(fd);
	return (caps.slot_type & CA_CI_LINK) ? 0 : -1;
}

static int ci_slot_close(struct dvb_ci_slot * slot)
{
	struct linklayer * ll = slot->opaque;

	if (--ll->refcount > 0)
		return 0;

	close(ll->fd);
	free(ll);
	return 0;
}

static int ci_slot_reset(struct dvb_ci_slot * slot)
{
	struct linklayer * ll = slot->opaque;
	struct dvb_ci_connection * conn;
	struct list_entry * pos, * next;

	for (pos = slot->connections.next; pos != NULL; pos = pos->next) {
		next = pos->next;
		conn = list_get_entry(pos, struct dvb_ci_connection, list);
		conn->close(conn);
	}

	if (ioctl(ll->fd, CA_RESET, 1 << slot->num) < 0) {
		fprintf(stderr, "[%.2i] Slot reset failed.\n", slot->num);
		return -EAGAIN;
	}

	slot->state = dvb_ci_slot_state_init;
	clock_gettime(CLOCK_MONOTONIC, &slot->lastread);

	return 0;
}

static int ci_init(struct dvb_ci * ci)
{
	struct linklayer * ll;
	int fd = -1, num;

	// XXX - use adapter to get the device name?
	if ((fd = open("/dev/dvb/adapter0/ca0", O_RDWR | O_NONBLOCK)) < 0)
		return -errno;

	ll = malloc(sizeof(struct linklayer));
	if (ll == NULL) {
		close(fd);
		return -ENOMEM;
	}

	memset(ll, 0, sizeof(struct linklayer));
	ll->fd = fd;

	if (ioctl(fd, CA_GET_CAP, &ll->caps) < 0) {
		free(ll);
		close(fd);
		return -errno;
	}

	if (ll->caps.slot_num <= 0) {
		free(ll);
		close(fd);
		return -ENODEV;
	}

	ll->nb_slots = ll->caps.slot_num;
	ll->slots = malloc(sizeof(struct dvb_ci_slot *) * ll->nb_slots);
	if (ll->slots == NULL) {
		return -ENOMEM;
	}

	for (num = 0; num < ll->nb_slots; ++num) {
		struct dvb_ci_slot * slot;

		slot = malloc(sizeof(struct dvb_ci_slot));
		if (slot == NULL)
			return -ENOMEM;

		memset(slot, 0, sizeof(struct dvb_ci_slot));

		++ll->refcount;

		slot->ci	= ci;
		slot->num	= num;
		slot->opaque	= ll;
		slot->close	= ci_slot_close;
		slot->reset	= ci_slot_reset;

		/* new */
		slot->send_tpdu	= slot_send_tpdu;

		ll->slots[num] = slot;
		dvb_ci_add_slot(ci, slot);
	}

	ll->notifier = malloc(sizeof(struct dvb_notifier));
	if (ll->notifier == NULL) {
		// XXX - free handle
		return -ENOMEM;
	}

	struct pollfd pfd = {
		fd : fd,
		events: POLLIN | POLLPRI | POLLOUT,
		revents: 0,
	};

	ll->notifier->opaque = ll;
	ll->notifier->callback = ll_notifier;

	if (dvb_add_notifier(ci->frontend->adapter->dvb, &pfd, ll->notifier)) {
		free(ll->notifier);
		return -ENOMEM; // XXX
	}

	return 0;
}

static int ci_exit(struct dvb_ci * ci)
{
	// XXX
	return -1;
}

static struct dvb_ci_slot * slot_by_id(struct linklayer * ll, int num)
{
	int i;
	for (i = 0; i < ll->nb_slots; ++i)
		if (ll->slots[i]->num == num)
			return ll->slots[i];
	return NULL;
}

static struct dvb_ci_connection * dvb_ci_connection_by_id(struct dvb_ci_slot * slot, int id)
{
	struct list_entry * pos;
	struct dvb_ci_connection * ret = NULL;

	for (pos = slot->connections.next; pos != NULL; pos = pos->next) {
		ret = list_get_entry(pos, struct dvb_ci_connection, list);
		if (ret->id == id)
			break;
	}

	return ret;
}


static int ll_process_slot(struct dvb_ci_slot * slot)
{
	struct linklayer * ll = slot->opaque;
	struct ca_slot_info info;
	unsigned int pflags = slot->flags;

	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);

	int diff = (now.tv_sec - slot->lastread.tv_sec) * 1000000 +
		   (now.tv_nsec - slot->lastread.tv_nsec) / 1000;

	int timeout = 10000000;
	if (abs(diff) > timeout) {
		fprintf(stderr, "[%.2i] CAM Timeout.\n", slot->num);
		slot->lastread = now;
		slot->reset(slot);
		return -EAGAIN;
	}

	info.num = slot->num;
	if (ioctl(ll->fd, CA_GET_SLOT_INFO, &info) < 0) {
		fprintf(stderr, "CA_GET_SLOT_INFO failed\n");
		return -EAGAIN;
	}

	slot->type = info.type;
	slot->flags = info.flags;

	if ((pflags & CA_CI_MODULE_READY) != (slot->flags & CA_CI_MODULE_READY))
	{
		if (slot->flags & CA_CI_MODULE_READY) {
			fprintf(stderr, "[%.2i] Module ready (type 0x%x flags 0x%x).\n",
				slot->num, info.type, info.flags);

			slot->state = dvb_ci_slot_state_init;
			dvb_ci_create_connection(slot, NULL);
			dvb_ci_module_inserted(slot);
		} else if (slot->state != dvb_ci_slot_state_init) {
			fprintf(stderr, "[%.2i] Module ejected.\n", slot->num);
			//dvb_ci_slot_release(slot); XXX
			dvb_ci_module_ejected(slot);
		}
	}

	if (slot->flags & CA_CI_MODULE_READY)
		return 0;

	return -EAGAIN;
}

static int ll_process_connections(struct dvb_ci_slot * slot)
{
	struct dvb_ci_connection * conn;
	struct dvb_ci_tpdu * tpdu;
	struct linklayer * ll = slot->opaque;
	struct list_entry * pos, * pos2;

	for (pos = slot->connections.next; pos != NULL; pos = pos->next) {
		conn = list_get_entry(pos, struct dvb_ci_connection, list);

		if (slot->state != dvb_ci_slot_state_idle)
			continue;

		for (pos2 = conn->outqueue.next; pos2 != NULL; pos2 = pos->next) {
			tpdu = list_get_entry(pos2, struct dvb_ci_tpdu, list);
			slot->state = dvb_ci_slot_state_reply;

			dprintf("[%.2i:%.2i:  ] >>>> tpdu: %s\n",
				slot->num, conn->id, tct_name(tpdu->tag));

			linklayer_send(ll, tpdu);
			list_remove_entry(&conn->outqueue, pos2);
			free(tpdu->data);
			free(tpdu);
			return 0;
		}
	}

	return -EAGAIN;
}

static int ll_process_slots(struct linklayer * ll)
{
	int i;

	for (i = 0; i < ll->nb_slots; ++i) {
		if (ll_process_slot(ll->slots[i]))
			continue;

		if (ll_process_connections(ll->slots[i]))
			continue;
	}

	return -EAGAIN;
}

static int ll_poll(struct linklayer * ll)
{
	struct dvb_ci_connection * conn;
	struct dvb_ci_slot * slot;
	struct dvb_ci_tpdu * tpdu;
	struct list_entry * pos; // XXX
	struct timespec now;
	int i;

	clock_gettime(CLOCK_MONOTONIC, &now);

	for (i = 0; i < ll->nb_slots; ++i) {
		slot = ll->slots[i];

		if (slot->state != dvb_ci_slot_state_idle)
			continue;

		for (pos = slot->connections.next; pos != NULL; pos = pos->next) {
			conn = list_get_entry(pos, struct dvb_ci_connection, list);

			if (conn->state != dvb_ci_tpconn_active)
				continue;

			int diff = (now.tv_sec - conn->laststatus.tv_sec) * 1000000 +
				   (now.tv_nsec - conn->laststatus.tv_nsec) / 1000;

			if (diff < 50000)
				continue;

			if (dvb_ci_create_tpdu(conn, dvb_ci_tct_receive, conn->id, &tpdu)) {
				return -ENOMEM;
			}

			slot->state = dvb_ci_slot_state_poll;

			linklayer_send(ll, tpdu);
			free(tpdu->data);
			free(tpdu);
		}
	}

	return 0;
}

static void ll_notifier(struct dvb_notifier * notifier,
			const struct pollfd * pfd)
{
	struct linklayer * ll = notifier->opaque;

	/* Maybe processing a little bit to fast? */

	if (pfd->revents & POLLIN || pfd->revents & POLLPRI) {
		if (linklayer_recv(ll) == 0)
			return;
		return;
	}

	if (pfd->revents & POLLOUT) {
		if (ll_process_slots(ll) == 0)
			return;

		ll_poll(ll);
	}
}

#if 0
/* ------------------------------------------------------------------------- */

int dvb_ci_destroy_connection(struct dvb_ci_connection * conn)
{
	struct dvb_ci_tpdu * tpdu;

	if (dvb_ci_create_tpdu(conn, dvb_ci_tct_delete, conn->id, &tpdu)) {
		free(conn);
		return -ENOMEM;
	}

	dvb_ci_send_tpdu(conn, tpdu);
	conn->state = dvb_ci_tpconn_delete;

	return 0;
}

/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */

#endif


