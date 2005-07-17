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

#ifndef _DVB_DEMUX_H_
#define _DVB_DEMUX_H_

#include <dvb/dvb.h>
#include <stdint.h>

typedef struct dvb_demux dvb_demux_t;
typedef struct dvb_filter dvb_filter_t;
typedef struct dvb_filter_entry dvb_filter_entry_t;

/*
 * dvb_open_filter
 * dvb_create_filter
 *
 * dvb_open_section_filter(demux, &section_filter);
 * dvb_close_section_filter(section_filter)
 *
 * dvb_section_filter_add(section_filter, pid, table_id, repeat, &subfilter);
 * dvb_section_filter_del(subfilter);
 */

enum dvb_demux_event_type {
	dvb_demux_open_event,
	dvb_demux_close_event,
	dvb_demux_data_event
};

/**
 * @dvb_section_filter		- passes data unparsed from demux
 * @dvb_pes_filter		- passes extracted pes packets from ts
 * @dvb_ts_filter		- passes raw ts packets
 */
enum dvb_filter_type {
	dvb_ts_filter,
	dvb_pes_filter,
	dvb_section_filter
};

struct dvb_demux_event {
	dvb_demux_t * demux;
	enum dvb_demux_event_type type;

	dvb_filter_t * filter;

	/* XXX - check this */
	int pid;
	int len;
	uint8_t * data;
};

/* demux device */

int dvb_open_demux(dvb_frontend_t * frontend, dvb_demux_t ** handle);
int dvb_close_demux(dvb_demux_t *);

/* events */

int dvb_get_demux_event_data(dvb_event_t *, struct dvb_demux_event *);

/* generic filter management */

int dvb_open_filter(dvb_demux_t * demux, enum dvb_filter_type type,
		    dvb_filter_t ** filter);

int dvb_close_filter(dvb_filter_t *);

int dvb_pause_filter(dvb_filter_t * filter);
int dvb_resume_filter(dvb_filter_t * filter);

/* filter entry management */

int dvb_pid_filter_add(dvb_filter_t *, int pid,
		       dvb_filter_entry_t ** entry);

int dvb_section_filter_add(dvb_filter_t *, int pid, uint8_t table_id,
			   dvb_filter_entry_t ** entry);

int dvb_filter_remove(dvb_filter_entry_t *);


/* XXX - are these needed? */
int dvb_set_filter_pid(dvb_filter_t *, int pid);
int dvb_read_filter(struct dvb_filter * filter);
int dvb_add_section_pid(struct dvb_filter * filter, int pid);

#if 0

/*
 * Notes:
 *  - PAT/PMT is so common to grab that they should be
 *    handled internally.
 *
 *  - dvb_program_status_event:
 *		- running (currently transmitting)
 *		- ca (ability to decrypt)
 *		- fe (signal)
 *		- demux (availability)
 *
 *  - An idle-time channel scanner would be neat,
 *    but it should be considered non-trivial.
 *
 *  - Automatic conditional access control should be optional.
 *
 *  - 
 */

int dvb_filter_remove_entry(dvb_filter_entry_t *);

#endif

#endif

