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

#include <dvb/plugin.h>

#include <dvb/internal.h>
#include <dvb/frontend.h>
#include <dvb/notifier.h>
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

static int ci_slot_close(struct dvb_ci_slot * slot);
static int ci_slot_reset(struct dvb_ci_slot * slot);

static void hl_notifier(struct dvb_notifier * notifier,
			const struct pollfd * pfd);


static struct dvb_ci_plugin ci_private = {
	probe:	ci_probe,
	init:	ci_init,
};

DECLARE_PLUGIN("ci_highlevel", dvb_ci_plugin_type, &ci_private);

/* TODO:
 *  - does the highlevel interface have session management?
 *  - if so, how does kernel space tell users of new sessions?
 *  - if not, how should it be emulated?
 */

struct highlevel {
	int refcount;
	int fd;

	ca_caps_t caps;
	struct dvb_notifier * notifier;

	int nb_slots;
	struct dvb_ci_slot ** slots;
};

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
	return (caps.slot_type & CA_CI) ? 0 : -1;
}

static int ci_init(struct dvb_ci * ci)
{
	struct highlevel * hl;
	int fd = -1, num;

	// XXX - use adapter to get the device name?
	if ((fd = open("/dev/dvb/adapter0/ca0", O_RDWR | O_NONBLOCK)) < 0)
		return -errno;

	hl = malloc(sizeof(struct highlevel));
	if (hl == NULL) {
		close(fd);
		return -ENOMEM;
	}

	memset(hl, 0, sizeof(struct highlevel));
	hl->refcount = 0;
	hl->fd = fd;

	if (ioctl(fd, CA_GET_CAP, &hl->caps) < 0) {
		free(hl);
		close(fd);
		return -errno;
	}

	if (hl->caps.slot_num <= 0) {
		free(hl);
		close(fd);
		return -ENODEV;
	}

	hl->nb_slots = hl->caps.slot_num;
	hl->slots = malloc(sizeof(struct dvb_ci_slot *) * hl->nb_slots);
	if (hl->slots == NULL) {
		return -ENOMEM;
	}

	for (num = 0; num < hl->nb_slots; ++num) {
		struct dvb_ci_slot * slot;

		slot = malloc(sizeof(struct dvb_ci_slot));
		if (slot == NULL)
			return -ENOMEM;

		memset(slot, 0, sizeof(struct dvb_ci_slot));

		++hl->refcount;

		slot->ci	= ci;
		slot->num	= num;
		slot->opaque	= hl;
		slot->close	= ci_slot_close;
		slot->reset	= ci_slot_reset;

		hl->slots[num] = slot;
		dvb_ci_add_slot(ci, slot);
	}

	hl->notifier = malloc(sizeof(struct dvb_notifier));
	if (hl->notifier == NULL) {
		// XXX - free handle
		return -ENOMEM;
	}

	struct pollfd pfd = {
		fd : fd,
		events: POLLIN | POLLPRI | POLLOUT,
		revents: 0,
	};

	hl->notifier->opaque = hl;
	hl->notifier->callback = hl_notifier;

	if (dvb_add_notifier(ci->frontend->adapter->dvb, &pfd, hl->notifier)) {
		free(hl->notifier);
		return -ENOMEM; // XXX
	}

	return 0;
}

static void free_highlevel(struct highlevel * hl)
{
	if (--hl->refcount > 0)
		return;

	close(hl->fd);
	free(hl);
}

static int ci_slot_close(struct dvb_ci_slot * slot)
{
	struct highlevel * hl = slot->opaque;
	free_highlevel(hl);
	return 0;
}

static int ci_slot_reset(struct dvb_ci_slot * slot)
{
	struct highlevel * hl = slot->opaque;

	if (ioctl(hl->fd, CA_RESET, 1 << slot->num) < 0) {
		fprintf(stderr, "[%.2i] Slot reset failed.\n", slot->num);
		return -EAGAIN;
	}

	slot->state = dvb_ci_slot_state_init;
	clock_gettime(CLOCK_MONOTONIC, &slot->lastread);

	return 0;
}

static void hl_notifier(struct dvb_notifier * notifier,
			const struct pollfd * pfd)
{
	// XXX - read any pending messages
}

