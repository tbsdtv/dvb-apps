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

#ifndef _SCAN_H_
#define _SCAN_H_

#include <dvb/list.h>

/* The DVB hierarchy:
 *
 * <adapter>
 *   <frontend>
 *     <source>
 *       <multiplex>
 *         <service>
 *           <component />
 *           <component />
 *           ...
 *         </service>
 *         <service />
 *         ....
 *       </multiplex>
 *       <multiplex />
 *       ...
 *     </source>
 *     <source />
 *     ...
 *   </frontend>
 *   <frontend />
 *   ...
 * </adapter>
 */

struct dvb_component {
	struct list_entry head;

	int stream_type;
	int pid;
};

struct dvb_service {
	struct list_entry head;

	int service_id;
	struct list_entry components;
};

struct dvb_multiplex {
	struct list_entry head;

	struct list_entry services;
};

struct dvb_source {
	struct list_entry head;

	struct list_entry multiplexes;
};

struct dvb_scan_service {
	struct dvb_service * service;
};

struct dvb_scan_multiplex {
	struct list_entry head;

	struct dvb_multiplex * multiplex;

	/* service queue */
	struct list_entry queue;
	struct list_entry failed_list;
};

struct dvb_scan {

	/* multplex queue */
	struct dvb_scan_multiplex * current;

	struct list_entry queue;
	struct list_entry failed_list;
};

int dvb_create_scan(struct dvb_scan **);
int dvb_free_scan(struct dvb_scan *);

int dvb_enqueue_multiplex(struct dvb_multiplex *);
int dvb_enqueue_service(struct dvb_service *);

#endif

