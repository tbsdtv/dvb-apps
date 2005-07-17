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

#ifndef _DVB_EVENT_H_
#define _DVB_EVENT_H_

#include <dvb/dvb.h>
#include <dvb/internal.h>
#include <stddef.h>

struct dvb_event {
	int ref; /* XXX: atomic */
	size_t size;
	enum dvb_event_type type;
};

struct dvb_event * dvb_alloc_event(enum dvb_event_type, size_t size);
void dvb_free_event(struct dvb_event *);

int dvb_enqueue_event(struct dvb *, struct dvb_event *);
int dvb_send_event(struct dvb *, struct dvb_event *);

int dvb_flush_events(struct dvb *);
int dvb_clear_events(struct dvb *);
int dvb_send_one_event(struct dvb *);

static inline void * dvb_get_event_data(struct dvb_event * event) {
	return event ? ((uint8_t*)event + sizeof(struct dvb_event)) : NULL;
}

static inline size_t dvb_get_event_data_size(struct dvb_event * event) {
	return event ? event->size - sizeof(struct dvb_event) : 0;
}

#endif

