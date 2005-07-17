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

#ifndef _DVB_CI_INTERNAL_H_
#define _DVB_CI_INTERNAL_H_

#include <time.h>
#include <list.h>
#include <stdint.h>
#include <dvb/dvb.h>
#include <dvb/thread.h>

#define dprintf(...) //do { fprintf(stderr, __VA_ARGS__); } while (0)

#include <ci/application.h>
#include <ci/session.h>
#include <ci/transport.h>

/* generic */

const char * tct_name(enum dvb_ci_tct tag);
const char * sot_name(enum dvb_ci_sot tag);
const char * aot_name(enum dvb_ci_aot tag);

int encode_asn_len(int len, uint8_t * buf, int * ppos);
int decode_asn_len(const uint8_t * buf, int * ppos);

/* slots */

struct dvb_ci_slot {

	struct dvb_ci * ci;

	void * opaque;

	int (*close) (struct dvb_ci_slot * slot);
	int (*reset) (struct dvb_ci_slot * slot);

	int (*send_tpdu) (struct dvb_ci_slot * slot,
			  struct dvb_ci_tpdu * tpdu);

	int ncid; /* next connection id */

	struct dvb_ci_application_info * app_info;

	/* old */
	struct list_entry list;

	struct timespec lastread;

	enum dvb_ci_slot_state state;
	struct list_entry connections;

	int num;
	int type;
	unsigned int flags;
};

int dvb_ci_add_slot(struct dvb_ci * ci, struct dvb_ci_slot * slot);

/* common interface */

#define MAX_RESOURCES 25

struct dvb_ci {
	struct list_entry list;

	struct dvb_frontend * frontend;
	struct dvb_ca * ca;

	struct dvb_ci_resource * resources[MAX_RESOURCES];

	struct list_entry slots;
};

int dvb_ci_new_session(struct dvb_ci * ci, struct dvb_ci_session * session);
int dvb_ci_remove_session(struct dvb_ci * ci, struct dvb_ci_session * session);

/* resources */

struct dvb_ci_resource {
	uint32_t rid;

	void * opaque;

	int (*session_created) (struct dvb_ci_resource * resource,
				struct dvb_ci_session * session);

	int (*session_closed) (struct dvb_ci_resource * resource,
			       struct dvb_ci_session * session);

	int (*recv_apdu) (struct dvb_ci_resource * resource,
			  struct dvb_ci_session * session,
			  struct dvb_ci_apdu * apdu);
};

int dvb_ci_add_default_resources(struct dvb_ci * ci);

int dvb_ci_add_resource(struct dvb_ci * ci,
			struct dvb_ci_resource *);

int dvb_ci_remove_resource(struct dvb_ci * ci,
			   struct dvb_ci_resource *);

/* events */

int dvb_ci_module_inserted(struct dvb_ci_slot * slot);
int dvb_ci_module_ejected(struct dvb_ci_slot * slot);

/* application info */

int dvb_ci_set_application_info(struct dvb_ci_slot * slot,
				struct dvb_ci_application_info * info);
int dvb_ci_clear_application_info(struct dvb_ci_slot * slot);

#endif

