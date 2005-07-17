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

#include <ci/ci.h>
#include <ci/internal.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

int dvb_ci_create_apdu(uint32_t tag, int len, uint8_t * data,
		       struct dvb_ci_apdu ** ptr)
{
	struct dvb_ci_apdu * apdu;

	apdu = malloc(sizeof(struct dvb_ci_apdu));
	if (apdu == NULL)
		return -ENOMEM;

	apdu->tag = tag;
	apdu->len = len;
	apdu->data = malloc(apdu->len);
	if (apdu->data == NULL) {
		free(apdu);
		return -ENOMEM;
	}

	memcpy(apdu->data, data, apdu->len);
	*ptr = apdu;
	return 0;
}

static int handle_apdu_profile_enq(struct dvb_ci_session * session,
				   struct dvb_ci_apdu * apdu)
{
	uint8_t buf[1024];
	int pos = 0, i;
	uint32_t types [] = { /* XXX - use ci->resources */
		dvb_ci_rid_resource_manager,
		dvb_ci_rid_application_information,
		dvb_ci_rid_conditional_access_support,
		dvb_ci_rid_host_control,
		dvb_ci_rid_date_time,
		dvb_ci_rid_mmi,
	};

	for (i = 0; i < sizeof(types) / sizeof(uint32_t); ++i) {
		uint32_t type = types[i];
		buf[pos++] = (type >> 24) & 0xff;
		buf[pos++] = (type >> 16) & 0xff;
		buf[pos++] = (type >> 8) & 0xff;
		buf[pos++] = (type >> 0) & 0xff;
	}

	if (dvb_ci_create_apdu(dvb_ci_aot_profile, pos, buf, &apdu)) {
		return -ENOMEM;
	}

	dvb_ci_send_apdu(session, apdu);
	return 0;
}

static int handle_apdu_app_info(struct dvb_ci_session * session,
				struct dvb_ci_apdu * apdu)
{
	struct dvb_ci_application_info * info;
	uint8_t * buf = apdu->data;
	int pos = 0, len;

	info = malloc(sizeof(struct dvb_ci_application_info));
	if (info == NULL)
		return -ENOMEM;

	info->type = buf[pos];
	info->man  = buf[++pos] << 8;
	info->man |=  buf[++pos];
	info->code  = buf[++pos] << 8;
	info->code |= buf[++pos];

	/* XXX - do a proper dvb string to ascii/utf8 */
	len = buf[++pos];
	info->name = malloc(len + 1);
	memcpy(info->name, buf + pos + 1, len);
	info->name[len] = 0;

	dvb_ci_set_application_info(session->conn->slot, info);
	return 0;
}

int dvb_ci_open_app(struct dvb_ci_session * session,
		    struct dvb_ci_application ** ptr)
{
	struct dvb_ci_application * app;

	if (session->app != NULL)
		return -EBUSY;

	app = malloc(sizeof(struct dvb_ci_application));
	if (app == NULL)
		return -ENOMEM;

	memset(app, 0, sizeof(struct dvb_ci_application));
	app->ci = session->ci;
	app->slot = session->slot;
	app->session = session;
	session->app = app;

	*ptr = app;
	return 0;
}

int dvb_ci_close_app(struct dvb_ci_application * app)
{
	app->session->app = NULL;
	free(app);
	return 0;
}


int dvb_ci_recv_apdu(struct dvb_ci_session * session,
		     struct dvb_ci_apdu * apdu)
{
	struct dvb_ci * ci = session->ci;
	int i;

	if (session->app) {
		session->app->recv_apdu(session->app, apdu);
		return 0;
	}

	for (i = 0; i < MAX_RESOURCES; ++i) {
		if (ci->resources[i] == NULL)
			continue;

		if (ci->resources[i]->rid != session->resid)
			continue;

		ci->resources[i]->recv_apdu(ci->resources[i], session, apdu);
	}

	switch (apdu->tag) {
		case dvb_ci_aot_profile:
			break;

		case dvb_ci_aot_profile_enq:
			handle_apdu_profile_enq(session, apdu);
			break;

		case dvb_ci_aot_app_info:
			handle_apdu_app_info(session, apdu);

		case dvb_ci_aot_profile_change:
			break;
	}

	return 0;
}

int dvb_ci_send_apdu(struct dvb_ci_session * session,
		     struct dvb_ci_apdu * apdu)
{
	struct dvb_ci_spdu * spdu;
	uint8_t buf[2];
	buf[0] = session->id >> 8;
	buf[1] = session->id;

	if (dvb_ci_create_spdu(dvb_ci_sot_number, 2, buf, &spdu))
		return -ENOMEM;

	list_append_entry(&spdu->apdu, &apdu->list);

	return dvb_ci_send_spdu(session, spdu);
}

