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

#ifndef _DVB_CI_APPLICATION_H_
#define _DVB_CI_APPLICATION_H_

struct dvb_ci_session;

enum dvb_ci_aot {
	dvb_ci_aot_profile_enq			= 0x9F8010,
	dvb_ci_aot_profile			= 0x9F8011,
	dvb_ci_aot_profile_change		= 0x9F8012,
	dvb_ci_aot_app_info_enq			= 0x9F8020,
	dvb_ci_aot_app_info			= 0x9F8021,
	dvb_ci_aot_enter_menu			= 0x9F8022,
	dvb_ci_aot_ca_info_enq			= 0x9F8030,
	dvb_ci_aot_ca_info			= 0x9F8031,
	dvb_ci_aot_ca_pmt			= 0x9F8032,
	dvb_ci_aot_ca_pmt_reply			= 0x9F8033,
	dvb_ci_aot_tune				= 0x9F8400,
	dvb_ci_aot_replace			= 0x9F8401,
	dvb_ci_aot_clear_replace		= 0x9F8402,
	dvb_ci_aot_ask_release			= 0x9F8403,
	dvb_ci_aot_date_time_enq		= 0x9F8440,
	dvb_ci_aot_date_time			= 0x9F8441,

	dvb_ci_aot_close_mmi			= 0x9F8800,
	dvb_ci_aot_display_control		= 0x9F8801,
	dvb_ci_aot_display_reply		= 0x9F8802,
	dvb_ci_aot_text_last			= 0x9F8803,
	dvb_ci_aot_text_more			= 0x9F8804,
	dvb_ci_aot_keypad_control		= 0x9F8805,
	dvb_ci_aot_keypress			= 0x9F8806,
	dvb_ci_aot_enq				= 0x9F8807,
	dvb_ci_aot_answ				= 0x9F8808,
	dvb_ci_aot_menu_last			= 0x9F8809,
	dvb_ci_aot_menu_more			= 0x9F880A,
	dvb_ci_aot_menu_answ			= 0x9F880B,
	dvb_ci_aot_list_last			= 0x9F880C,
	dvb_ci_aot_list_more			= 0x9F880D,
	dvb_ci_aot_subtitle_segment_last	= 0x9F880E,
	dvb_ci_aot_subtitle_segment_more	= 0x9F880F,
	dvb_ci_aot_display_message		= 0x9F8810,
	dvb_ci_aot_scene_end_mark		= 0x9F8811,
	dvb_ci_aot_scene_done			= 0x9F8812,
	dvb_ci_aot_scene_control		= 0x9F8813,
	dvb_ci_aot_subtitle_download_last	= 0x9F8814,
	dvb_ci_aot_subtitle_download_more	= 0x9F8815,
	dvb_ci_aot_flush_download		= 0x9F8816,
	dvb_ci_aot_download_reply		= 0x9F8817,

	dvb_ci_aot_comms_cmd			= 0x9F8C00,
	dvb_ci_aot_connection_descriptor	= 0x9F8C01,
	dvb_ci_aot_comms_reply			= 0x9F8C02,
	dvb_ci_aot_comms_send_last		= 0x9F8C03,
	dvb_ci_aot_comms_send_more		= 0x9F8C04,
	dvb_ci_aot_comms_rcv_last		= 0x9F8C05,
	dvb_ci_aot_comms_rcv_more		= 0x9F8C06,
};

struct dvb_ci_apdu {
	struct list_entry list;

	uint32_t tag;
	int len;
	uint8_t * data;
};

struct dvb_ci_application {
	struct dvb_ci * ci;
	struct dvb_ci_slot * slot;
	struct dvb_ci_session * session;

	void * opaque;
	int (* recv_apdu)(struct dvb_ci_application *, struct dvb_ci_apdu *);
};

int dvb_ci_open_app(struct dvb_ci_session *, struct dvb_ci_application **);
int dvb_ci_close_app(struct dvb_ci_application *);

int dvb_ci_create_apdu(uint32_t tag, int len, uint8_t * data,
		       struct dvb_ci_apdu ** ptr);

int dvb_ci_send_apdu(struct dvb_ci_session * session,
		     struct dvb_ci_apdu * apdu);

int dvb_ci_recv_apdu(struct dvb_ci_session * session,
		     struct dvb_ci_apdu * apdu);

#endif

