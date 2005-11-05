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

const char * tct_name(enum dvb_ci_tct tag)
{
	switch (tag) {
	case dvb_ci_tct_status:		return "dvb_ci_tct_status";
	case dvb_ci_tct_receive:	return "dvb_ci_tct_receive";
	case dvb_ci_tct_create:		return "dvb_ci_tct_create";
	case dvb_ci_tct_create_reply:	return "dvb_ci_tct_create_reply";
	case dvb_ci_tct_delete:		return "dvb_ci_tct_delete";
	case dvb_ci_tct_delete_reply:	return "dvb_ci_tct_delete_reply";
	case dvb_ci_tct_request:	return "dvb_ci_tct_request";
	case dvb_ci_tct_new:		return "dvb_ci_tct_new";
	case dvb_ci_tct_error:		return "dvb_ci_tct_error";
	case dvb_ci_tct_data_more:	return "dvb_ci_tct_data_more";
	case dvb_ci_tct_data_last:	return "dvb_ci_tct_data_last";
	}

	return NULL;
}

const char * sot_name(enum dvb_ci_sot tag)
{
	switch (tag) {
	case dvb_ci_sot_open_request:	return "dvb_ci_sot_open_request";
	case dvb_ci_sot_open_response:	return "dvb_ci_sot_open_response";
	case dvb_ci_sot_create:		return "dvb_ci_sot_create";
	case dvb_ci_sot_create_response:return "dvb_ci_sot_create_response";
	case dvb_ci_sot_close_request:	return "dvb_ci_sot_close_request";
	case dvb_ci_sot_close_response:	return "dvb_ci_sot_close_response";
	case dvb_ci_sot_number:		return "dvb_ci_sot_number";
	}

	return NULL;
}

const char * aot_name(enum dvb_ci_aot tag)
{
	switch (tag) {
	case dvb_ci_aot_profile_enq:		return "dvb_ci_aot_profile_enq";
	case dvb_ci_aot_profile:		return "dvb_ci_aot_profile";
	case dvb_ci_aot_profile_change:		return "dvb_ci_aot_profile_change";
	case dvb_ci_aot_app_info_enq:		return "dvb_ci_aot_app_info_enq";
	case dvb_ci_aot_app_info:		return "dvb_ci_aot_app_info";
	case dvb_ci_aot_enter_menu:		return "dvb_ci_aot_enter_menu";
	case dvb_ci_aot_ca_info_enq:		return "dvb_ci_aot_ca_info_enq";
	case dvb_ci_aot_ca_info:		return "dvb_ci_aot_ca_info";
	case dvb_ci_aot_ca_pmt:			return "dvb_ci_aot_ca_pmt";
	case dvb_ci_aot_ca_pmt_reply:		return "dvb_ci_aot_ca_pmt_reply";
	case dvb_ci_aot_tune:			return "dvb_ci_aot_tune";
	case dvb_ci_aot_replace:		return "dvb_ci_aot_replace";
	case dvb_ci_aot_clear_replace:		return "dvb_ci_aot_clear_replace";
	case dvb_ci_aot_ask_release:		return "dvb_ci_aot_ask_release";
	case dvb_ci_aot_date_time_enq:		return "dvb_ci_aot_date_time_enq";
	case dvb_ci_aot_date_time:		return "dvb_ci_aot_date_time";

	case dvb_ci_aot_close_mmi:		return "dvb_ci_aot_close_mmi";
	case dvb_ci_aot_display_control:	return "dvb_ci_aot_display_control";
	case dvb_ci_aot_display_reply:		return "dvb_ci_aot_display_reply";
	case dvb_ci_aot_text_last:		return "dvb_ci_aot_text_last";
	case dvb_ci_aot_text_more:		return "dvb_ci_aot_text_more";
	case dvb_ci_aot_keypad_control:		return "dvb_ci_aot_keypad_control";
	case dvb_ci_aot_keypress:		return "dvb_ci_aot_keypress";
	case dvb_ci_aot_enq:			return "dvb_ci_aot_enq";
	case dvb_ci_aot_answ:			return "dvb_ci_aot_answ";
	case dvb_ci_aot_menu_last:		return "dvb_ci_aot_menu_last";
	case dvb_ci_aot_menu_more:		return "dvb_ci_aot_menu_more";
	case dvb_ci_aot_menu_answ:		return "dvb_ci_aot_menu_answ";
	case dvb_ci_aot_list_last:		return "dvb_ci_aot_list_last";
	case dvb_ci_aot_list_more:		return "dvb_ci_aot_list_more";
	case dvb_ci_aot_subtitle_segment_last:	return "dvb_ci_aot_subtitle_segment_last";
	case dvb_ci_aot_subtitle_segment_more:	return "dvb_ci_aot_subtitle_segment_more";
	case dvb_ci_aot_display_message:	return "dvb_ci_aot_display_message";
	case dvb_ci_aot_scene_end_mark:		return "dvb_ci_aot_scene_end_mark";
	case dvb_ci_aot_scene_done:		return "dvb_ci_aot_scene_done";
	case dvb_ci_aot_scene_control:		return "dvb_ci_aot_scene_control";
	case dvb_ci_aot_subtitle_download_last:	return "dvb_ci_aot_subtitle_download_last";
	case dvb_ci_aot_subtitle_download_more:	return "dvb_ci_aot_subtitle_download_more";
	case dvb_ci_aot_flush_download:		return "dvb_ci_aot_flush_download";
	case dvb_ci_aot_download_reply:		return "dvb_ci_aot_download_reply";

	case dvb_ci_aot_comms_cmd:		return "dvb_ci_aot_comms_cmd";
	case dvb_ci_aot_connection_descriptor:	return "dvb_ci_aot_connection_descriptor";
	case dvb_ci_aot_comms_reply:		return "dvb_ci_aot_comms_reply";
	case dvb_ci_aot_comms_send_last:	return "dvb_ci_aot_comms_send_last";
	case dvb_ci_aot_comms_send_more:	return "dvb_ci_aot_comms_send_more";
	case dvb_ci_aot_comms_rcv_last:		return "dvb_ci_aot_comms_rcv_last";
	case dvb_ci_aot_comms_rcv_more:		return "dvb_ci_aot_comms_rcv_more";
	}

	return NULL;
}

int dvb_ci_add_default_resources(struct dvb_ci * ci)
{
#if 0
	dvb_open_ca(ci, &ci->ca);
	dvb_open_host_control(ci, &ci->host_control);
	dvb_open_date_time(ci, &ci->date_time);
	dvb_open_mmi(ci, &ci->mmi);
#endif

	return 0;
}

const struct dvb_ci_application_info *
dvb_ci_get_application_info(const struct dvb_ci_slot * slot)
{
	/* XXX - slot state */
	if (slot == NULL)
		return NULL;

	return slot->app_info;
}

int dvb_ci_set_application_info(struct dvb_ci_slot * slot,
				struct dvb_ci_application_info * info)
{
	struct dvb_event * event;
	struct dvb_ci_event_data * event_data;

	if (slot->app_info != NULL) {
		free(slot->app_info->name);
		free(slot->app_info);
	}

	slot->app_info = info;

	event = dvb_alloc_event(dvb_ci_event,
				sizeof(struct dvb_ci_event_data));

	if (event == NULL)
		return -ENOMEM;

	event_data = dvb_get_event_data(event);
	event_data->slot = slot;
	event_data->type = dvb_ci_application_info_event;

	dvb_send_event(slot->ci->frontend->adapter->dvb, event);
	return 0;
}

int dvb_ci_clear_application_info(struct dvb_ci_slot * slot)
{
	if (slot->app_info) {
		free(slot->app_info->name);
		free(slot->app_info);
	}

	slot->app_info = NULL;
	return 0;
}

int dvb_open_ci(struct dvb_frontend * frontend, struct dvb_ci ** handle)
{
	struct dvb_ci * ci;
	int ret;
	const struct dvb_plugin * plugins, * pos;

	if (frontend == NULL)
		return -EINVAL;

	ci = malloc(sizeof(struct dvb_ci));
	if (ci == NULL)
		return -ENOMEM;

	memset(ci, 0, sizeof(struct dvb_ci));

	LIST_ENTRY_INIT(&ci->slots);
	ci->frontend = frontend;

	if ((ret = dvb_find_plugins(frontend->adapter->dvb,
				    dvb_ci_plugin_type, &plugins)))
		return ret;

	for (pos = plugins; pos != NULL; pos = pos->next) {
		struct dvb_ci_plugin * ci_plugin = pos->priv;

		if (pos->type != dvb_ci_plugin_type)
			continue;

		if (ci_plugin->probe(frontend))
			continue;

		ci_plugin->init(ci);
	}

	if (handle != NULL)
		*handle = ci;

	list_append_entry(&frontend->ci, &ci->list);

	dvb_ci_add_default_resources(ci); /* XXX */
	return 0;
}

int dvb_close_ci(struct dvb_ci * ci)
{
	struct list_entry * pos, * next;
	struct dvb_ci_slot * slot;

	for (pos = ci->slots.next; pos != NULL; pos = next) {
		next = pos->next;
		slot = list_get_entry(pos, struct dvb_ci_slot, list);

		list_remove_entry(&ci->slots, pos);
		free(slot);
	}

	free(ci);
	return 0;
}

int dvb_get_ci_event_data(struct dvb_event * event,
			  struct dvb_ci_event_data * data)
{
	if (event == NULL || data == NULL)
		return -EINVAL;

	if (dvb_get_event_type(event)      != dvb_ci_event ||
	    dvb_get_event_data_size(event) != sizeof(struct dvb_ci_event_data))
		return -EINVAL;

	memcpy(data, dvb_get_event_data(event),
	       sizeof(struct dvb_ci_event_data));

	return 0;
}

#if 0
/* -------------------------------------------------------------------------- */

static int dvb_ci_session_release(struct dvb_ci_session * session)
{
	struct dvb_ci_spdu * spdu;
	struct list_entry * pos, * next;

	for (pos = session->outqueue.next; pos != NULL; pos = next) {
		next = pos->next;
		spdu = list_get_entry(pos, struct dvb_ci_spdu, list);

		list_remove_entry(&session->outqueue, pos);
		free(spdu); /* XXX */
	}

	for (pos = session->inqueue.next; pos != NULL; pos = next) {
		next = pos->next;
		spdu = list_get_entry(pos, struct dvb_ci_spdu, list);

		list_remove_entry(&session->inqueue, pos);
		free(spdu); /* XXX */
	}

	return 0;
}

static int dvb_ci_connection_release(struct dvb_ci_connection * conn)
{
	struct dvb_ci_session * session;
	struct dvb_ci_tpdu * tpdu;
	struct list_entry * pos, * next;

	for (pos = conn->sessions.next; pos != NULL; pos = next) {
		next = pos->next;
		session = list_get_entry(pos, struct dvb_ci_session, list);

		dvb_ci_session_release(session);
		list_remove_entry(&conn->sessions, pos);
		free(session);
	}

	for (pos = conn->outqueue.next; pos != NULL; pos = next) {
		next = pos->next;
		tpdu = list_get_entry(pos, struct dvb_ci_tpdu, list);

		list_remove_entry(&conn->outqueue, pos);
		free(tpdu->data);
		free(tpdu);
	}

	for (pos = conn->inqueue.next; pos != NULL; pos = next) {
		next = pos->next;
		tpdu = list_get_entry(pos, struct dvb_ci_tpdu, list);

		list_remove_entry(&conn->inqueue, pos);
		free(tpdu->data);
		free(tpdu);
	}

	return 0;
}

static int dvb_ci_slot_release(struct dvb_ci_slot * slot)
{
	struct dvb_ci_connection * conn;
	struct list_entry * pos, * next;

	slot->reset(slot);
	slot->state = dvb_ci_slot_state_released;

	for (pos = slot->connections.next; pos != NULL; pos = next) {
		next = pos->next;
		conn = list_get_entry(pos, struct dvb_ci_connection, list);

		dvb_ci_connection_release(conn);
		list_remove_entry(&slot->connections, pos);
		free(conn);
	}

	return 0;
}

#endif

