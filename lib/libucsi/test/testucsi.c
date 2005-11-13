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
#include <convert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <dvbdemux.h>

void parse_section(uint8_t *buf, int len, int pid);
void parse_descriptor(struct descriptor *d, int indent);

#define TIME_CHECK_VAL 1131835761
#define DURATION_CHECK_VAL 5643

int main(int argc, char *argv[])
{
	int demuxfd;
	int dvrfd;
	int adapter;
	unsigned char databuf[TRANSPORT_PACKET_LENGTH*20];
	int sz;
	int pid;
	int i;
	int used;
	int section_status;
	unsigned char continuities[TRANSPORT_MAX_PIDS];
	struct section_buf *section_bufs[TRANSPORT_MAX_PIDS];
	struct transport_packet *tspkt;
	struct transport_values tsvals;
	int pidlimit = -1;

	if ((argc < 2) || (argc > 3)) {
		fprintf(stderr, "Syntax: testucsi <adapter id> [<pid to limit to>]\n");
		exit(1);
	}
	adapter = atoi(argv[1]);
	if (argc == 3)
		sscanf(argv[2], "%i", &pidlimit);
	printf("Using adapter %i\n", adapter);
   
	// check the dvbdate conversion functions
	unixtime_to_dvbdate(TIME_CHECK_VAL, databuf);
	if (dvbdate_to_unixtime(databuf) != TIME_CHECK_VAL) {
		fprintf(stderr, "XXXX dvbdate function check failed (%i!=%i)\n",
			TIME_CHECK_VAL, (int) dvbdate_to_unixtime(databuf));
		exit(1);
	}
	seconds_to_dvbduration(DURATION_CHECK_VAL, databuf);
	if (dvbduration_to_seconds(databuf) != DURATION_CHECK_VAL) {
		fprintf(stderr, "XXXX dvbduration function check failed (%i!=%i)\n",
			DURATION_CHECK_VAL, (int) dvbduration_to_seconds(databuf));
		exit(1);
	}

	printf("dvbdate function checks passed\n");

	// open devices
	if ((demuxfd = dvbdemux_open_demux(adapter, 0)) < 0) {
		perror("demux");
		exit(1);
	}
	if ((dvrfd = dvbdemux_open_dvr(adapter, 0, 1)) < 0) {
		perror("dvr");
		exit(1);
	}

	// make the buffer a bit larger
	if (dvbdemux_set_buffer(demuxfd, 1024*1024)) {
		perror("set buffer");
		exit(1);
	}

	// setup filter to capture stuff
	if (dvbdemux_set_pid_filter(demuxfd, pidlimit, DVBDEMUX_INPUT_FRONTEND, DVBDEMUX_OUTPUT_DVR, 1)) {
		perror("set pid filter");
		exit(1);
	}

	// process the data
	memset(continuities, 0, sizeof(continuities));
	memset(section_bufs, 0, sizeof(section_bufs));
	while(1) {
		if ((sz = read(dvrfd, databuf, sizeof(databuf))) < 0) {
			if (sz == -EOVERFLOW) {
				fprintf(stderr, "data overflow!\n");
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
	//				fprintf(stderr, "XXXX bad section %04x %i\n",pid, section_status);
					section_buf_reset(section_bufs[pid]);
				}
			}
		}
	}

	return 0;
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
		printf("pcr_pid:0x%02x\n", pmt->pcr_pid);
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
		for(index=0; index<objects_length; index++) {
			printf("\t0x%04x: 0x%02x\n", index, objects[index]);
		}
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
		printf("original_network_id:0x%04x\n", sdt->original_network_id);
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
		struct dvb_int_section_target_loop_entry *cur_loop_entry;
		struct dvb_int_section_operational_loop *operational_loop;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("Decode INT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((_int = dvb_int_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX INT section decode error\n");
			return;
		}
		printf("action_type:0x%02x platform_id_hash:0x%02x version_number:%i current_next_indicator:%i section_number:0x%02x last_section_number:0x%02x platform_id:0x%06x processing_order:0x%02x\n",
		       _int->action_type,
		       _int->platform_id_hash,
		       _int->version_number,
		       _int->current_next_indicator,
		       _int->section_number,
		       _int->last_section_number,
		       _int->platform_id,
		       _int->processing_order);
		dvb_int_section_platform_descriptors_for_each(_int, curd) {
			parse_descriptor(curd, 1);
		}
		dvb_int_section_target_loop_for_each(_int, cur_loop_entry) {
			dvb_int_section_target_loop_entry_target_descriptors_for_each(cur_loop_entry, curd) {
				parse_descriptor(curd, 2);
			}
			operational_loop = dvb_int_section_target_loop_entry_operational_loop(cur_loop_entry);
			dvb_int_section_operational_loop_operational_descriptors_for_each(operational_loop, curd) {
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
		printf("transport_stream_id:0x%04x original_network_id:0x%04x segment_last_section_number:0x%02x last_table_id:0x%02x\n",
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
		return;
	}

	printf("\n");
}

void parse_descriptor(struct descriptor *d, int indent)
{
	switch(d->tag) {
	case dtag_mpeg_video_stream:
	case dtag_mpeg_audio_stream:
	case dtag_mpeg_hierarchy:
	case dtag_mpeg_registration:
	case dtag_mpeg_data_stream_alignment:
	case dtag_mpeg_target_background_grid:
	case dtag_mpeg_video_window:
	case dtag_mpeg_ca:
	case dtag_mpeg_iso_639_language:
	case dtag_mpeg_system_clock:
	case dtag_mpeg_multiplex_buffer_utilization:
	case dtag_mpeg_copyright:
	case dtag_mpeg_maximum_bitrate:
	case dtag_mpeg_private_data_indicator:
	case dtag_mpeg_smoothing_buffer:
	case dtag_mpeg_std:
	case dtag_mpeg_ibp:
	case dtag_mpeg_4_video:
	case dtag_mpeg_4_audio:
	case dtag_mpeg_iod:
	case dtag_mpeg_sl:
	case dtag_mpeg_fmc:
	case dtag_mpeg_external_es_id:
	case dtag_mpeg_muxcode:
	case dtag_mpeg_fmxbuffer_size:
	case dtag_mpeg_multiplex_buffer:
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
		fprintf(stderr, "XXXX Unknown descriptor_tag:0x%02x\n", d->tag);
		return;
	}
}
