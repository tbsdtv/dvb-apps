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
#include <dvb/program.h>

#include <stdio.h>
#include <assert.h>
#include <list.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <si/common.h>
#include <si/descriptor.h>
#include <si/section.h>
#include <si/mpeg/section.h>
#include <si/dvb/section.h>
#include <si/mpeg/pat_section.h>
#include <si/mpeg/pmt_section.h>
#include <si/dvb/bat_section.h>
#include <si/dvb/sdt_section.h>
#include <si/dvb/nit_section.h>
#include <si/dvb/eit_section.h>

#include <ci/ca.h>

static struct list_entry programs;
struct dvb_ca * ca = NULL;
struct dvb_program * ca_program = NULL;
int last_version = -1;


static struct dvb_program * find_program(int program_number)
{
	struct dvb_program * program;
	struct list_entry * pos;

	for (pos = programs.next; pos; pos = pos->next) {
		program = list_get_entry(pos, struct dvb_program, list);
		if (program->program_number == program_number)
			return program;
	}

	return NULL;
}

void parse_pat(dvb_filter_t * filter, struct section_ext * ext)
{
	struct mpeg_pat_section * pat;
	struct list_entry old_programs = programs;
	struct list_entry * pos, * next;
	struct dvb_program * program;
	struct mpeg_pat_program * patpos;

	pat = mpeg_pat_section_parse(ext);
	if (pat == NULL)
		return;

	LIST_ENTRY_INIT(&programs);

	mpeg_pat_section_programs_for_each(pat, patpos) {
		if (patpos->program_number == 0)
			continue;

		for (pos = old_programs.next; pos; pos = next) {
			next = pos->next;
			program = list_get_entry(pos, struct dvb_program, list);
			if (program->program_number != patpos->program_number)
				continue;
			list_remove_entry(&old_programs, pos);
			list_append_entry(&programs, pos);
			break;
		}

		if (pos == NULL) {
			program = malloc(sizeof(struct dvb_program));
			if (program == NULL)
				return; /* -ENOMEM */

			memset(program, 0, sizeof(struct dvb_program));
			LIST_ENTRY_INIT(&program->streams);
			program->program_number = patpos->program_number;
			program->pmtpid = patpos->pid;

#if 1
			fprintf(stderr, "adding program %i:%i\n",
				patpos->program_number, patpos->pid);
#endif

			if (dvb_section_filter_add(filter, patpos->pid, -1,
					           &program->pmt_filter_entry))
				fprintf(stderr, "seting pmtfilter failed\n");

			list_append_entry(&programs, &program->list);
		}
	}

	
	for (pos = old_programs.next; pos; pos = next) {
		next = pos->next;
		program = list_get_entry(pos, struct dvb_program, list);
		list_remove_entry(&old_programs, pos);

		if (ca_program == program)
			ca_program = NULL;

#if 0
		fprintf(stderr, "removing program %i:%i\n",
			program->program_number, program->pmtpid);
#endif

		if (dvb_filter_remove(program->pmt_filter_entry))
			fprintf(stderr, "seting pmtfilter failed\n");

		free(program);
	}
}

void parse_pmt(dvb_filter_t * filter, struct section_ext * ext)
{
	struct mpeg_pmt_section * pmt;
	struct mpeg_pmt_stream * stream = NULL;
	struct descriptor * descriptor = NULL;
	struct dvb_program * program;

	pmt = mpeg_pmt_section_parse(ext, SYSTEM_MPEG | SYSTEM_DVB);
	if (pmt == NULL)
		return;

	program = find_program(pmt->head.table_id_ext);
	if (program == NULL) {
		fprintf(stderr, "Unable to find program %i\n",
				pmt->head.table_id_ext);
		return;
	}

	if (program->pmt == NULL ||
	    program->pmt->head.length != pmt->head.length) {
		program->pmt = malloc(sizeof(struct section) + pmt->head.length);
		if (program->pmt == NULL)
			return;
	}

	memcpy(program->pmt, pmt, sizeof(struct section) + pmt->head.length);

	if (ca_program == NULL && program->program_number == 605)//7010)
		ca_program = program;

	if (ca_program == program) {
		if (ca == NULL)
			return;

		last_version = program->pmt->head.version_number;

		if (program->ca_program == NULL)
			dvb_ca_create_program(ca, &program->ca_program);

		mpeg_pmt_section_streams_for_each(pmt, stream) {
			if (stream->stream_type > 4)
				continue;

			dvb_ca_select_stream(program->ca_program, stream->pid);
		}

		dvb_ca_update_section(program->ca_program, pmt);
	}

	//fprintf(stderr, "-------------------------- program map\n");
	mpeg_pmt_section_descriptors_for_each(pmt, descriptor) {
		//fprintf(stderr, "descriptor: %x\n", descriptor->tag);
	}

	mpeg_pmt_section_streams_for_each(pmt, stream) {
		//fprintf(stderr, "stream %i\n", stream->pid);

		mpeg_pmt_stream_descriptors_for_each(stream, descriptor) {
			//fprintf(stderr, "\tdescriptor: %x\n", descriptor->tag);
		}
	}
}

void parse_data(dvb_filter_t * filter, uint8_t * buf, int len)
{
	struct section * section;
	struct section_ext * section_ext;

	section = parse_section(buf, len);
	if (section == NULL)
		return;

	section_ext = parse_section_ext(section, 1);
	if (section_ext == NULL)
		return;

	switch (section->table_id) {
		case sct_program_association:
			parse_pat(filter, section_ext);
			break;

		case sct_program_map:
			parse_pmt(filter, section_ext);
			break;
	}
}

static dvb_filter_t * small_filter = NULL;
static dvb_filter_t * large_filter = NULL;

void handle_demux_event(struct dvb * dvb, dvb_event_t * event)
{
	struct dvb_demux_event data;

	if (dvb_get_demux_event_data(event, &data))
		return;

	switch (data.type) {
	case dvb_demux_open_event:
		dvb_open_filter(data.demux, dvb_section_filter, &small_filter);
		dvb_section_filter_add(small_filter, 0x00, 0x0, NULL);

		dvb_open_filter(data.demux, dvb_section_filter, &large_filter);
		break;

	case dvb_demux_close_event:
		break;

	case dvb_demux_data_event:
		parse_data(data.filter, data.data, data.len);
		break;
	}
}

void handle_frontend_event(struct dvb * dvb, dvb_event_t * event)
{
	struct dvb_frontend_event_data data;

	if (dvb_get_frontend_event_data(event, &data))
		return;

	switch (data.type) {
	case dvb_frontend_open_event:
	{
		dvb_ci_t * ci;
		dvb_demux_t * demux;

		if (dvb_frontend_readonly(data.frontend)) {
			dvb_open_frontend(data.frontend);
		}

		dvb_open_demux(data.frontend, &demux);
		dvb_open_ci(data.frontend, &ci);
		dvb_create_ca(&ca);
		break;
	}
	case dvb_frontend_close_event:
		break;
	case dvb_frontend_tune_event:
		break;
	case dvb_frontend_lock_event:
		break;
	}
}

void handle_ci_session_create_event(struct dvb * dvb, struct dvb_ci_event_data * data)
{
	int ret;

	switch (dvb_ci_get_session_resource_id(data->session))
	{
		case dvb_ci_rid_conditional_access_support:
		{
			if ((ret = dvb_ca_add_session(ca, data->session))) {
				fprintf(stderr, "dvb_ca_open failed: %i\n", ret);
				return;
			}
			break;
		}

		default:
#if 0
			fprintf(stderr, "unknown session created, resource id = 0x%.6x\n",
				dvb_ci_get_session_resource_id(data->session));
#endif
			break;
	}
}

void handle_ci_session_close_event(struct dvb * dvb, struct dvb_ci_event_data * data)
{
	int ret;

	switch (dvb_ci_get_session_resource_id(data->session))
	{
		case dvb_ci_rid_conditional_access_support:
		{
			if ((ret = dvb_ca_remove_session(ca, data->session))) {
				fprintf(stderr, "dvb_ca_open failed: %i\n", ret);
				return;
			}
			break;
		}

		default:
#if 0
			fprintf(stderr, "unknown session created, resource id = 0x%.6x\n",
				dvb_ci_get_session_resource_id(data->session));
#endif
			break;
	}
}

void handle_ci_event(struct dvb * dvb, dvb_event_t * event)
{
	struct dvb_ci_event_data data;
	if (dvb_get_ci_event_data(event, &data))
		return;

	switch (data.type) {
		case dvb_ci_module_insert_event:
			//fprintf(stderr, "module inserted in slot %p\n", data.slot);
			break;

		case dvb_ci_module_eject_event:
			//fprintf(stderr, "module ejected from slot %p\n", data.slot);
			break;

		case dvb_ci_application_info_event:
		{
			const struct dvb_ci_application_info * info;
			const char * typestr;

			info = dvb_ci_get_application_info(data.slot);
			if (info == NULL)
				return;

			switch (info->type) {
				case 0x01: typestr = "Conditional Access"; break;
				case 0x02: typestr = "Electronic Program Guide"; break;
				default:   typestr = "Reserved"; break;
			}

			fprintf(stderr, "%s, %s\n", typestr, info->name);
			break;
		}

		case dvb_ci_session_create_event:
			handle_ci_session_create_event(dvb, &data);
			break;

		case dvb_ci_session_close_event:
			handle_ci_session_close_event(dvb, &data);
			break;
	}
}

void my_hook(struct dvb * dvb, dvb_event_t * event, void * opaque)
{
	switch (dvb_get_event_type(event)) {
	case dvb_frontend_event:
		handle_frontend_event(dvb, event);
		break;

	case dvb_demux_event:
		handle_demux_event(dvb, event);
		break;

	case dvb_ci_event:
		handle_ci_event(dvb, event);
		break;

	default:
		fprintf(stderr, "unknown event\n");
	}
}

int main(int argc, char ** argv)
{
	int ret;
	struct dvb * dvb;
	struct dvb_adapter * adapter;

	if ((ret = dvb_create_handle(&dvb))) {
		fprintf(stderr, "dvb_create_handle failed %i:%s\n",
			ret, dvb_strerror(ret));
		return ret;
	}

	if ((ret = dvb_add_event_hook(dvb, my_hook, NULL))) {
		fprintf(stderr, "dvb_add_event_hook failed %i:%s\n",
			ret, dvb_strerror(ret));
		return ret;
	}

	if ((ret = dvb_open_adapter(dvb, "dvb:0", &adapter))) {
		fprintf(stderr, "dvb_open_adapter failed %i:%s\n",
			ret, dvb_strerror(ret));
		return ret;
	}

	return dvb_enter_thread_loop(dvb);
}

#if 0
void parse_nit(struct dvb_demux * demux, struct section_ext * ext)
{
	struct descriptor * desc;
	struct nit_transport * pos;
	struct nit_section * nit;

	nit = parse_nit_section(ext);
	if (nit == NULL)
		return;

	nit_for_each_network_descriptor(desc, nit) {
	}

	nit_for_each_transport(pos, nit) {
		//fprintf(stderr, "\ttransport %x\n", pos->transport_stream_id);
		nit_for_each_transport_descriptor(desc, pos) {
			//fprintf(stderr, "\t\tdesc->tag %x\n", desc->tag);
		}
	}
}

void parse_sdt(struct dvb_demux * demux, struct section_ext * ext)
{
	struct descriptor * desc;
	struct sdt_service * pos;
	struct sdt_section * sdt;

	sdt = parse_sdt_section(ext);
	if (sdt == NULL)
		return;

	sdt_for_each_service(pos, sdt) {
		//fprintf(stderr, "\tservice %i / %i\n", pos->service_id, pos->descriptors_loop_length);
		sdt_for_each_service_descriptor(desc, pos) {
			//fprintf(stderr, "\t\tdesc->tag %x\n", desc->tag);
		}
	}
}

static inline void eit_event_start_time(uint64_t time, uint32_t duration)
{
#if 0
	int seconds = (((time & 0xf0) >> 4) * 10) + ((time & 0xf) >> 0);
	int minutes = (((time & 0xf000) >> 12) * 10) + ((time & 0xf00) >> 8);
	int hours   = (((time & 0xf00000) >> 20) * 10) + ((time & 0xf0000) >> 16);

	int d_seconds = (((duration & 0xf0) >> 4) * 10) + ((duration & 0xf) >> 0);
	int d_minutes = (((duration & 0xf000) >> 12) * 10) + ((duration & 0xf00) >> 8);
	int d_hours   = (((duration & 0xf00000) >> 20) * 10) + ((duration & 0xf0000) >> 16);

	fprintf(stderr, "		event %.2i:%.2i:%.2i (%.2i:%.2i:%.2i)\r",
		hours, minutes, seconds, d_hours, d_minutes, d_seconds);
#endif
}

void parse_eit(struct dvb_demux * demux, struct section_ext * ext)
{
	struct eit_section * eit;
	int count, i;

	eit = parse_eit_section(ext);
	if (eit == NULL)
		return;

	count = eit_event_count(eit);
	//fprintf(stderr, "eit: %x / %i\r", ext->table_id_ext, count);

	for (i = 0; i < count; ++i) {
		struct eit_event * event = eit_event(eit, i);
		if (event == NULL)
			return;

		eit_event_start_time(event->start_time, event->duration);
	}
}
#endif

#if 0
		case sct_network_information_actual:
		case sct_network_information_other:
			parse_nit(demux, section_ext);
			break;

		case sct_service_description_actual:
		case sct_service_description_other:
			parse_sdt(demux, section_ext);
			break;

		case 0x4e ... 0x6f:
			parse_eit(demux, section_ext);
			break;
#endif

