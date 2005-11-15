/*
 * section and descriptor parser test/sample application.
 *
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
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

#include <ucsi/mpeg/descriptor.h>
#include <ucsi/mpeg/section.h>
#include <ucsi/dvb/descriptor.h>
#include <ucsi/dvb/section.h>
#include <ucsi/transport_packet.h>
#include <ucsi/section_buf.h>
#include <ucsi/dvb/types.h>
#include <dvbapi/dvbdemux.h>
#include <dvbapi/dvbfe.h>
#include <dvbcfg/dvbcfg_seed_backend_file.h>
#include <dvbcfg/dvbcfg_diseqc_backend_file.h>
#include <dvbcfg/dvbcfg_source.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

void receive_data(int dvrfd, int timeout);
void parse_section(uint8_t *buf, int len, int pid);
void parse_descriptor(struct descriptor *d, int indent);
void iprintf(int indent, char *fmt, ...);
void hexdump(int indent, char *prefix, uint8_t *buf, int buflen);

#define TIME_CHECK_VAL 1131835761
#define DURATION_CHECK_VAL 5643

#define MAX_TUNE_TIME 3000
#define MAX_DUMP_TIME 60


int main(int argc, char *argv[])
{
	int demuxfd;
	int dvrfd;
	int adapter;
	char *seedfile;
	char *diseqcfile;
	int pidlimit = -1;
	dvbdate_t dvbdate;
	dvbduration_t dvbduration;
	struct dvbcfg_seed_backend *seedbackend;
	struct dvbcfg_seed *seeds = NULL;
	struct dvbcfg_diseqc_backend *diseqcbackend;
	struct dvbcfg_diseqc *diseqcs = NULL;
	struct dvbcfg_source *sources = NULL;
	dvbfe_handle_t fe;
	struct dvbfe_info feinfo;
	struct dvbcfg_diseqc *diseqc_wildcard;
	struct dvbcfg_diseqc *diseqc;
	struct dvbcfg_diseqc_entry *dentry;

	// process arguments
	if ((argc < 4) || (argc > 5)) {
		fprintf(stderr, "Syntax: testucsi <adapter id> <seed file> <diseqc file> [<pid to limit to>]\n");
		exit(1);
	}
	adapter = atoi(argv[1]);
	seedfile = argv[2];
	diseqcfile = argv[3];
	if (argc == 5)
		sscanf(argv[4], "%i", &pidlimit);
	printf("Using adapter %i\n", adapter);

	// check the dvbdate conversion functions
	unixtime_to_dvbdate(TIME_CHECK_VAL, dvbdate);
	if (dvbdate_to_unixtime(dvbdate) != TIME_CHECK_VAL) {
		fprintf(stderr, "XXXX dvbdate function check failed (%i!=%i)\n",
			TIME_CHECK_VAL, (int) dvbdate_to_unixtime(dvbdate));
		exit(1);
	}
	seconds_to_dvbduration(DURATION_CHECK_VAL, dvbduration);
	if (dvbduration_to_seconds(dvbduration) != DURATION_CHECK_VAL) {
		fprintf(stderr, "XXXX dvbduration function check failed (%i!=%i)\n",
			DURATION_CHECK_VAL, (int) dvbduration_to_seconds(dvbduration));
		exit(1);
	}

	// open the frontend
	if ((fe = dvbfe_open(adapter, 0, 0)) == NULL) {
		perror("open frontend");
		exit(1);
	}
	if (dvbfe_get_info(fe, 0, &feinfo)) {
		perror("get feinfo");
		exit(1);
	}

	// try and open the seed file
	if (dvbcfg_seed_backend_file_create("/", seedfile, 1, &sources, 1, &seedbackend)) {
		fprintf(stderr, "XXXX Failed to create seed backend\n");
		exit(1);
	}
	if (dvbcfg_seed_load(seedbackend, &seeds)) {
		fprintf(stderr, "XXXX Failed to load seeds from supplied file\n");
		exit(1);
	}

	// try and open the disqec file
	if (dvbcfg_diseqc_backend_file_create(diseqcfile, &sources, 1, &diseqcbackend)) {
		fprintf(stderr, "XXXX Failed to create diseqc backend\n");
		exit(1);
	}
	if (dvbcfg_diseqc_load(diseqcbackend, &diseqcs)) {
		fprintf(stderr, "XXXX Failed to load disecqs from supplied file\n");
		exit(1);
	}

	// open demux devices
	if ((demuxfd = dvbdemux_open_demux(adapter, 0, 0)) < 0) {
		perror("demux");
		exit(1);
	}
	if ((dvrfd = dvbdemux_open_dvr(adapter, 0, 1, 1)) < 0) {
		perror("dvr");
		exit(1);
	}

	// make the demux buffer a bit larger
	if (dvbdemux_set_buffer(demuxfd, 1024*1024)) {
		perror("set buffer");
		exit(1);
	}

	// setup filter to capture stuff
	if (dvbdemux_set_pid_filter(demuxfd, pidlimit, DVBDEMUX_INPUT_FRONTEND, DVBDEMUX_OUTPUT_DVR, 1)) {
		perror("set pid filter");
		exit(1);
	}

	// process all seeds
	diseqc_wildcard = dvbcfg_diseqc_find(diseqcs, NULL);
	while(seeds) {
		struct dvbfe_parameters delivery;
		delivery = seeds->delivery.u.dvb;
		if (feinfo.type == DVBFE_TYPE_DVBS) {
			if ((diseqc = dvbcfg_diseqc_find(diseqcs, seeds->source)) == NULL) {
				diseqc = diseqc_wildcard;
			}
			if (diseqc != NULL) {
				if ((dentry = dvbcfg_diseqc_find_entry(diseqc,
						seeds->delivery.u.dvb.frequency,
						seeds->delivery.u.dvb.u.dvbs.polarization)) != NULL) {
					printf("Diseqc: %s\n", dentry->command);
					dvbfe_diseqc_command(fe, dentry->command);
				}
			}
		}

		printf("Tuning to: %i(%i) %i %i\n",
		       seeds->delivery.u.dvb.frequency,
		       delivery.frequency,
		       delivery.u.dvbs.symbol_rate,
		       delivery.u.dvbs.fec_inner);
		if (dvbfe_set(fe, &delivery, MAX_TUNE_TIME)) {
			fprintf(stderr, "Failed to lock!\n");
		} else {
			printf("Tuned successfully!\n");
			receive_data(dvrfd, MAX_DUMP_TIME);
		}

		seeds = seeds->next;
	}

	return 0;
}

void receive_data(int dvrfd, int timeout)
{
	unsigned char databuf[TRANSPORT_PACKET_LENGTH*20];
	int sz;
	int pid;
	int i;
	int used;
	int section_status;
	time_t starttime;
	unsigned char continuities[TRANSPORT_MAX_PIDS];
	struct section_buf *section_bufs[TRANSPORT_MAX_PIDS];
	struct transport_packet *tspkt;
	struct transport_values tsvals;

	// process the data
	starttime = time(NULL);
	memset(continuities, 0, sizeof(continuities));
	memset(section_bufs, 0, sizeof(section_bufs));
	while((time(NULL) - starttime) < timeout) {
		// got some!
		if ((sz = read(dvrfd, databuf, sizeof(databuf))) < 0) {
			if (errno == EOVERFLOW) {
				fprintf(stderr, "data overflow!\n");
				continue;
			} else if (errno == EAGAIN) {
				usleep(100);
				continue;
			} else {
				perror("read error");
				exit(1);
			}
		}
		for(i=0; i < sz; i+=TRANSPORT_PACKET_LENGTH) {
			// parse the transport packet
			tspkt = transport_packet_init(databuf + i);
			if (tspkt == NULL) {
				fprintf(stderr, "XXXX Bad sync byte\n");
				continue;
			}
			pid = transport_packet_pid(tspkt);

			// extract all TS packet values even though we don't need them (to check for
			// library segfaults etc)
			if (transport_packet_values_extract(tspkt, &tsvals, 0xffff) < 0) {
				fprintf(stderr, "XXXX Bad packet received (pid:%04x)\n", pid);
				continue;
			}

			// check continuity
			if (transport_packet_continuity_check(tspkt,
			    tsvals.flags & transport_adaptation_flag_discontinuity,
			    continuities + pid)) {
				fprintf(stderr, "XXXX Continuity error (pid:%04x)\n", pid);
				continuities[pid] = 0;
				if (section_bufs[pid] != NULL) {
					section_buf_reset(section_bufs[pid]);
				}
				continue;
			}

			// allocate section buf if we don't have one already
			if (section_bufs[pid] == NULL) {
				section_bufs[pid] = (struct section_buf*)
					malloc(sizeof(struct section_buf) + DVB_MAX_SECTION_BYTES);
				if (section_bufs[pid] == NULL) {
					fprintf(stderr, "Failed to allocate section buf (pid:%04x)\n", pid);
					exit(1);
				}
				section_buf_init(section_bufs[pid], DVB_MAX_SECTION_BYTES);
			}

			// process the payload data as a section
			while(tsvals.payload_length) {
				used = section_buf_add_transport_payload(section_bufs[pid],
									 tsvals.payload,
									 tsvals.payload_length,
									 tspkt->payload_unit_start_indicator,
									 &section_status);
				tspkt->payload_unit_start_indicator = 0;
				tsvals.payload_length -= used;
				tsvals.payload += used;

				if (section_status == 1) {
					parse_section(section_buf_data(section_bufs[pid]),
						      section_bufs[pid]->len, pid);
					section_buf_reset(section_bufs[pid]);
				} else if (section_status < 0) {
					// some kind of error - just discard
					fprintf(stderr, "XXXX bad section %04x %i\n",pid, section_status);
					section_buf_reset(section_bufs[pid]);
				}
			}
		}
	}
}

void parse_section(uint8_t *buf, int len, int pid)
{
	struct section *section;
	struct section_ext *section_ext = NULL;

	if ((section = section_codec(buf, len)) == NULL) {
		return;
	}

	switch(section->table_id) {
	case stag_mpeg_program_association:
	{
		struct mpeg_pat_section *pat;
		struct mpeg_pat_program *cur;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode PAT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((pat = mpeg_pat_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX PAT section decode error\n");
			return;
		}
		printf("transport_stream_id:0x%04x\n", mpeg_pat_section_transport_stream_id(pat));
		mpeg_pat_section_programs_for_each(pat, cur) {
			printf("\tprogram_number:0x%04x pid:0x%04x\n", cur->program_number, cur->pid);
		}
		break;
	}

	case stag_mpeg_conditional_access:
	{
		struct mpeg_cat_section *cat;
		struct descriptor *curd;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode CAT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((cat = mpeg_cat_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX CAT section decode error\n");
			return;
		}
		mpeg_cat_section_descriptors_for_each(cat, curd) {
			parse_descriptor(curd, 1);
		}
		break;
	}

	case stag_mpeg_program_map:
	{
		struct mpeg_pmt_section *pmt;
		struct descriptor *curd;
		struct mpeg_pmt_stream *cur_stream;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode PMT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((pmt = mpeg_pmt_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX PMT section decode error\n");
			return;
		}
		printf("program_number:0x%04x pcr_pid:0x%02x\n", mpeg_pmt_section_program_number(pmt), pmt->pcr_pid);
		mpeg_pmt_section_descriptors_for_each(pmt, curd) {
			parse_descriptor(curd, 1);
		}
		mpeg_pmt_section_streams_for_each(pmt, cur_stream) {
			printf("\tstream_type:0x%02x pid:0x%04x\n", cur_stream->stream_type, cur_stream->pid);
			mpeg_pmt_stream_descriptors_for_each(cur_stream, curd) {
				parse_descriptor(curd, 2);
			}
		}
		break;
	}

	case stag_mpeg_transport_stream_description:
	{
		struct mpeg_tsdt_section *tsdt;
		struct descriptor *curd;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode TSDT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((tsdt = mpeg_tsdt_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX TSDT section decode error\n");
			return;
		}
		mpeg_tsdt_section_descriptors_for_each(tsdt, curd) {
			parse_descriptor(curd, 1);
		}
		break;
	}

	case stag_mpeg_iso14496_scene_description:
	case stag_mpeg_iso14496_object_description:
	{
		struct mpeg_odsmt_section *odsmt;
		struct mpeg_odsmt_stream *cur_stream;
		struct descriptor *curd;
		int index;
		uint8_t *objects;
		int objects_length;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode ISO14496 (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((odsmt = mpeg_odsmt_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX ISO14496 section decode error\n");
			return;
		}
		printf("PID:0x%04x\n", mpeg_odsmt_section_pid(odsmt));
		mpeg_odsmt_section_streams_for_each(osdmt, cur_stream, index) {
			if (odsmt->stream_count == 0) {
				printf("\tSINGLE 0x%04x\n", cur_stream->u.single.esid);
			} else {
				printf("\tMULTI 0x%04x 0x%02x\n", cur_stream->u.multi.esid, cur_stream->u.multi.fmc);
			}
			mpeg_odsmt_stream_descriptors_for_each(osdmt, cur_stream, curd) {
				parse_descriptor(curd, 2);
			}
		}
		objects = mpeg_odsmt_section_object_descriptors(odsmt, &objects_length);
		if (objects == NULL) {
			printf("XXXX OSDMT parse error\n");
			break;
		}
		hexdump(1, "SCT ", objects, objects_length);
		break;
	}

	case stag_dvb_network_information_actual:
	case stag_dvb_network_information_other:
	{
		struct dvb_nit_section *nit;
	   	struct descriptor *curd;
		struct dvb_nit_section_part2 *part2;
		struct dvb_nit_transport *cur_transport;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode NIT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((nit = dvb_nit_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX NIT section decode error\n");
			return;
		}
		printf("network_id:0x%04x\n", dvb_nit_section_network_id(nit));
		dvb_nit_section_descriptors_for_each(nit, curd) {
			parse_descriptor(curd, 1);
		}
		part2 = dvb_nit_section_part2(nit);
		dvb_nit_section_transports_for_each(nit, part2, cur_transport) {
			printf("\ttransport_stream_id:0x%04x original_network_id:0x%04x\n", cur_transport->transport_stream_id, cur_transport->original_network_id);
			dvb_nit_transport_descriptors_for_each(cur_transport, curd) {
				parse_descriptor(curd, 2);
			}
		}
		break;
	}

	case stag_dvb_service_description_actual:
	case stag_dvb_service_description_other:
	{
		struct dvb_sdt_section *sdt;
		struct dvb_sdt_service *cur_service;
		struct descriptor *curd;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode SDT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((sdt = dvb_sdt_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX SDT section decode error\n");
			return;
		}
		printf("transport_stream_id:0x%04x original_network_id:0x%04x\n", dvb_sdt_section_transport_stream_id(sdt), sdt->original_network_id);
		dvb_sdt_section_services_for_each(sdt, cur_service) {
			printf("\tservice_id:0x%04x eit_schedule_flag:%i eit_present_following_flag:%i running_status:%i free_ca_mode:%i\n",
			       cur_service->service_id,
			       cur_service->eit_schedule_flag,
			       cur_service->eit_present_following_flag,
			       cur_service->running_status,
			       cur_service->free_ca_mode);
			dvb_sdt_service_descriptors_for_each(cur_service, curd) {
				parse_descriptor(curd, 2);
			}
		}
		break;
	}

	case stag_dvb_bouquet_association:
	{
		struct dvb_bat_section *bat;
		struct descriptor *curd;
		struct dvb_bat_section_part2 *part2;
		struct dvb_bat_transport *cur_transport;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode BAT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((bat = dvb_bat_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX BAT section decode error\n");
			return;
		}
		printf("bouquet_id:0x%04x\n", dvb_bat_section_bouquet_id(bat));
		dvb_bat_section_descriptors_for_each(bat, curd) {
			parse_descriptor(curd, 1);
		}
		part2 = dvb_bat_section_part2(bat);
		dvb_bat_section_transports_for_each(part2, cur_transport) {
			printf("\ttransport_stream_id:0x%04x original_network_id:0x%04x\n",
			       cur_transport->transport_stream_id,
			       cur_transport->original_network_id);
			dvb_bat_transport_descriptors_for_each(cur_transport, curd) {
				parse_descriptor(curd, 2);
			}
		}
		break;
	}

	case stag_dvb_ip_mac_notification:
	{
		struct dvb_int_section *_int;
		struct descriptor *curd;
		struct dvb_int_target *cur_target;
		struct dvb_int_operational_loop *operational_loop;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode INT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((_int = dvb_int_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX INT section decode error\n");
			return;
		}
		printf("action_type:0x%02x platform_id_hash:0x%02x platform_id:0x%06x processing_order:0x%02x\n",
		       dvb_int_section_action_type(_int),
		       dvb_int_section_platform_id_hash(_int),
		       _int->platform_id,
		       _int->processing_order);
		dvb_int_section_platform_descriptors_for_each(_int, curd) {
			parse_descriptor(curd, 1);
		}
		dvb_int_section_target_loop_for_each(_int, cur_target) {
			dvb_int_target_target_descriptors_for_each(cur_target, curd) {
				parse_descriptor(curd, 2);
			}
			operational_loop = dvb_int_target_operational_loop(cur_target);
			dvb_int_operational_loop_operational_descriptors_for_each(operational_loop, curd) {
				parse_descriptor(curd, 3);
			}
		}
		break;
	}

	case stag_dvb_event_information_nownext_actual:
	case stag_dvb_event_information_nownext_other:
	case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
	case 0x58: case 0x59: case 0x5a: case 0x5b: case 0x5c: case 0x5d: case 0x5e: case 0x5f:
	case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
	case 0x68: case 0x69: case 0x6a: case 0x6b: case 0x6c: case 0x6d: case 0x6e: case 0x6f:
	{
		struct dvb_eit_section *eit;
		struct dvb_eit_event *cur_event;
		struct descriptor *curd;
		time_t start_time;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode EIT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((eit = dvb_eit_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX EIT section decode error\n");
			return;
		}
		printf("service_id:0x%04x transport_stream_id:0x%04x original_network_id:0x%04x segment_last_section_number:0x%02x last_table_id:0x%02x\n",
		       dvb_eit_section_service_id(eit),
		       eit->transport_stream_id,
		       eit->original_network_id,
		       eit->segment_last_section_number,
		       eit->last_table_id);
		dvb_eit_section_events_for_each(eit, cur_event) {
			start_time = dvbdate_to_unixtime(cur_event->start_time);
			printf("\tevent_id:0x%04x duration:%i running_status:%i free_ca_mode:%i start_time:%i -- %s",
			       cur_event->event_id,
			       dvbduration_to_seconds(cur_event->duration),
			       cur_event->running_status,
			       cur_event->free_ca_mode,
			       (int) start_time,
			       ctime(&start_time));
			dvb_eit_event_descriptors_for_each(cur_event, curd) {
				parse_descriptor(curd, 2);
			}
		}
		break;
	}

	case stag_dvb_time_date:
	{
		struct dvb_tdt_section *tdt;
		time_t dvbtime;

		printf("Decode TDT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((tdt = dvb_tdt_section_codec(section)) == NULL) {
			fprintf(stderr, "XXXX TDT section decode error\n");
			return;
		}
		dvbtime = dvbdate_to_unixtime(tdt->utc_time);
		printf("Time: %i -- %s", (int) dvbtime, ctime(&dvbtime));
		break;
	}

	case stag_dvb_running_status:
	{
		struct dvb_rst_section *rst;
		struct dvb_rst_status *cur_status;

		printf("Decode RST (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((rst = dvb_rst_section_codec(section)) == NULL) {
			fprintf(stderr, "XXXX RST section decode error\n");
			return;
		}
		dvb_rst_section_statuses_for_each(rst, cur_status) {
			printf("\ttransport_stream_id:0x%04x original_network_id:0x%04x service_id:0x%04x event_id:0x%04x running_status:%i\n",
			       cur_status->transport_stream_id,
			       cur_status->original_network_id,
			       cur_status->service_id,
			       cur_status->event_id,
			       cur_status->running_status);
		}
		break;
	}

	case stag_dvb_stuffing:
	{
		struct dvb_st_section *st;

		printf("Decode ST (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((st = dvb_st_section_codec(section)) == NULL) {
			fprintf(stderr, "XXXX ST section decode error\n");
			return;
		}
		printf("Length: %i\n", dvb_st_section_data_length(st));
		break;
	}

	case stag_dvb_time_offset:
	{
		struct dvb_tot_section *tot;
		struct descriptor *curd;
		time_t dvbtime;

		if (section_check_crc(section))
			return;
		printf("Decode TOT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((tot = dvb_tot_section_codec(section)) == NULL) {
			fprintf(stderr, "XXXX TOT section decode error\n");
			return;
		}
		dvbtime = dvbdate_to_unixtime(tot->utc_time);
		printf("utc_time: %i -- %s", (int) dvbtime, ctime(&dvbtime));
		dvb_tot_section_descriptors_for_each(tot, curd) {
			parse_descriptor(curd, 1);
		}
		break;
	}

	case stag_dvb_discontinuity_information:
	{
		struct dvb_dit_section *dit;

		printf("Decode DIT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((dit = dvb_dit_section_codec(section)) == NULL) {
			fprintf(stderr, "XXXX DIT section decode error\n");
			return;
		}
		printf("transition_flag:%i\n", dit->transition_flag);
		break;
	}

	case stag_dvb_selection_information:
	{
		struct dvb_sit_section *sit;
		struct descriptor *curd;
		struct dvb_sit_service *cur_service;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode SIT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((sit = dvb_sit_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX SIT section decode error\n");
			return;
		}
		dvb_sit_section_descriptors_for_each(sit, curd) {
			parse_descriptor(curd, 1);
		}
		dvb_sit_section_services_for_each(sit, cur_service) {
			printf("\tservice_id:0x%04x running_status:%i\n", cur_service->service_id, cur_service->running_status);
			dvb_sit_service_descriptors_for_each(cur_service, curd) {
				parse_descriptor(curd, 2);
			}
		}
		break;
	}

	default:
		fprintf(stderr, "XXXX Unknown table_id:0x%02x (pid:0x%04x)\n", section->table_id, pid);
//		hexdump(0, "SCT ", buf, len);
		return;
	}

	printf("\n");
}

void parse_descriptor(struct descriptor *d, int indent)
{
	switch(d->tag) {
	case dtag_mpeg_video_stream:
	{
		struct mpeg_video_stream_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_video_stream_descriptor\n");
		dx = mpeg_video_stream_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_video_stream_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC multiple_frame_rate_flag:%i frame_rate_code:%i mpeg_1_only_flag:%i constrained_parameter_flag:%i still_picture_flag:%i\n",
			dx->multiple_frame_rate_flag,
			dx->frame_rate_code,
			dx->mpeg_1_only_flag,
			dx->constrained_parameter_flag,
			dx->still_picture_flag);
		if (!dx->mpeg_1_only_flag) {
			struct mpeg_video_stream_extra *extra = mpeg_video_stream_descriptor_extra(dx);
			iprintf(indent, "DSC profile_and_level_indication:0x%02x chroma_format:%i frame_rate_extension:%i\n",
				extra->profile_and_level_indication,
				extra->chroma_format,
				extra->frame_rate_extension);
		}
		break;
	}

	case dtag_mpeg_audio_stream:
	{
		struct mpeg_audio_stream_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_audio_stream_descriptor\n");
		dx = mpeg_audio_stream_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_audio_stream_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC free_format_flag:%i id:%i layer:%i variable_rate_audio_indicator:%i\n",
			dx->free_format_flag,
			dx->id,
			dx->layer,
			dx->variable_rate_audio_indicator);
		break;
	}

	case dtag_mpeg_hierarchy:
	{
		struct mpeg_hierarchy_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_hierarchy_descriptor\n");
		dx = mpeg_hierarchy_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_hierarchy_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC hierarchy_type:%i hierarchy_layer_index:%i hierarchy_embedded_layer_index:%i hierarchy_channel:%i\n",
			dx->hierarchy_type,
			dx->hierarchy_layer_index,
			dx->hierarchy_embedded_layer_index,
			dx->hierarchy_channel);
		break;
	}

	case dtag_mpeg_registration:
	{
		struct mpeg_registration_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_registration_descriptor\n");
		dx = mpeg_registration_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_registration_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC format_identifier:0x%x\n",
			dx->format_identifier);
		iprintf(indent, "DSC additional_id_info:\n");
		hexdump(indent, "DSC ",
			mpeg_registration_descriptor_additional_id_info(dx),
			mpeg_registration_descriptor_additional_id_info_length(dx));
		break;
	}

	case dtag_mpeg_data_stream_alignment:
	{
		struct mpeg_data_stream_alignment_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_data_stream_alignment_descriptor\n");
		dx = mpeg_data_stream_alignment_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_data_stream_alignment_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC alignment_type:%i\n",
			dx->alignment_type);
		break;
	}

	case dtag_mpeg_target_background_grid:
	{
		struct mpeg_target_background_grid_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_target_background_grid_descriptor\n");
		dx = mpeg_target_background_grid_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_target_background_grid_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC horizontal_size:%i vertical_size:%i aspect_ratio_information:%i\n",
			dx->horizontal_size,
		        dx->vertical_size,
		        dx->aspect_ratio_information);
		break;
	}

	case dtag_mpeg_video_window:
	{
		struct mpeg_video_window_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_video_window_descriptor\n");
		dx = mpeg_video_window_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_video_window_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC horizontal_offset:%i vertical_offset:%i window_priority:%i\n",
			dx->horizontal_offset,
			dx->vertical_offset,
			dx->window_priority);
		break;
	}

	case dtag_mpeg_ca:
	{
		struct mpeg_ca_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_ca_descriptor\n");
		dx = mpeg_ca_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_ca_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC ca_system_id:0x%04x ca_pid:0x%04x\n",
			dx->ca_system_id,
			dx->ca_pid);
		iprintf(indent, "DSC data:\n");
		hexdump(indent, "DSC ", mpeg_ca_descriptor_data(dx), mpeg_ca_descriptor_data_length(dx));
		break;
	}

	case dtag_mpeg_iso_639_language:
	{
		struct mpeg_iso_639_language_descriptor *dx;
		struct mpeg_iso_639_language_code *cur_lang;

		iprintf(indent, "DSC Decode mpeg_iso_639_language_descriptor\n");
		dx = mpeg_iso_639_language_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_iso_639_language_descriptor decode error\n");
			return;
		}
		mpeg_iso_639_language_descriptor_languages_for_each(dx, cur_lang) {
			iprintf(indent+1, "DSC language_code:%c%c%c audio_type:0x%02x\n",
				cur_lang->language_code[0],
				cur_lang->language_code[1],
				cur_lang->language_code[2],
				cur_lang->audio_type);
		}
		break;
	}

	case dtag_mpeg_system_clock:
	{
		struct mpeg_system_clock_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_system_clock_descriptor\n");
		dx = mpeg_system_clock_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_system_clock_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC external_clock_reference_indicator:%i clock_accuracy_integer:%i clock_accuracy_exponent:%i\n",
			dx->external_clock_reference_indicator,
			dx->clock_accuracy_integer,
		        dx->clock_accuracy_exponent);
		break;
	}

	case dtag_mpeg_multiplex_buffer_utilization:
	{
		struct mpeg_multiplex_buffer_utilization_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_multiplex_buffer_utilization_descriptor\n");
		dx = mpeg_multiplex_buffer_utilization_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_multiplex_buffer_utilization_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC bound_valid_flag:%i ltw_offset_lower_bound:%i ltw_offset_upper_bound:%i\n",
			dx->bound_valid_flag,
			dx->ltw_offset_lower_bound,
			dx->ltw_offset_upper_bound);
		break;
	}

	case dtag_mpeg_copyright:
	{
		struct mpeg_copyright_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_copyright_descriptor\n");
		dx = mpeg_copyright_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_copyright_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC copyright_identifier:0x%08x\n",
			dx->copyright_identifier);
		iprintf(indent, "DSC data:\n");
		hexdump(indent, "DSC ", mpeg_copyright_descriptor_data(dx), mpeg_copyright_descriptor_data_length(dx));
		break;
	}

	case dtag_mpeg_maximum_bitrate:
	{
		struct mpeg_maximum_bitrate_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_maximum_bitrate_descriptor\n");
		dx = mpeg_maximum_bitrate_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_maximum_bitrate_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC maximum_bitrate:%i\n",
			dx->maximum_bitrate);
		break;
	}

	case dtag_mpeg_private_data_indicator:
	{
		struct mpeg_private_data_indicator_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_private_data_indicator_descriptor\n");
		dx = mpeg_private_data_indicator_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_private_data_indicator_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC private_data_indicator:0x%x\n",
			dx->private_data_indicator);
		break;
	}

	case dtag_mpeg_smoothing_buffer:
	{
		struct mpeg_smoothing_buffer_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_smoothing_buffer_descriptor\n");
		dx = mpeg_smoothing_buffer_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_smoothing_buffer_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC sb_leak_rate:%i sb_size:%i\n",
			dx->sb_leak_rate,
		        dx->sb_size);
		break;
	}

	case dtag_mpeg_std:
	{
		struct mpeg_std_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_std_descriptor\n");
		dx = mpeg_std_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_std_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC leak_valid_flag:%i\n",
			dx->leak_valid_flag);
		break;
	}

	case dtag_mpeg_ibp:
	{
		struct mpeg_ibp_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_ibp_descriptor\n");
		dx = mpeg_ibp_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_ibp_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC closed_gop_flag:%i identical_gop_flag:%i max_gop_length:%i\n",
			dx->closed_gop_flag, dx->identical_gop_flag, dx->max_gop_length);
		break;
	}

	case dtag_mpeg_4_video:
	{
		struct mpeg4_video_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg4_video_descriptor\n");
		dx = mpeg4_video_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg4_video_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC mpeg4_visual_profile_and_level:0x%02x\n",
			dx->mpeg4_visual_profile_and_level);
		break;
	}

	case dtag_mpeg_4_audio:
	{
		struct mpeg4_audio_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg4_audio_descriptor\n");
		dx = mpeg4_audio_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg4_audio_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC mpeg4_audio_profile_and_level:0x%02x\n",
			dx->mpeg4_audio_profile_and_level);
		break;
	}

	case dtag_mpeg_iod:
	{
		struct mpeg_iod_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_iod_descriptor\n");
		dx = mpeg_iod_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_iod_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC scope_of_iod_label:0x%08x iod_label:0x%02x\n",
			dx->scope_of_iod_label, dx->iod_label);
		iprintf(indent, "DSC iod:\n");
		hexdump(indent, "DSC ", mpeg_iod_descriptor_iod(dx), mpeg_iod_descriptor_iod_length(dx));
		break;
	}

	case dtag_mpeg_sl:
	{
		struct mpeg_sl_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_sl_descriptor\n");
		dx = mpeg_sl_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_sl_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC es_id:0x%04x\n",
			dx->es_id);
		break;
	}

	case dtag_mpeg_fmc:
	{
		struct mpeg_fmc_descriptor *dx;
		struct mpeg_flex_mux *cur_fm;

		iprintf(indent, "DSC Decode mpeg_fmc_descriptor\n");
		dx = mpeg_fmc_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_fmc_descriptor_descriptor decode error\n");
			return;
		}
		mpeg_fmc_descriptor_muxes_for_each(dx, cur_fm) {
			iprintf(indent+1, "DSC es_id:0x%04x flex_mux_channel:0x%02x\n",
				cur_fm->es_id,
				cur_fm->flex_mux_channel);
		}
		break;
	}

	case dtag_mpeg_external_es_id:
	{
		struct mpeg_external_es_id_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_external_es_id_descriptor\n");
		dx = mpeg_external_es_id_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_external_es_id_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC external_es_id:0x%04x\n",
			dx->external_es_id);
		break;
	}

	case dtag_mpeg_muxcode:
	{
		struct mpeg_muxcode_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_muxcode_descriptor\n");
		dx = mpeg_muxcode_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_muxcode_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC entries:\n");
		hexdump(indent, "DSC ", mpeg_muxcode_descriptor_entries(dx), mpeg_muxcode_descriptor_entries_length(dx));
		break;
	}

	case dtag_mpeg_fmxbuffer_size:
	{
		struct mpeg_fmxbuffer_size_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_fmxbuffer_size_descriptor\n");
		dx = mpeg_fmxbuffer_size_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_fmxbuffer_size_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC descriptors:\n");
		hexdump(indent, "DSC ", mpeg_fmxbuffer_size_descriptor_descriptors(dx), mpeg_fmxbuffer_size_descriptor_descriptors_length(dx));
		break;
	}

	case dtag_mpeg_multiplex_buffer:
	{
		struct mpeg_multiplex_buffer_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_multiplex_buffer_descriptor\n");
		dx = mpeg_multiplex_buffer_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_multiplex_buffer_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC mb_buffer_size:%i tb_leak_rate:%i\n",
			dx->mb_buffer_size, dx->tb_leak_rate);
		break;
	}

	case dtag_dvb_network_name:
	case dtag_dvb_service_list:
	case dtag_dvb_stuffing:
	case dtag_dvb_satellite_delivery_system:
	case dtag_dvb_cable_delivery_system:
	case dtag_dvb_vbi_data:
	case dtag_dvb_vbi_teletext:
	case dtag_dvb_bouquet_name:
	case dtag_dvb_service:
	case dtag_dvb_country:
	case dtag_dvb_linkage:
	case dtag_dvb_nvod_reference:
	case dtag_dvb_time_shifted_service:
	case dtag_dvb_short_event:
	case dtag_dvb_extended_event:
	case dtag_dvb_time_shifted_event:
	case dtag_dvb_component:
	case dtag_dvb_mosaic:
	case dtag_dvb_stream_identifier:
	case dtag_dvb_ca_identifier:
	case dtag_dvb_content:
	case dtag_dvb_parental_rating:
	case dtag_dvb_teletext:
	case dtag_dvb_telephone:
	case dtag_dvb_local_time_offset:
	case dtag_dvb_subtitling:
	case dtag_dvb_terrestial_delivery_system:
	case dtag_dvb_multilingual_network_name:
	case dtag_dvb_multilingual_bouquet_name:
	case dtag_dvb_multilingual_service_name:
	case dtag_dvb_multilingual_component:
	case dtag_dvb_private_data_specifier:
	case dtag_dvb_service_move:
	case dtag_dvb_short_smoothing_buffer:
	case dtag_dvb_frequency_list:
	case dtag_dvb_partial_transport_stream:
	case dtag_dvb_data_broadcast:
	case dtag_dvb_data_broadcast_id:
	case dtag_dvb_transport_stream:
	case dtag_dvb_dsng:
	case dtag_dvb_pdc:
	case dtag_dvb_ac3:
	case dtag_dvb_ancillary_data:
	case dtag_dvb_cell_list:
	case dtag_dvb_cell_frequency_link:
	case dtag_dvb_announcement_support:
	case dtag_dvb_application_signalling:
	case dtag_dvb_adaption_field_data:
	case dtag_dvb_service_identifier:
	case dtag_dvb_service_availability:
	default:
		fprintf(stderr, "DSC XXXX Unknown descriptor_tag:0x%02x\n", d->tag);
		return;
	}
}

void iprintf(int indent, char *fmt, ...)
{
	va_list ap;

	while(indent--) {
		printf("\t");
	}

	va_start(ap, fmt);
	vprintf(fmt, ap);
	va_end(ap);
}

void hexdump(int indent, char *prefix, uint8_t *buf, int buflen)
{
	int i;
	int j;
	int max;
	char line[512];

	for(i=0; i< buflen; i+=16) {
		max = 16;
		if ((i + max) > buflen)
				max = buflen - i;

		memset(line, 0, sizeof(line));
		memset(line + 4 + 48 + 1, ' ', 16);
		sprintf(line, "%02x: ", i);
		for(j=0; j<max; j++) {
			sprintf(line + 4 + (j*3), "%02x", buf[i+j]);
			if ((buf[i+j] > 31) && (buf[i+j] < 127))
				line[4 + 48 + 1 + j] = buf[i+j];
			else
				line[4 + 48 + 1 + j] = '.';
		}

		for(j=0; j< 4 + 48;  j++) {
			if (!line[j])
				line[j] = ' ';
		}
		line[4+48] = '|';

		for(j=0; j < indent; j++) {
			printf("\t");
		}
		printf("%s%s|\n", prefix, line);
	}
}
