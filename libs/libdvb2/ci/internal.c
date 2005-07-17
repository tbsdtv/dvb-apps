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

#include <dvb/dvb.h>
#include <dvb/frontend.h>
#include <dvb/notifier.h>
#include <dvb/event.h>
#include <ci/ci.h>
#include <ci/internal.h>
#include <ci/ca.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <linux/dvb/ca.h>
#include <dvb/plugin.h>

int encode_asn_len(int len, uint8_t * buf, int * ppos)
{
	int pos = *ppos;

	if (len >= 0x80) {
		int size = 1;
		while (len >> (size * 8))
			++size;

		buf[pos++] = size | 0x80;
		while (size--)
			buf[pos++] = (len >> (size * 8)) & 0xFF;
	} else {
		buf[pos++] = len;
	}

	*ppos = pos;
	return len;
}

int decode_asn_len(const uint8_t * buf, int * ppos)
{
	int pos = *ppos;
	int len = buf[pos++];

	if (len & 0x80) {
		int size = len & 0x7F;
		len = 0;
		while (size--)
			len |= buf[pos++] << (size * 8);
	}

	*ppos = pos;
	return len;
}

int dvb_ci_add_resource(struct dvb_ci * ci,
			struct dvb_ci_resource * resource)
{
	int i;

	for (i = 0; i < MAX_RESOURCES; ++i) {
		if (ci->resources[i] != NULL)
			continue;

		ci->resources[i] = resource;
		return 0;
	}

	/* XXX - profile changed */
	return -EOVERFLOW;
}

int dvb_ci_remove_resource(struct dvb_ci * ci,
			   struct dvb_ci_resource * resource)
{
	int i;

	for (i = 0; i < MAX_RESOURCES; ++i) {
		if (ci->resources[i] != resource)
			continue;

		ci->resources[i] = NULL;
		return 0;
	}

	/* XXX - profile changed */
	return -EINVAL;
}

int dvb_ci_new_session(struct dvb_ci * ci, struct dvb_ci_session * session)
{
	struct dvb_event * event;
	struct dvb_ci_event_data * event_data;

	struct dvb_ci_apdu * apdu;
	int i;

	for (i = 0; i < MAX_RESOURCES; ++i) {
		if (ci->resources[i] == NULL)
			continue;

		if (ci->resources[i]->rid != session->resid)
			continue;

		ci->resources[i]->session_created(ci->resources[i], session);
	}

	switch (session->resid) {
		case dvb_ci_rid_resource_manager:
			if (dvb_ci_create_apdu(dvb_ci_aot_profile_change, 0, NULL, &apdu))
				return -ENOMEM;
			dvb_ci_send_apdu(session, apdu);
			break;

		case dvb_ci_rid_application_information:
			if (dvb_ci_create_apdu(dvb_ci_aot_app_info_enq, 0, NULL, &apdu))
				return -ENOMEM;
			dvb_ci_send_apdu(session, apdu);
			break;

#if 0
		case dvb_ci_rid_host_control:
		case dvb_ci_rid_date_time:
		case dvb_ci_rid_mmi:
		default:
			fprintf(stderr, "resource: %x\n", session->resid);
#endif
	}

	event = dvb_alloc_event(dvb_ci_event,
				sizeof(struct dvb_ci_event_data));

	if (event == NULL)
		return -ENOMEM;

	event_data = dvb_get_event_data(event);
	event_data->type = dvb_ci_session_create_event;
	event_data->slot = session->conn->slot;
	event_data->session = session;

	dvb_enqueue_event(ci->frontend->adapter->dvb, event);
	return 0;
}

int dvb_ci_remove_session(struct dvb_ci * ci, struct dvb_ci_session * session)
{
	struct dvb_event * event;
	struct dvb_ci_event_data * event_data;

	event = dvb_alloc_event(dvb_ci_event,
				sizeof(struct dvb_ci_event_data));

	if (event == NULL)
		return -ENOMEM;

	event_data = dvb_get_event_data(event);
	event_data->type = dvb_ci_session_close_event;
	event_data->slot = session->conn->slot;
	event_data->session = session;

	dvb_send_event(ci->frontend->adapter->dvb, event);
	return 0;
}

int dvb_ci_add_slot(struct dvb_ci * ci, struct dvb_ci_slot * slot)
{
	LIST_ENTRY_INIT(&slot->list);
	LIST_ENTRY_INIT(&slot->connections);

	list_append_entry(&ci->slots, &slot->list);

	slot->reset(slot);
	return 0;
}

int dvb_ci_module_inserted(struct dvb_ci_slot * slot)
{
	struct dvb_event * event;
	struct dvb_ci_event_data * event_data;

	event = dvb_alloc_event(dvb_ci_event,
				sizeof(struct dvb_ci_event_data));

	if (event == NULL)
		return -ENOMEM;

	event_data = dvb_get_event_data(event);
	event_data->slot = slot;
	event_data->type = dvb_ci_module_insert_event;

	dvb_enqueue_event(slot->ci->frontend->adapter->dvb, event);
	return 0;
}

int dvb_ci_module_ejected(struct dvb_ci_slot * slot)
{
	struct dvb_event * event;
	struct dvb_ci_event_data * event_data;

	dvb_ci_clear_application_info(slot);

	event = dvb_alloc_event(dvb_ci_event,
				sizeof(struct dvb_ci_event_data));

	if (event == NULL)
		return -ENOMEM;

	event_data = dvb_get_event_data(event);
	event_data->slot = slot;
	event_data->type = dvb_ci_module_eject_event;

	dvb_send_event(slot->ci->frontend->adapter->dvb, event);
	return 0;
}


