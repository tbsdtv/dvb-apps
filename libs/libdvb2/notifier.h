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

#ifndef _DVB_NOTIFIER_H_
#define _DVB_NOTIFIER_H_

#include <sys/poll.h>

struct dvb;
struct dvb_notifier;

typedef void (*dvb_notifier_callback_t) (struct dvb_notifier *,
					 const struct pollfd *);

struct dvb_notifier {
	void * opaque;
	dvb_notifier_callback_t callback;
};

struct dvb_poll_callbacks {
	void (*add) (int fd, short events);
	void (*remove) (int fd);
};

int dvb_set_poll_callbacks(struct dvb * dvb,
			   const struct dvb_poll_callbacks *);

int dvb_revents(struct dvb * dvb, const struct pollfd *, int nfds);
int dvb_revent(struct dvb * dvb, int fd, short revents);

int dvb_add_notifier(struct dvb * dvb, const struct pollfd * pfd,
		     struct dvb_notifier * notifier);

int dvb_remove_notifier(struct dvb * dvb, struct dvb_notifier * notifier);

int dvb_poll(struct dvb * dvb, int timeout);

#endif

