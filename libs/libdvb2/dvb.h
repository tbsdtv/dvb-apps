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

#ifndef _DVB_H_
#define _DVB_H_

typedef struct dvb dvb_t;
typedef struct dvb_adapter dvb_adapter_t;
typedef struct dvb_frontend dvb_frontend_t;

typedef struct dvb_program dvb_program_t;
typedef struct dvb_stream dvb_stream_t;

typedef struct dvb_descriptor dvb_descriptor_t;
typedef struct dvb_descriptor_list dvb_descriptor_list_t;

typedef struct dvb_event dvb_event_t;

#include <stdint.h>
#include <ci/ci.h>
#include <demux.h>

/* handle */

int dvb_create_handle(dvb_t ** handle);
int dvb_close_handle(dvb_t * dvb);

/* adapter */

int dvb_open_adapter(dvb_t * dvb, const char * device,
		     struct dvb_adapter ** handle);
int dvb_close_adapter(struct dvb_adapter *);

/* error */

const char * dvb_strerror(int error);

/* event */

enum dvb_event_type { /* name?? */
	dvb_ci_event,
	dvb_dvr_event,
	dvb_demux_event,
	dvb_frontend_event,
	dvb_thread_event
};

typedef void (*dvb_event_hook_t)(dvb_t *, dvb_event_t *, void * opaque);

enum dvb_event_type dvb_get_event_type(dvb_event_t *);

int dvb_add_event_hook(dvb_t * dvb, dvb_event_hook_t hook, void * opaque);
int dvb_remove_event_hook(dvb_t * dvb, dvb_event_hook_t hook);

/* polling */

int dvb_poll_descriptors_count(dvb_t * dvb);
int dvb_poll_descriptors(dvb_t * dvb, int ** fds, int size);
int dvb_poll_descriptors_revents(dvb_t * dvb, int ** fds, int size);

/* thread */

int dvb_enter_thread_loop(dvb_t * dvb);
int dvb_start_thread(dvb_t * dvb);
int dvb_stop_thread(dvb_t * dvb);

int dvb_trylock_thread(dvb_t * dvb);
void dvb_lock_thread(dvb_t * dvb);
void dvb_unlock_thread(dvb_t * dvb);

/* frontend */

enum dvb_frontend_event_type {
	dvb_frontend_open_event,
	dvb_frontend_close_event,
	dvb_frontend_tune_event,
	dvb_frontend_lock_event
};

struct dvb_frontend_event_data {
	dvb_frontend_t * frontend;
	enum dvb_frontend_event_type type;
};

int dvb_open_frontend(dvb_frontend_t *);
int dvb_close_frontend(dvb_frontend_t *);

int dvb_update_frontend(dvb_frontend_t *);
int dvb_frontend_locked(dvb_frontend_t *);
int dvb_frontend_readonly(dvb_frontend_t *);

int dvb_get_frontend_event_data(dvb_event_t *, struct dvb_frontend_event_data *);

#endif

