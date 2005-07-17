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

#ifndef _DVB_CI_H_
#define _DVB_CI_H_

#include <dvb/dvb.h>
#include <stdint.h>

typedef struct dvb_ci dvb_ci_t;

enum dvb_ci_event_type {
	dvb_ci_module_insert_event,
	dvb_ci_module_eject_event,
	dvb_ci_application_info_event,
	dvb_ci_session_create_event,
	dvb_ci_session_close_event
};

enum dvb_ci_slot_state {
	dvb_ci_slot_state_released,
	dvb_ci_slot_state_init,
	dvb_ci_slot_state_idle,
	dvb_ci_slot_state_reply,
	dvb_ci_slot_state_poll
};

enum dvb_ci_resource_id {
	dvb_ci_rid_resource_manager		= 0x00010041,
	dvb_ci_rid_application_information	= 0x00020041,
	dvb_ci_rid_conditional_access_support	= 0x00030041,
	dvb_ci_rid_host_control			= 0x00200041,
	dvb_ci_rid_date_time			= 0x00240041,
	dvb_ci_rid_mmi				= 0x00400041
};

struct dvb_ci_event_data {
	enum dvb_ci_event_type type;

	const struct dvb_ci_slot * slot;
	struct dvb_ci_session * session;
};

struct dvb_ci_application_info {
	uint8_t type;
	uint16_t man;
	uint16_t code;
	char * name;
};

int dvb_open_ci(dvb_frontend_t * frontend, dvb_ci_t ** handle);
int dvb_close_ci(dvb_ci_t * handle);

int dvb_get_ci_event_data(dvb_event_t *, struct dvb_ci_event_data *);

const struct dvb_ci_application_info *
dvb_ci_get_application_info(const struct dvb_ci_slot *);

uint32_t dvb_ci_get_session_resource_id(const struct dvb_ci_session *);

#endif

