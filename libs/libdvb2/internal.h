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

#ifndef _DVB_INTERNAL_H_
#define _DVB_INTERNAL_H_

#define MAX_CARDS	16
#define MAX_FDS		4
#define MAX_DEVICES	4

#include <dvb/dvb.h>
#include <dvb/list.h>
#include <dvb/notifier.h>
#include <dvb/array.h>

struct dvb {
	struct list_entry list;

	struct list_entry adapters;

	struct dvb_thread_data * thread;
	struct dvb_event_data * event;

	int error;
	char errorstr[4096];

	struct list_entry programs;

	/* poll */
	int nb_notifiers;
	struct pollfd * pollfds;
	struct ptr_array notifiers;
	struct dvb_poll_callbacks poll_callbacks;
};

struct dvb_adapter {
	struct list_entry list;

	struct dvb * dvb;
	int num;

	struct list_entry frontends;
};

int dvb_error(int error, const char * fmt, ...);

#endif

