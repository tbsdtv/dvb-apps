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
#include <ci/ca.h>
#include <ci/internal.h>
#include <ci/session.h>

#include <si/descriptor.h>
#include <si/mpeg/descriptor.h>
#include <si/mpeg/pmt_section.h>

#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <stdio.h>

enum dbg {
	DBG_CA_PMT		= 0x01,
	DBG_CA_PMT_REPLY	= 0x02,
	DBG_CA_PMT_DUMP		= 0x04,
	DBG_CA_PMT_REPLY_DUMP	= 0x08,
	DBG_CA_OTHER		= 0x10,
};

int dbg_mask = 0;//DBG_CA_PMT_REPLY | DBG_CA_PMT | DBG_CA_PMT_REPLY_DUMP;

#undef dprintf
#define dprintf(mask, fmt, args...) \
	if (dbg_mask & mask) fprintf(stderr, fmt, ##args)

enum ca_pmt_list_mgmt {
	ca_pmt_more	= 0x00,
	ca_pmt_first	= 0x01,
	ca_pmt_last	= 0x02,
	ca_pmt_only	= 0x03,
	ca_pmt_add	= 0x04,
	ca_pmt_update	= 0x05,
};

enum ca_pmt_cmd {
	ca_pmt_cmd_ok_descrambling	= 0x01,
	ca_pmt_cmd_ok_mmi		= 0x02,
	ca_pmt_cmd_query		= 0x03,
	ca_pmt_cmd_not_selected		= 0x04,
};

struct dvb_ca {
	struct list_entry sessions;
	struct list_entry programs;
};

struct dvb_ca_session {
	struct list_entry list;

	int nb_ca_system_ids;
	uint16_t * ca_system_ids;

	struct dvb_ca * ca;
	struct dvb_ci_session * session;
	struct dvb_ci_application * app;
};

struct dvb_ca_program {
	struct list_entry list;

	struct dvb_ca * ca;
	int program_number;

	int nb_pmt_sections;
	struct mpeg_pmt_section ** pmts;

	enum ca_pmt_list_mgmt list_mgmt;
	enum ca_pmt_cmd cmd;

	struct list_entry streams;
};

struct dvb_ca_stream {
	struct list_entry list;

	int pid;
	enum ca_pmt_cmd cmd;
};

static int dvb_ca_send_pmt(struct dvb_ca * ca);

static inline struct dvb_ca_session *
	dvb_ca_find_ca_session(struct dvb_ca * ca,
			       struct dvb_ci_session * session);

static inline int free_ca_session(struct dvb_ca_session * ca_session);

static inline struct dvb_ca_program *
	dvb_ca_find_program(struct dvb_ca * ca, int program_number);

static inline struct dvb_ca_stream *
	dvb_ca_find_stream(struct dvb_ca_program * prg, int pid);

static int dvb_ca_recv_apdu(struct dvb_ci_application * app,
			    struct dvb_ci_apdu * apdu);

int dvb_create_ca(struct dvb_ca ** ptr)
{
	struct dvb_ca * ca;

	ca = malloc(sizeof(struct dvb_ca));
	if (ca == NULL)
		return -ENOMEM;

	memset(ca, 0, sizeof(struct dvb_ca));

	if (ptr)
		*ptr = ca;

	return 0;
}

int dvb_close_ca(struct dvb_ca * ca)
{
	/* FIXME: free all sessions */
	free(ca);
	return 0;
}

int dvb_ca_add_session(struct dvb_ca * ca, struct dvb_ci_session * session)
{
	struct dvb_ca_session * ca_session;
	struct dvb_ci_apdu * apdu;

	if (ca == NULL)
		return -EINVAL;

	if (session->resid != dvb_ci_rid_conditional_access_support)
		return -EINVAL;

	ca_session = malloc(sizeof(struct dvb_ca_session));
	if (ca_session == NULL)
		return -ENOMEM;

	memset(ca_session, 0, sizeof(struct dvb_ca_session));
	ca_session->ca = ca;
	ca_session->session = session;

	if (dvb_ci_open_app(session, &ca_session->app)) {
		free(ca_session);
		return -EBUSY;
	}

	ca_session->app->opaque = ca_session;
	ca_session->app->recv_apdu = dvb_ca_recv_apdu;

	if (dvb_ci_create_apdu(dvb_ci_aot_ca_info_enq, 0, NULL, &apdu)) {
		/* FIXME: dvb_ci_close_app(ca_session->app); */
		free(ca_session);
		return -ENOMEM;
	}

	dvb_ci_send_apdu(session, apdu);
	list_append_entry(&ca->sessions, &ca_session->list);

	return 0;
}

int dvb_ca_remove_session(struct dvb_ca * ca, struct dvb_ci_session * session)
{
	struct dvb_ca_session * ca_session;

	ca_session = dvb_ca_find_ca_session(ca, session);
	if (ca_session == NULL)
		return -EINVAL;

	list_remove_entry(&ca->sessions, &ca_session->list);
	dvb_ci_close_app(ca_session->app);
	free(ca_session);
	return 0;
}

int dvb_ca_create_program(struct dvb_ca * ca, struct dvb_ca_program ** ptr)
{
	struct dvb_ca_program * program;

	program = malloc(sizeof(struct dvb_ca_program));
	if (program == NULL)
		return -ENOMEM;

	memset(program, 0, sizeof(struct dvb_ca_program));
	LIST_ENTRY_INIT(&program->streams);
	program->ca = ca;
	program->list_mgmt = ca_pmt_more;
	program->cmd = ca_pmt_cmd_query;
	list_append_entry(&ca->programs, &program->list);

	*ptr = program;
	return 0;
}

int dvb_ca_select_stream(struct dvb_ca_program * prg, int pid)
{
	struct dvb_ca_stream * stream;
	struct list_entry * pos;

	if (prg == NULL)
		return -EINVAL;

	for (pos = prg->streams.next; pos; pos = pos->next) {
		stream = list_get_entry(pos, struct dvb_ca_stream, list);
		if (stream->pid == pid)
			return 0;
	}

	stream = malloc(sizeof(struct dvb_ca_stream));
	if (stream == NULL)
		return -ENOMEM;

	stream->pid = pid;
	stream->cmd = ca_pmt_cmd_query;

	list_append_entry(&prg->streams, &stream->list);
	return 0;
}

int dvb_ca_deselect_stream(struct dvb_ca_program * prg, int pid)
{
	struct dvb_ca_stream * stream;
	struct list_entry * pos;

	if (prg == NULL)
		return -EINVAL;

	for (pos = prg->streams.next; pos; pos = pos->next) {
		stream = list_get_entry(pos, struct dvb_ca_stream, list);
		if (stream->pid == pid) {
			list_remove_entry(&prg->streams, &stream->list);
			free(stream);
			return 0;
		}
	}

	return 0;
}

int dvb_ca_stream_is_selected(struct dvb_ca_program * prg, int pid)
{
	return dvb_ca_find_stream(prg, pid) != NULL;
}

int dvb_ca_update_section(struct dvb_ca_program * prg,
			  const struct mpeg_pmt_section * pmt)
{
	struct mpeg_pmt_section * pmt_copy;

	if (prg->nb_pmt_sections != pmt->head.last_section_number + 1) {
		int i;

		for (i = 0; i < prg->nb_pmt_sections; ++i)
			free(prg->pmts[i]);
		free(prg->pmts);

		prg->nb_pmt_sections = pmt->head.last_section_number + 1;
		prg->pmts = malloc(prg->nb_pmt_sections * sizeof(void *));
		if (prg->pmts == NULL)
			return -ENOMEM;

		memset(prg->pmts, 0, prg->nb_pmt_sections * sizeof(void *));
	}

	if (pmt->head.section_number > pmt->head.last_section_number)
		return -EINVAL;

	pmt_copy = prg->pmts[pmt->head.section_number];

	if (pmt_copy == NULL || pmt_copy->head.length != pmt->head.length) {
		free(pmt_copy);
		pmt_copy = malloc(pmt->head.length + sizeof(struct section));
		if (pmt_copy == NULL)
			return -ENOMEM;

		memcpy(pmt_copy, pmt, pmt->head.length + sizeof(struct section));
		prg->pmts[pmt_copy->head.section_number] = pmt_copy;
		prg->program_number = pmt->head.table_id_ext;

		dvb_ca_send_pmt(prg->ca);
		return 0;
	}

#if 1
	if (pmt_copy->head.version_number == pmt->head.version_number)
		return 0;
#endif

	memcpy(pmt_copy, pmt, pmt->head.length + sizeof(struct section));
	prg->program_number = pmt->head.table_id_ext;

	dvb_ca_send_pmt(prg->ca);
	return 0;
}

static inline int free_ca_session(struct dvb_ca_session * ca_session)
{
	dvb_ci_close_app(ca_session->app);
	return 0;
}

static inline struct dvb_ca_session *
	dvb_ca_find_ca_session(struct dvb_ca * ca,
			       struct dvb_ci_session * session)
{
	struct dvb_ca_session * ca_session;
	struct list_entry * pos;

	for (pos = ca->sessions.next; pos; pos = pos->next) {
		ca_session = list_get_entry(pos, struct dvb_ca_session, list);
		if (ca_session->session == session)
			return ca_session;
	}

	return NULL;
}

static inline struct dvb_ca_program *
	dvb_ca_find_program(struct dvb_ca * ca, int program_number)
{
	struct dvb_ca_program * prg;
	struct list_entry * pos;

	for (pos = ca->programs.next; pos; pos = pos->next) {
		prg = list_get_entry(pos, struct dvb_ca_program, list);
		if (prg->program_number == program_number)
			return prg;
	}

	return NULL;
}

static inline struct dvb_ca_stream *
	dvb_ca_find_stream(struct dvb_ca_program * prg, int pid)
{
	struct dvb_ca_stream * stream;
	struct list_entry * pos;

	for (pos = prg->streams.next; pos; pos = pos->next) {
		stream = list_get_entry(pos, struct dvb_ca_stream, list);
		if (stream->pid == pid)
			return stream;
	}

	return NULL;
}

static inline int
	__ca_session_has_ca_system_id(struct dvb_ca_session * session,
				      uint16_t ca_system_id)
{
	int i;

	for (i = 0; i < session->nb_ca_system_ids; ++i) {
		if (session->ca_system_ids[i] == ca_system_id)
			return 1;
	}

	return 0;
}

static inline int
	__section_has_ca_system_id(struct mpeg_pmt_section * pmt,
				   struct dvb_ca_session * session)
{
	struct descriptor * pos;
	struct mpeg_pmt_stream * stream;

	mpeg_pmt_section_descriptors_for_each(pmt, pos) {
		struct mpeg_ca_descriptor * d = (void *)pos;
		if (pos->tag != dtag_mpeg_ca)
			continue;
		if (__ca_session_has_ca_system_id(session, d->ca_system_id))
			return 1;
	}

	mpeg_pmt_section_streams_for_each(pmt, stream) {
		mpeg_pmt_stream_descriptors_for_each(stream, pos) {
			struct mpeg_ca_descriptor * d = (void *)pos;
			if (pos->tag != dtag_mpeg_ca)
				continue;
			if (__ca_session_has_ca_system_id(session, d->ca_system_id))
				return 1;
		}
	}

	return 0;
}

static inline int
	__program_has_ca_system_id(struct dvb_ca_program * prg,
				   struct dvb_ca_session * session)
{
	int i;

	for (i = 0; i < prg->nb_pmt_sections; ++i) {
		struct mpeg_pmt_section * pmt = prg->pmts[i];

		if (pmt == NULL) // XXX
			continue;

		if (__section_has_ca_system_id(pmt, session))
			return 1;
	}

	return 0;
}

static int __ca_send_pmt(struct dvb_ca_session * ca_session,
			 struct dvb_ca_program * prg)
{
	struct dvb_ci_apdu * apdu;
	uint8_t buf[4096]; // FIXME: variable buffer
	int len = 0, len_pos, i;

	if (__program_has_ca_system_id(prg, ca_session) == 0)
		return 0;

	buf[len++] = prg->list_mgmt;
	buf[len++] = prg->program_number >> 8;
	buf[len++] = prg->program_number;
	buf[len++] = 0xff;
	len_pos = len;
	buf[len++] = 0x00;
	buf[len++] = 0x00;

	for (i = 0; i < prg->nb_pmt_sections; ++i) {
		struct descriptor * pos;
		struct mpeg_pmt_section * pmt = prg->pmts[i];

		if (pmt == NULL) // XXX
			continue;

		mpeg_pmt_section_descriptors_for_each(pmt, pos) {
			struct mpeg_ca_descriptor * d = (void *)pos;
			int pi, plen;
			uint8_t * pdata;

			if (pos->tag != dtag_mpeg_ca)
				continue;

			if (__ca_session_has_ca_system_id(ca_session, d->ca_system_id) == 0)
				continue;

			if (len_pos + 2 == len)
				buf[len++] = prg->cmd;

			buf[len++] = d->d.tag;
			buf[len++] = d->d.len;
			buf[len++] = d->ca_system_id >> 8;
			buf[len++] = d->ca_system_id;
			buf[len++] = d->ca_pid >> 8;
			buf[len++] = d->ca_pid;

			pdata = mpeg_ca_descriptor_data(d);
			plen = mpeg_ca_descriptor_data_length(d);

			for (pi = 0; pi < plen; ++pi)
				buf[len++] = pdata[pi];
		}
	}

	buf[len_pos+0] = (len - (len_pos + 2)) >> 8;
	buf[len_pos+1] = (len - (len_pos + 2));

	for (i = 0; i < prg->nb_pmt_sections; ++i) {
		struct mpeg_pmt_stream * stream;
		struct mpeg_pmt_section * pmt = prg->pmts[i];

		if (pmt == NULL) // XXX
			continue;

		mpeg_pmt_section_streams_for_each(pmt, stream) {
			struct dvb_ca_stream * ca_stream;
			struct descriptor * pos;
			int cmd = ca_pmt_cmd_not_selected;

			ca_stream = dvb_ca_find_stream(prg, stream->pid);
			if (ca_stream != NULL)
				cmd = ca_stream->cmd;

			if (cmd == ca_pmt_cmd_not_selected)
				continue;

			buf[len++] = stream->stream_type;
			buf[len++] = stream->pid >> 8;
			buf[len++] = stream->pid;
			len_pos = len;
			buf[len++] = 0x00;
			buf[len++] = 0x00;

			dprintf(DBG_CA_PMT, "stream_type %x, pid %x, cmd %.2x\n",
				stream->stream_type, stream->pid, cmd);

			mpeg_pmt_stream_descriptors_for_each(stream, pos) {
				struct mpeg_ca_descriptor * d = (void *)pos;
				int pi, plen;
				uint8_t * pdata;

				if (pos->tag != dtag_mpeg_ca)
					continue;

				if (__ca_session_has_ca_system_id(ca_session, d->ca_system_id) == 0)
					continue;

				if (len_pos + 2 == len)
					buf[len++] = cmd;

				pdata = mpeg_ca_descriptor_data(d);
				plen = mpeg_ca_descriptor_data_length(d);

				dprintf(DBG_CA_PMT, "stream level %.4x:%.4x (%i pb)\n",
					d->ca_system_id, d->ca_pid, plen);

				buf[len++] = d->d.tag;
				buf[len++] = d->d.len;
				buf[len++] = d->ca_system_id >> 8;
				buf[len++] = d->ca_system_id;
				buf[len++] =(d->ca_pid >> 8) & 0x1f;
				buf[len++] = d->ca_pid;

				for (pi = 0; pi < plen; ++pi)
					buf[len++] = pdata[pi];
			}

			buf[len_pos+0] = (len - (len_pos + 2)) >> 8;
			buf[len_pos+1] = (len - (len_pos + 2));
		}
	}

	if (dvb_ci_create_apdu(dvb_ci_aot_ca_pmt, len, buf, &apdu))
		return -ENOMEM;

	dprintf(DBG_CA_PMT_DUMP, "sending %.6x %i bytes, data:\n",
		apdu->tag, apdu->len);

	for (i = 0; i < len; ++i) {
		if (i % 8 == 0)
			dprintf(DBG_CA_PMT_DUMP, "\n");
		dprintf(DBG_CA_PMT_DUMP, " 0x%.2x", buf[i]);
	}
	dprintf(DBG_CA_PMT_DUMP, "\n\n");

	dvb_ci_send_apdu(ca_session->session, apdu);

	return 0;
}

static int dvb_ca_send_pmt(struct dvb_ca * ca)
{
	struct dvb_ca_session * ca_session;
	struct dvb_ca_program * prg;
	struct list_entry * ca_pos, * prg_pos;

	for (ca_pos = ca->sessions.next; ca_pos; ca_pos = ca_pos->next) {
		ca_session = list_get_entry(ca_pos, struct dvb_ca_session, list);

		for (prg_pos = ca->programs.next; prg_pos; prg_pos = prg_pos->next) {
			prg = list_get_entry(prg_pos, struct dvb_ca_program, list);

			__ca_send_pmt(ca_session, prg);
		}
	}

	return 0;
}

static int handle_ca_pmt_reply(struct dvb_ci_application * app,
			       struct dvb_ci_apdu * apdu)
{
	struct dvb_ca_session * ca_session = app->opaque;
	int program_number = apdu->data[0] << 8 | apdu->data[1];
	struct dvb_ca_program * prg;
	uint8_t ca_enable = apdu->data[3];
	int pos = 4, i;

	dprintf(DBG_CA_PMT_REPLY, "ca_pmt_reply: %x %x\n",
		program_number, ca_enable & 0x7f);

	dprintf(DBG_CA_PMT_REPLY_DUMP, "recived %.6x %i bytes, data:\n",
		apdu->tag, apdu->len);

	for (i = 0; i < apdu->len; ++i) {
		if (i % 8 == 0)
			dprintf(DBG_CA_PMT_REPLY_DUMP, "\n");
		dprintf(DBG_CA_PMT_REPLY_DUMP, " 0x%.2x", apdu->data[i]);
	}
	dprintf(DBG_CA_PMT_REPLY_DUMP, "\n\n");

	prg = dvb_ca_find_program(ca_session->ca, program_number);
	if (prg == NULL)
		return -EINVAL;

	if (ca_enable & 0x80 && (ca_enable & 0x7f) == 0x01) {
		struct list_entry * pos;

		prg->cmd = ca_pmt_cmd_ok_descrambling;

		for (pos = prg->streams.next; pos; pos = pos->next) {
			struct dvb_ca_stream * stream;
			stream = list_get_entry(pos, struct dvb_ca_stream, list);
			stream->cmd = ca_pmt_cmd_ok_descrambling;
		}
	}

	while (pos < apdu->len) {
		struct dvb_ca_stream * stream;
		int pid = (apdu->data[pos] & 0x1f) << 8 | apdu->data[pos+1];
		ca_enable = apdu->data[pos+2];
		pos += 3;

		stream = dvb_ca_find_stream(prg, pid);
		if (stream == NULL)
			continue;

		dprintf(DBG_CA_PMT_REPLY, "ca_pmt_reply: stream %x %x\n",
			stream->pid, ca_enable);

		if (ca_enable & 0x80 && (ca_enable & 0x7f) == 0x01) {
			stream->cmd = ca_pmt_cmd_ok_descrambling;
		}
	}

	dvb_ca_send_pmt(ca_session->ca);
	return 0;
}

static int handle_ca_info(struct dvb_ci_application * app,
			  struct dvb_ci_apdu * apdu)
{
	struct dvb_ca_session * ca_session = app->opaque;
	int i;

	if (ca_session->nb_ca_system_ids != apdu->len / 2) {
		free(ca_session->ca_system_ids);
		ca_session->ca_system_ids = malloc((apdu->len / 2) * sizeof(uint16_t));
		if (ca_session->ca_system_ids == NULL)
			return -ENOMEM;

		ca_session->nb_ca_system_ids = apdu->len / 2;
	}

	fprintf(stderr, "Conditional Access Info:");
	for (i = 0; i < apdu->len; i += 2) {
		uint16_t id = apdu->data[i] << 8 | apdu->data[i+1];
		fprintf(stderr, " 0x%.4x", id);
		ca_session->ca_system_ids[i/2] = id;
	}
	fprintf(stderr, "\n");

	dvb_ca_send_pmt(ca_session->ca);
	return 0;
}

static int dvb_ca_recv_apdu(struct dvb_ci_application * app,
			    struct dvb_ci_apdu * apdu)
{
	switch (apdu->tag) {
		case dvb_ci_aot_ca_info:
			return handle_ca_info(app, apdu);

		case dvb_ci_aot_ca_pmt_reply:
			return handle_ca_pmt_reply(app, apdu);

		default:
			fprintf(stderr, "Unexpected ca apdu %.6x\n", apdu->tag);
			break;
	}

	return 0;
}

