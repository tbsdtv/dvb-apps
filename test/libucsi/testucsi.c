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

#include <libucsi/mpeg/descriptor.h>
#include <libucsi/mpeg/section.h>
#include <libucsi/dvb/descriptor.h>
#include <libucsi/dvb/section.h>
#include <libucsi/transport_packet.h>
#include <libucsi/section_buf.h>
#include <libucsi/dvb/types.h>
#include <libdvbapi/dvbdemux.h>
#include <libdvbapi/dvbfe.h>
#include <libdvbcfg/dvbcfg_zapchannel.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>

void receive_data(int dvrfd, int timeout);
void parse_section(uint8_t *buf, int len, int pid);
void parse_descriptor(struct descriptor *d, int indent);
void iprintf(int indent, char *fmt, ...);
void hexdump(int indent, char *prefix, uint8_t *buf, int buflen);
int zapchannels_cb(void *private, struct dvbcfg_zapchannel *channel);

#define TIME_CHECK_VAL 1131835761
#define DURATION_CHECK_VAL 5643

#define MAX_TUNE_TIME 3000
#define MAX_DUMP_TIME 60

struct dvbfe_handle *fe;
struct dvbfe_info feinfo;
int demuxfd;
int dvrfd;

int main(int argc, char *argv[])
{
	int adapter;
	char *channelsfile;
	int pidlimit = -1;
	dvbdate_t dvbdate;
	dvbduration_t dvbduration;

	// process arguments
	if ((argc < 3) || (argc > 4)) {
		fprintf(stderr, "Syntax: testucsi <adapter id> <channels file> [<pid to limit to>]\n");
		exit(1);
	}
	adapter = atoi(argv[1]);
	channelsfile = argv[2];
	if (argc == 4)
		sscanf(argv[3], "%i", &pidlimit);
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

	// process all the channels
	FILE *channels = fopen(channelsfile, "r");
	if (channels == NULL) {
		fprintf(stderr, "Unable to open %s\n", channelsfile);
		exit(1);
	}
        dvbcfg_zapchannel_load(channels, NULL, zapchannels_cb);
        return 0;
}

int zapchannels_cb(void *private, struct dvbcfg_zapchannel *channel)
{
        (void) private;

        if (feinfo.type == DVBFE_TYPE_DVBS) {
                // FIXME: do diseqc
        }

        printf("Tuning to: %i %i %i\n",
                channel->fe_params.frequency,
                channel->fe_params.u.dvbs.symbol_rate,
                channel->fe_params.u.dvbs.fec_inner);
        if (dvbfe_set(fe, &channel->fe_params, MAX_TUNE_TIME)) {
                fprintf(stderr, "Failed to lock!\n");
        } else {
                printf("Tuned successfully!\n");
                receive_data(dvrfd, MAX_DUMP_TIME);
        }

	return 0;
}

void receive_data(int _dvrfd, int timeout)
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
		if ((sz = read(_dvrfd, databuf, sizeof(databuf))) < 0) {
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
		printf("SCT Decode PAT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((pat = mpeg_pat_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "SCT XXXX PAT section decode error\n");
			return;
		}
		printf("SCT transport_stream_id:0x%04x\n", mpeg_pat_section_transport_stream_id(pat));
		mpeg_pat_section_programs_for_each(pat, cur) {
			printf("\tSCT program_number:0x%04x pid:0x%04x\n", cur->program_number, cur->pid);
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
		printf("SCT Decode CAT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((cat = mpeg_cat_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "SCT XXXX CAT section decode error\n");
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
		printf("SCT Decode PMT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((pmt = mpeg_pmt_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "SCT XXXX PMT section decode error\n");
			return;
		}
		printf("SCT program_number:0x%04x pcr_pid:0x%02x\n", mpeg_pmt_section_program_number(pmt), pmt->pcr_pid);
		mpeg_pmt_section_descriptors_for_each(pmt, curd) {
			parse_descriptor(curd, 1);
		}
		mpeg_pmt_section_streams_for_each(pmt, cur_stream) {
			printf("\tSCT stream_type:0x%02x pid:0x%04x\n", cur_stream->stream_type, cur_stream->pid);
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
		printf("SCT Decode TSDT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((tsdt = mpeg_tsdt_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "SCT XXXX TSDT section decode error\n");
			return;
		}
		mpeg_tsdt_section_descriptors_for_each(tsdt, curd) {
			parse_descriptor(curd, 1);
		}
		break;
	}

	case stag_mpeg_metadata:
	{
		struct mpeg_metadata_section *metadata;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("SCT Decode metadata (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((metadata = mpeg_metadata_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "SCT XXXX metadata section decode error\n");
			return;
		}
		printf("SCT random_access_indicator:%i decoder_config_flag:%i fragment_indicator:%i service_id:%02x\n",
		       mpeg_metadata_section_random_access_indicator(metadata),
		       mpeg_metadata_section_decoder_config_flag(metadata),
		       mpeg_metadata_section_fragment_indicator(metadata),
		       mpeg_metadata_section_service_id(metadata));
		hexdump(0, "SCT ", mpeg_metadata_section_data(metadata), mpeg_metadata_section_data_length(metadata));
		break;
	}

	case stag_mpeg_iso14496_scene_description:
	case stag_mpeg_iso14496_object_description:
	{
		struct mpeg_odsmt_section *odsmt;
		struct mpeg_odsmt_stream *cur_stream;
		struct descriptor *curd;
		int _index;
		uint8_t *objects;
		size_t objects_length;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("SCT Decode ISO14496 (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((odsmt = mpeg_odsmt_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX ISO14496 section decode error\n");
			return;
		}
		printf("SCT PID:0x%04x\n", mpeg_odsmt_section_pid(odsmt));
		mpeg_odsmt_section_streams_for_each(osdmt, cur_stream, _index) {
			if (odsmt->stream_count == 0) {
				printf("\tSCT SINGLE 0x%04x\n", cur_stream->u.single.esid);
			} else {
				printf("\tSCT MULTI 0x%04x 0x%02x\n", cur_stream->u.multi.esid, cur_stream->u.multi.fmc);
			}
			mpeg_odsmt_stream_descriptors_for_each(osdmt, cur_stream, curd) {
				parse_descriptor(curd, 2);
			}
		}
		objects = mpeg_odsmt_section_object_descriptors(odsmt, &objects_length);
		if (objects == NULL) {
			printf("SCT XXXX OSDMT parse error\n");
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
		printf("SCT Decode NIT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((nit = dvb_nit_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "SCT XXXX NIT section decode error\n");
			return;
		}
		printf("SCT network_id:0x%04x\n", dvb_nit_section_network_id(nit));
		dvb_nit_section_descriptors_for_each(nit, curd) {
			parse_descriptor(curd, 1);
		}
		part2 = dvb_nit_section_part2(nit);
		dvb_nit_section_transports_for_each(nit, part2, cur_transport) {
			printf("\tSCT transport_stream_id:0x%04x original_network_id:0x%04x\n", cur_transport->transport_stream_id, cur_transport->original_network_id);
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
		printf("SCT Decode SDT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((sdt = dvb_sdt_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX SDT section decode error\n");
			return;
		}
		printf("SCT transport_stream_id:0x%04x original_network_id:0x%04x\n", dvb_sdt_section_transport_stream_id(sdt), sdt->original_network_id);
		dvb_sdt_section_services_for_each(sdt, cur_service) {
			printf("\tSCT service_id:0x%04x eit_schedule_flag:%i eit_present_following_flag:%i running_status:%i free_ca_mode:%i\n",
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
		printf("SCT Decode BAT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((bat = dvb_bat_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "SCT XXXX BAT section decode error\n");
			return;
		}
		printf("SCT bouquet_id:0x%04x\n", dvb_bat_section_bouquet_id(bat));
		dvb_bat_section_descriptors_for_each(bat, curd) {
			parse_descriptor(curd, 1);
		}
		part2 = dvb_bat_section_part2(bat);
		dvb_bat_section_transports_for_each(part2, cur_transport) {
			printf("\tSCT transport_stream_id:0x%04x original_network_id:0x%04x\n",
			       cur_transport->transport_stream_id,
			       cur_transport->original_network_id);
			dvb_bat_transport_descriptors_for_each(cur_transport, curd) {
				parse_descriptor(curd, 2);
			}
		}
		break;
	}

	case stag_dvb_update_notification:
	case stag_dvb_ip_mac_notification:
	{
		struct dvb_int_section *_int;
		struct descriptor *curd;
		struct dvb_int_target *cur_target;
		struct dvb_int_operational_loop *operational_loop;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("SCT Decode INT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((_int = dvb_int_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX INT section decode error\n");
			return;
		}
		printf("SCT action_type:0x%02x platform_id_hash:0x%02x platform_id:0x%06x processing_order:0x%02x\n",
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
		printf("SCT Decode EIT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((eit = dvb_eit_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "XXXX EIT section decode error\n");
			return;
		}
		printf("SCT service_id:0x%04x transport_stream_id:0x%04x original_network_id:0x%04x segment_last_section_number:0x%02x last_table_id:0x%02x\n",
		       dvb_eit_section_service_id(eit),
		       eit->transport_stream_id,
		       eit->original_network_id,
		       eit->segment_last_section_number,
		       eit->last_table_id);
		dvb_eit_section_events_for_each(eit, cur_event) {
			start_time = dvbdate_to_unixtime(cur_event->start_time);
			printf("\tSCT event_id:0x%04x duration:%i running_status:%i free_ca_mode:%i start_time:%i -- %s",
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

		printf("SCT Decode TDT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((tdt = dvb_tdt_section_codec(section)) == NULL) {
			fprintf(stderr, "XXXX TDT section decode error\n");
			return;
		}
		dvbtime = dvbdate_to_unixtime(tdt->utc_time);
		printf("SCT Time: %i -- %s", (int) dvbtime, ctime(&dvbtime));
		break;
	}

	case stag_dvb_running_status:
	{
		struct dvb_rst_section *rst;
		struct dvb_rst_status *cur_status;

		printf("SCT Decode RST (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((rst = dvb_rst_section_codec(section)) == NULL) {
			fprintf(stderr, "SCT XXXX RST section decode error\n");
			return;
		}
		dvb_rst_section_statuses_for_each(rst, cur_status) {
			printf("\tSCT transport_stream_id:0x%04x original_network_id:0x%04x service_id:0x%04x event_id:0x%04x running_status:%i\n",
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

		printf("SCT Decode ST (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((st = dvb_st_section_codec(section)) == NULL) {
			fprintf(stderr, "SCT XXXX ST section decode error\n");
			return;
		}
		printf("SCT Length: %i\n", dvb_st_section_data_length(st));
		break;
	}

	case stag_dvb_time_offset:
	{
		struct dvb_tot_section *tot;
		struct descriptor *curd;
		time_t dvbtime;

		if (section_check_crc(section))
			return;
		printf("SCT Decode TOT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((tot = dvb_tot_section_codec(section)) == NULL) {
			fprintf(stderr, "SCT XXXX TOT section decode error\n");
			return;
		}
		dvbtime = dvbdate_to_unixtime(tot->utc_time);
		printf("SCT utc_time: %i -- %s", (int) dvbtime, ctime(&dvbtime));
		dvb_tot_section_descriptors_for_each(tot, curd) {
			parse_descriptor(curd, 1);
		}
		break;
	}

	case stag_dvb_tva_container:
	{
		struct dvb_tva_container_section *tva;

		if ((section_ext = section_ext_decode(section, 1)) == NULL) {
			return;
		}
		printf("SCT Decode tva (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((tva = dvb_tva_container_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "SCT XXXX tva section decode error\n");
			return;
		}
		printf("SCT container_id:%04x\n",
		       dvb_tva_container_section_container_id(tva));
		hexdump(0, "SCT ", dvb_tva_container_section_data(tva), dvb_tva_container_section_data_length(tva));
		break;
	}

	case stag_dvb_discontinuity_information:
	{
		struct dvb_dit_section *dit;

		printf("SCT Decode DIT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((dit = dvb_dit_section_codec(section)) == NULL) {
			fprintf(stderr, "SCT XXXX DIT section decode error\n");
			return;
		}
		printf("SCT transition_flag:%i\n", dit->transition_flag);
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
		printf("SCT Decode SIT (pid:0x%04x) (table:0x%02x)\n", pid, section->table_id);
		if ((sit = dvb_sit_section_codec(section_ext)) == NULL) {
			fprintf(stderr, "SCT XXXX SIT section decode error\n");
			return;
		}
		dvb_sit_section_descriptors_for_each(sit, curd) {
			parse_descriptor(curd, 1);
		}
		dvb_sit_section_services_for_each(sit, cur_service) {
			printf("\tSCT service_id:0x%04x running_status:%i\n", cur_service->service_id, cur_service->running_status);
			dvb_sit_service_descriptors_for_each(cur_service, curd) {
				parse_descriptor(curd, 2);
			}
		}
		break;
	}

	default:
		fprintf(stderr, "SCT XXXX Unknown table_id:0x%02x (pid:0x%04x)\n", section->table_id, pid);
		hexdump(0, "SCT ", buf, len);
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
			iprintf(indent+1, "DSC language_code:%.3s audio_type:0x%02x\n",
				cur_lang->language_code,
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

	case dtag_mpeg_content_labelling:
	{
		struct mpeg_content_labelling_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_content_labelling_descriptor\n");
		dx = mpeg_content_labelling_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_content_labelling_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC metadata_application_format:%04x\n",
			dx->metadata_application_format);
		struct mpeg_content_labelling_descriptor_application_format_identifier *id =
			mpeg_content_labelling_descriptor_id(dx);
		if (id != NULL) {
			iprintf(indent, "DSC application_format_id:%04x\n",
				id->id);
		}
		struct mpeg_content_labelling_descriptor_flags *flags =
			mpeg_content_labelling_descriptor_flags(dx);
		if (flags != NULL) {
			iprintf(indent, "DSC content_reference_id_record_flag:%i content_time_base_indicator:%02x\n",
				flags->content_reference_id_record_flag,
				flags->content_time_base_indicator);

			struct mpeg_content_labelling_descriptor_reference_id *reference_id =
				mpeg_content_labelling_descriptor_reference_id(flags);
			if (reference_id != NULL) {
				hexdump(indent, "DSC reference_id " ,
					mpeg_content_reference_id_data(reference_id),
					reference_id->content_reference_id_record_length);
			}

			struct mpeg_content_labelling_descriptor_time_base *time_base =
				mpeg_content_labelling_descriptor_time_base(flags);
			if (time_base != NULL) {
				iprintf(indent, "DSC time_base content_time_base_value:%lli metadata_time_base_value:%lli\n",
					time_base->content_time_base_value,
					time_base->metadata_time_base_value);
			}

			struct mpeg_content_labelling_descriptor_content_id *content_id =
				mpeg_content_labelling_descriptor_content_id(flags);
			if (content_id != NULL) {
				iprintf(indent, "DSC content_id contentId:%i\n",
					content_id->contentId);
			}

			struct mpeg_content_labelling_descriptor_time_base_association *time_base_assoc =
				mpeg_content_labelling_descriptor_time_base_assoc(flags);
			if (time_base_assoc != NULL) {
				hexdump(indent, "DSC time_base_assoc" ,
					mpeg_time_base_association_data(time_base_assoc),
					time_base_assoc->time_base_association_data_length);
			}

			uint8_t *priv;
			int priv_length;
			priv = mpeg_content_labelling_descriptor_data(dx, flags, &priv_length);
			hexdump(indent, "DSC private_data", priv, priv_length);
		}

		break;
	}

	case dtag_mpeg_metadata_pointer:
	{
		struct mpeg_metadata_pointer_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_metadata_pointer_descriptor\n");
		dx = mpeg_metadata_pointer_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_metadata_pointer_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC metadata_application_format:%04x\n",
			dx->metadata_application_format);

		struct mpeg_metadata_pointer_descriptor_application_format_identifier *id =
			mpeg_metadata_pointer_descriptor_appid(dx);
		if (id != NULL) {
			iprintf(indent, "DSC application_format_id:%04x\n",
				id->id);
		}

		struct mpeg_metadata_pointer_descriptor_format_identifier *did =
			mpeg_metadata_pointer_descriptor_formid(dx);
		if (did != NULL) {
			iprintf(indent, "DSC mpeg_metadata_pointer_descriptor_format_id:%04x\n",
				did->id);
		}

		struct mpeg_metadata_pointer_descriptor_flags *flags =
			mpeg_metadata_pointer_descriptor_flags(dx);
		if (flags != NULL) {
			iprintf(indent, "DSC metadata_service_id:%i metadata_locator_record_flag:%i mpeg_carriage_flags:%x\n",
				flags->metadata_service_id,
				flags->metadata_locator_record_flag,
				flags->mpeg_carriage_flags);

			struct mpeg_metadata_pointer_descriptor_locator *locator =
				mpeg_metadata_pointer_descriptor_locator(flags);
			if (locator != NULL) {
				hexdump(indent, "DSC locator" ,
					mpeg_metadata_pointer_descriptor_locator_data(locator),
					locator->metadata_locator_record_length);
			}

			struct mpeg_metadata_pointer_descriptor_program_number *pnum=
				mpeg_metadata_pointer_descriptor_program_number(flags);
			if (pnum != NULL) {
				iprintf(indent, "DSC program_number number:%04x\n",
					pnum->number);
			}

			struct mpeg_metadata_pointer_descriptor_carriage *carriage =
				mpeg_metadata_pointer_descriptor_carriage(flags);
			if (carriage != NULL) {
				iprintf(indent, "DSC carriage transport_stream_location:%04x transport_stream_id:%04x\n",
					carriage->transport_stream_location,
					carriage->transport_stream_id);
			}

			uint8_t *priv;
			int priv_length;
			priv = mpeg_metadata_pointer_descriptor_private_data(dx, flags, &priv_length);
			hexdump(indent, "DSC private_data" , priv, priv_length);
		}

		break;
	}

	case dtag_mpeg_metadata:
	{
		struct mpeg_metadata_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_metadata_descriptor\n");
		dx = mpeg_metadata_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_metadata_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC metadata_application_format:%04x\n",
			dx->metadata_application_format);

		struct mpeg_metadata_descriptor_application_format_identifier *id =
			mpeg_metadata_descriptor_appid(dx);
		if (id != NULL) {
			iprintf(indent, "DSC application_format_id:%04x\n",
				id->id);
		}

		struct mpeg_metadata_descriptor_format_identifier *did =
			mpeg_metadata_descriptor_formid(dx);
		if (did != NULL) {
			iprintf(indent, "DSC mpeg_metadata_descriptor_format_id:%04x\n",
				did->id);
		}

		struct mpeg_metadata_descriptor_flags *flags =
			mpeg_metadata_descriptor_flags(dx);
		if (flags != NULL) {
			iprintf(indent, "DSC metadata_service_id:%i decoder_config_flags:%i dsm_cc_flag:%x\n",
				flags->metadata_service_id,
				flags->decoder_config_flags,
				flags->dsm_cc_flag);

			struct mpeg_metadata_descriptor_service_identifier *serviceid=
				mpeg_metadata_descriptor_sevice_identifier(flags);
			if (serviceid != NULL) {
				hexdump(indent, "DSC service_id" ,
					mpeg_metadata_descriptor_service_identifier_data(serviceid),
					serviceid->service_identification_length);
			}

			struct mpeg_metadata_descriptor_decoder_config *dconfig=
				mpeg_metadata_descriptor_decoder_config(flags);
			if (dconfig != NULL) {
				hexdump(indent, "DSC decoder_config" ,
					mpeg_metadata_descriptor_decoder_config_data(dconfig),
					dconfig->decoder_config_length);
			}

			struct mpeg_metadata_descriptor_decoder_config_id_record *dconfigid=
				mpeg_metadata_descriptor_decoder_config_id_record(flags);
			if (dconfigid != NULL) {
				hexdump(indent, "DSC decoder_config" ,
					mpeg_metadata_descriptor_decoder_config_id_record_data(dconfigid),
					dconfigid->decoder_config_id_record_length);
			}

			struct mpeg_metadata_descriptor_decoder_config_service_id *dserviceid=
				mpeg_metadata_descriptor_decoder_config_service_id(flags);
			if (dserviceid != NULL) {
				iprintf(indent, "DSC decoder config service_id:%04x\n",
					dserviceid->decoder_config_metadata_service_id);
			}

			struct mpeg_metadata_descriptor_decoder_config_reserved *reserved=
				mpeg_metadata_descriptor_decoder_config_reserved(flags);
			if (reserved != NULL) {
				hexdump(indent, "DSC reserved" ,
					mpeg_metadata_descriptor_decoder_config_reserved_data(reserved),
 					reserved->reserved_data_length);
			}

			uint8_t *priv;
			int priv_length;
			priv = mpeg_metadata_descriptor_private_data(dx, flags, &priv_length);
			hexdump(indent, "DSC private_data" , priv, priv_length);
		}

		break;
	}

	case dtag_mpeg_metadata_std:
	{
		struct mpeg_metadata_std_descriptor *dx;

		iprintf(indent, "DSC Decode mpeg_metadata_std_descriptor\n");
		dx = mpeg_metadata_std_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX mpeg_metadata_std_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC metadata_input_leak_rate:%i metadata_buffer_size:%i metadata_output_leak_rate:%i\n",
			dx->metadata_input_leak_rate,
			dx->metadata_buffer_size,
			dx->metadata_output_leak_rate);
		break;
	}



	case dtag_dvb_network_name:
	{
		struct dvb_network_name_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_network_name_descriptor\n");
		dx = dvb_network_name_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_network_name_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC name:%.*s\n",
			dvb_network_name_descriptor_name_length(dx),
			dvb_network_name_descriptor_name(dx));
		break;
	}

	case dtag_dvb_service_list:
	{
		struct dvb_service_list_descriptor *dx;
		struct dvb_service_list_service *curs;

		iprintf(indent, "DSC Decode dvb_service_list_descriptor\n");
		dx = dvb_service_list_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_service_list_descriptor decode error\n");
			return;
		}
		dvb_service_list_descriptor_services_for_each(dx, curs) {
			iprintf(indent+1, "DSC service_id:0x%04x service_type:0x%02x\n",
				curs->service_id, curs->service_type);
		}
		break;
	}

	case dtag_dvb_stuffing:
	{
		struct dvb_stuffing_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_stuffing_descriptor\n");
		dx = dvb_stuffing_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_stuffing_descriptor decode error\n");
			return;
		}
		hexdump(indent, "DSC",
			dvb_stuffing_descriptor_data(dx),
			dvb_stuffing_descriptor_data_length(dx));
		break;
	}

	case dtag_dvb_satellite_delivery_system:
	{
		struct dvb_satellite_delivery_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_satellite_delivery_descriptor\n");
		dx = dvb_satellite_delivery_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_satellite_delivery_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC frequency:%i orbital_position:%i west_east:%i polarization:%i roll_off:%i modulation_system:%i modulation_type: %i symbol_rate:%i fec_inner:%i\n",
			dx->frequency,
			dx->orbital_position,
			dx->west_east_flag,
			dx->polarization,
			dx->roll_off,
			dx->modulation_system,
			dx->modulation_type,
			dx->symbol_rate,
			dx->fec_inner);
		break;
	}

	case dtag_dvb_cable_delivery_system:
	{
		struct dvb_cable_delivery_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_cable_delivery_descriptor\n");
		dx = dvb_cable_delivery_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_cable_delivery_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC frequency:%i fec_outer:%i modulation:%i symbol_rate:%i fec_inner:%i\n",
			dx->frequency, dx->fec_outer, dx->modulation,
			dx->symbol_rate, dx->fec_inner);
		break;
	}

	case dtag_dvb_vbi_data:
	{
		struct dvb_vbi_data_descriptor *dx;
		struct dvb_vbi_data_entry *cur;
		struct dvb_vbi_data_x *curx;

		iprintf(indent, "DSC Decode dvb_vbi_data_descriptor\n");
		dx = dvb_vbi_data_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_vbi_data_descriptor decode error\n");
			return;
		}
		dvb_vbi_data_descriptor_entries_for_each(dx, cur) {
			curx = dvb_vbi_data_entry_data_x(cur);
			iprintf(indent+1, "DSC data_service_id:0x%04x\n", cur->data_service_id);
			if (cur == NULL) {
				hexdump(indent+1, "DSC", dvb_vbi_data_entry_data(cur), cur->data_length);
			} else {
				iprintf(indent+1, "DSC field_parity:%i line_offset:%i\n",
					curx->field_parity, curx->line_offset);
			}
		}
		break;
	}

	case dtag_dvb_vbi_teletext:
	{
		struct dvb_vbi_teletext_descriptor *dx;
		struct dvb_vbi_teletext_entry *cur;

		iprintf(indent, "DSC Decode dvb_vbi_teletext_descriptor\n");
		dx = dvb_vbi_teletext_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_vbi_teletext_descriptor decode error\n");
			return;
		}
		dvb_vbi_teletext_descriptor_entries_for_each(dx, cur) {
			iprintf(indent+1, "DSC language_code:%.3s type:%i magazine_number:%i page_number:%i\n",
				cur->language_code,
				cur->type, cur->magazine_number, cur->page_number);
		}
		break;
	}

	case dtag_dvb_bouquet_name:
	{
		struct dvb_bouquet_name_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_bouquet_name_descriptor\n");
		dx = dvb_bouquet_name_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_bouquet_name_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC name:%.*s\n",
			dvb_bouquet_name_descriptor_name_length(dx),
			dvb_bouquet_name_descriptor_name(dx));
		break;
	}

	case dtag_dvb_service:
	{
		struct dvb_service_descriptor *dx;
		struct dvb_service_descriptor_part2 *part2;

		iprintf(indent, "DSC Decode dvb_service_descriptor\n");
		dx = dvb_service_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_service_descriptor decode error\n");
			return;
		}
		part2 = dvb_service_descriptor_part2(dx);
		iprintf(indent, "DSC service_type provider_name:%.*s service_name:%.*s\n",
			dx->service_provider_name_length,
			dvb_service_descriptor_service_provider_name(dx),
			part2->service_name_length,
			dvb_service_descriptor_service_name(part2));
		break;
	}

	case dtag_dvb_country_availability:
	{
		struct dvb_country_availability_descriptor *dx;
		struct dvb_country_availability_entry *cur;

		iprintf(indent, "DSC Decode dvb_country_availability_descriptor\n");
		dx = dvb_country_availability_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_country_availability_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC country_availability_flag:%i\n", dx->country_availability_flag);
		dvb_country_availability_descriptor_countries_for_each(dx, cur) {
			iprintf(indent+1, "DSC country_code:%.3s\n", cur->country_code);
		}
		break;
	}

	case dtag_dvb_linkage:
	{
		struct dvb_linkage_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_linkage_descriptor\n");
		dx = dvb_linkage_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_linkage_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC transport_stream_id:0x%04x original_network_id:0x%04x service_id:0x%04x linkage_type:0x%02x\n",
			dx->transport_stream_id, dx->original_network_id, dx->service_id, dx->linkage_type);
		switch(dx->linkage_type) {
		case 0x08:
		{
			struct dvb_linkage_data_08 *d08 = dvb_linkage_data_08(dx);
			int network_id = dvb_linkage_data_08_network_id(dx, d08);
			int initial_service_id = dvb_linkage_data_08_initial_service_id(dx, d08);
			int length = 0;
			uint8_t *data;

			data = dvb_linkage_data_08_data(dx, d08, &length);
			iprintf(indent, "DSC hand_over_type:%i origin_type:%i\n",
				d08->hand_over_type, d08->origin_type);
			if (network_id != -1) {
				iprintf(indent, "DSC network_id:0x%04x\n", network_id);
			}
			if (initial_service_id != -1) {
				iprintf(indent, "DSC initial_service_id:0x%04x\n", initial_service_id);
			}
			hexdump(indent+1, "DSC", data, length);
			break;
		}

		case 0x0b:
		{
			struct dvb_linkage_data_0b *data = dvb_linkage_data_0b(dx);
			struct dvb_platform_id *platid;
			struct dvb_platform_name *curplatname;

			dvb_dvb_linkage_data_0b_platform_id_for_each(data, platid) {
				iprintf(indent+1, "DSC platform_id:0x%06x\n", platid->platform_id);
				dvb_platform_id_platform_name_for_each(platid, curplatname) {
					iprintf(indent+2, "DSC language_code:%.3s platform_name:%.*s\n",
						curplatname->language_code,
						curplatname->platform_name_length, dvb_platform_name_text(curplatname));
				}
			}
			break;
		}

		case 0x0c:
		{
			struct dvb_linkage_data_0c *data = dvb_linkage_data_0c(dx);

			iprintf(indent, "DSC table_type:0x%02x\n", data->table_type);
			if (dvb_linkage_data_0c_bouquet_id(data)) {
				iprintf(indent, "DSC bouquet_id:0x%04x\n",
					dvb_linkage_data_0c_bouquet_id(data));
			}
			break;
		}

		default:
			hexdump(indent+1, "DSC", dvb_linkage_descriptor_data(dx), dvb_linkage_descriptor_data_length(dx));
			break;
		}
		break;
	}

	case dtag_dvb_nvod_reference:
	{
		struct dvb_nvod_reference_descriptor *dx;
		struct dvb_nvod_reference *cur;

		iprintf(indent, "DSC Decode dvb_nvod_reference_descriptor\n");
		dx = dvb_nvod_reference_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_nvod_reference_descriptor decode error\n");
			return;
		}
		dvb_nvod_reference_descriptor_references_for_each(dx, cur) {
			iprintf(indent+1, "DSC transport_stream_id:0x%04x original_network_id:0x%04x service_id:0x%04x\n",
				cur->transport_stream_id, cur->original_network_id,
				cur->service_id);
		}
		break;
	}

	case dtag_dvb_time_shifted_service:
	{
		struct dvb_time_shifted_service_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_time_shifted_service_descriptor\n");
		dx = dvb_time_shifted_service_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_time_shifted_service_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC reference_service_id:0x%04x\n", dx->reference_service_id);
		break;
	}

	case dtag_dvb_short_event:
	{
		struct dvb_short_event_descriptor *dx;
		struct dvb_short_event_descriptor_part2 *part2;

		iprintf(indent, "DSC Decode dvb_short_event_descriptor\n");
		dx = dvb_short_event_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_short_event_descriptor decode error\n");
			return;
		}
		part2 = dvb_short_event_descriptor_part2(dx);
		iprintf(indent, "DSC language_code:%3s event_name:%.*s text:%.*s\n",
			dx->language_code,
			dx->event_name_length, dvb_short_event_descriptor_event_name(dx),
			part2->text_length, dvb_short_event_descriptor_text(part2));
		break;
	}

	case dtag_dvb_extended_event:
	{
		struct dvb_extended_event_descriptor *dx;
		struct dvb_extended_event_descriptor_part2 *part2;
		struct dvb_extended_event_item *cur;

		iprintf(indent, "DSC Decode dvb_extended_event_descriptor\n");
		dx = dvb_extended_event_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_extended_event_descriptor decode error\n");
			return;
		}
		part2 = dvb_extended_event_descriptor_part2(dx);
		iprintf(indent, "DSC descriptor_number:%i last_descriptor_number:%i language_code:%.3s text:%.*s\n",
			dx->descriptor_number, dx->last_descriptor_number,
			dx->language_code,
			part2->text_length, dvb_extended_event_descriptor_part2_text(part2));
		dvb_extended_event_descriptor_items_for_each(dx, cur) {
			struct dvb_extended_event_item_part2 *ipart2 =
				dvb_extended_event_item_part2(cur);
			iprintf(indent+1, "DSC description:%.*s item:%.*s\n",
				cur->item_description_length, dvb_extended_event_item_description(cur),
				ipart2->item_length, dvb_extended_event_item_part2_item(ipart2));
		}
		break;
	}

	case dtag_dvb_time_shifted_event:
	{
		struct dvb_time_shifted_event_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_time_shifted_event_descriptor\n");
		dx = dvb_time_shifted_event_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_time_shifted_event_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC reference_service_id:0x%04x reference_event_id:0x%04x\n",
			dx->reference_service_id, dx->reference_event_id);
		break;
	}

	case dtag_dvb_component:
	{
		struct dvb_component_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_component_descriptor\n");
		dx = dvb_component_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_component_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC stream_content:%i component_type:%i component_tag: %i language_code:%.3s, text:%.*s\n",
			dx->stream_content,
			dx->component_type,
			dx->component_tag,
			dx->language_code,
			dvb_component_descriptor_text_length(dx),
			dvb_component_descriptor_text(dx));
		break;
	}

	case dtag_dvb_mosaic:
	{
		struct dvb_mosaic_descriptor *dx;
		struct dvb_mosaic_info *curinfo;

		iprintf(indent, "DSC Decode dvb_mosaic_descriptor\n");
		dx = dvb_mosaic_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_mosaic_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC mosaic_entry_point:%i number_of_horiz_elementary_cells:%i number_of_vert_elementary_cells:%i\n",
			dx->mosaic_entry_point, dx->number_of_horiz_elementary_cells,
			dx->number_of_vert_elementary_cells);
		dvb_mosaic_descriptor_infos_for_each(dx, curinfo) {
			struct dvb_mosaic_info_part2 *part2;
			struct dvb_mosaic_linkage *linkage;
			struct dvb_mosaic_elementary_cell_field *curfield;

			part2 = dvb_mosaic_info_part2(curinfo);
			linkage = dvb_mosaic_linkage(part2);
			iprintf(indent+1, "DSC logical_cell_id:%i logical_cell_presentation_info:%i cell_linkage_info:0x%02x\n",
				curinfo->logical_cell_id, curinfo->logical_cell_presentation_info,
			        part2->cell_linkage_info);
			if (linkage) {
				switch(part2->cell_linkage_info) {
				case 0x01:
					iprintf(indent+1, "DSC bouquet_id:0x%04x\n",
							linkage->u.linkage_01.bouquet_id);
					break;

				case 0x02:
					iprintf(indent+1, "DSC original_network_id:0x%04x transport_stream_id:0x%04x service_id:0x%04x\n",
							linkage->u.linkage_02.original_network_id,
							linkage->u.linkage_02.transport_stream_id,
							linkage->u.linkage_02.service_id);
					break;

				case 0x03:
					iprintf(indent+1, "DSC original_network_id:0x%04x transport_stream_id:0x%04x service_id:0x%04x\n",
							linkage->u.linkage_03.original_network_id,
							linkage->u.linkage_03.transport_stream_id,
							linkage->u.linkage_03.service_id);
					break;

				case 0x04:
					iprintf(indent+1, "DSC original_network_id:0x%04x transport_stream_id:0x%04x service_id:0x%04x event_id:0x%04x\n",
							linkage->u.linkage_04.original_network_id,
							linkage->u.linkage_04.transport_stream_id,
							linkage->u.linkage_04.service_id,
							linkage->u.linkage_04.event_id);
					break;
				}
			}

			dvb_mosaic_info_fields_for_each(curinfo, curfield) {
				iprintf(indent+2, "DSC elementary_cell_id:0x%02x\n",
					curfield->elementary_cell_id);
			}
		}
		break;
	}

	case dtag_dvb_stream_identifier:
	{
		struct dvb_stream_identifier_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_stream_identifier_descriptor\n");
		dx = dvb_stream_identifier_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_stream_identifier_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC component_tag:%i\n",
			dx->component_tag);
		break;
	}

	case dtag_dvb_ca_identifier:
	{
		struct dvb_ca_identifier_descriptor *dx;
		int i;
		uint16_t *ids;

		iprintf(indent, "DSC Decode dvb_ca_identifier_descriptor\n");
		dx = dvb_ca_identifier_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_ca_identifier_descriptor decode error\n");
			return;
		}
		ids = dvb_ca_identifier_descriptor_ca_system_ids(dx);
		for(i=0; i< dvb_ca_identifier_descriptor_ca_system_ids_count(dx); i++) {
			iprintf(indent+i, "DSC system_id:0x%04x\n", ids[i]);
		}
		break;
	}

	case dtag_dvb_content:
	{
		struct dvb_content_descriptor *dx;
		struct dvb_content_nibble *cur;

		iprintf(indent, "DSC Decode dvb_content_descriptor\n");
		dx = dvb_content_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_content_descriptor decode error\n");
			return;
		}
		dvb_content_descriptor_nibbles_for_each(dx, cur) {
			iprintf(indent+1, "DSC content_nibble_level_1:%i content_nibble_level_2:%i user_nibble_1:%i user_nibble_2:%i\n",
				cur->content_nibble_level_1, cur->content_nibble_level_2,
				cur->user_nibble_1, cur->user_nibble_2);
		}
		break;
	}

	case dtag_dvb_parental_rating:
	{
		struct dvb_parental_rating_descriptor *dx;
		struct dvb_parental_rating *cur;

		iprintf(indent, "DSC Decode dvb_parental_rating_descriptor\n");
		dx = dvb_parental_rating_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_parental_rating_descriptor decode error\n");
			return;
		}
		dvb_parental_rating_descriptor_ratings_for_each(dx, cur) {
			iprintf(indent+1, "DSC country_code:%.3s rating:%i\n",
				cur->country_code, cur->rating);
		}
		break;
	}

	case dtag_dvb_teletext:
	{
		struct dvb_teletext_descriptor *dx;
		struct dvb_teletext_entry *cur;

		iprintf(indent, "DSC Decode dvb_teletext_descriptor\n");
		dx = dvb_teletext_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_teletext_descriptor decode error\n");
			return;
		}
		dvb_teletext_descriptor_entries_for_each(dx, cur) {
			iprintf(indent+1, "DSC language_code:%.3s type:%i magazine_number:%i page_number:%i\n",
				cur->language_code,
				cur->type, cur->magazine_number, cur->page_number);
		}
		break;
	}

	case dtag_dvb_telephone:
	{
		struct dvb_telephone_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_telephone_descriptor\n");
		dx = dvb_telephone_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_telephone_descriptor decode error\n");
			return;
		}
		iprintf(indent,
			"DSC foreign_availability:%i connection_type:%i country_prefix:%.*s "
			"international_area_code:%.*s operator_code:%.*s national_area_code:%.*s core_number:%.*s\n",
			dx->foreign_availability, dx->connection_type,
			dx->country_prefix_length, dvb_telephone_descriptor_country_prefix(dx),
			dx->international_area_code_length, dvb_telephone_descriptor_international_area_code(dx),
			dx->operator_code_length, dvb_telephone_descriptor_operator_code(dx),
			dx->national_area_code_length, dvb_telephone_descriptor_national_area_code(dx),
			dx->core_number_length, dvb_telephone_descriptor_core_number(dx));
		break;
	}

	case dtag_dvb_local_time_offset:
	{
		struct dvb_local_time_offset_descriptor *dx;
		struct dvb_local_time_offset *cur;

		iprintf(indent, "DSC Decode dvb_local_time_offset_descriptor\n");
		dx = dvb_local_time_offset_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_local_time_offset_descriptor decode error\n");
			return;
		}
		dvb_local_time_offset_descriptor_offsets_for_each(dx, cur) {
			iprintf(indent+1,
				"DSC country_code:%.3s country_region_id:%i "
				"local_time_offset_polarity:%i local_time_offset:%i "
				"time_of_change:%i next_time_offset:%i\n",
				cur->country_code, cur->country_region_id,
				cur->local_time_offset_polarity,
				dvbhhmm_to_seconds(cur->local_time_offset),
				dvbdate_to_unixtime(cur->time_of_change),
				dvbhhmm_to_seconds(cur->next_time_offset));
		}
		break;
	}

	case dtag_dvb_subtitling:
	{
		struct dvb_subtitling_descriptor *dx;
		struct dvb_subtitling_entry *cur;

		iprintf(indent, "DSC Decode dvb_subtitling_descriptor\n");
		dx = dvb_subtitling_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_subtitling_descriptor decode error\n");
			return;
		}
		dvb_subtitling_descriptor_subtitles_for_each(dx, cur) {
			iprintf(indent+1,
				"DSC language_code:%.3s subtitling_type:0x%02x composition_page_id:0x%04x ancillary_page_id:0x%04x\n",
				cur->language_code, cur->subtitling_type,
				cur->composition_page_id, cur->ancillary_page_id);
		}
		break;
	}

	case dtag_dvb_terrestial_delivery_system:
	{
		struct dvb_terrestrial_delivery_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_terrestrial_delivery_descriptor\n");
		dx = dvb_terrestrial_delivery_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_terrestrial_delivery_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC centre_frequency:%i bandwidth:%i priority:%i "
				"time_slicing_indicator:%i mpe_fec_indicator:%i constellation:%i "
				"hierarchy_information:%i code_rate_hp_stream:%i "
				"code_rate_lp_stream:%i guard_interval:%i transmission_mode:%i "
				"other_frequency_flag:%i\n",
			dx->centre_frequency, dx->bandwidth, dx->priority,
			dx->time_slicing_indicator, dx->mpe_fec_indicator,
			dx->constellation,
			dx->hierarchy_information, dx->code_rate_hp_stream,
			dx->code_rate_lp_stream, dx->guard_interval,
			dx->transmission_mode, dx->other_frequency_flag);
		break;
	}

	case dtag_dvb_multilingual_network_name:
	{
		struct dvb_multilingual_network_name_descriptor *dx;
		struct dvb_multilingual_network_name *cur;

		iprintf(indent, "DSC Decode dvb_multilingual_network_name_descriptor\n");
		dx = dvb_multilingual_network_name_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_multilingual_network_name_descriptor decode error\n");
			return;
		}
		dvb_multilingual_network_name_descriptor_names_for_each(dx, cur) {
			iprintf(indent+1,
				"DSC language_code:%.3s network_name:%.*s\n",
				cur->language_code,
				cur->network_name_length,
				dvb_multilingual_network_name_name(cur));
		}
		break;
	}

	case dtag_dvb_multilingual_bouquet_name:
	{
		struct dvb_multilingual_bouquet_name_descriptor *dx;
		struct dvb_multilingual_bouquet_name *cur;

		iprintf(indent, "DSC Decode dvb_multilingual_bouquet_name_descriptor\n");
		dx = dvb_multilingual_bouquet_name_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_multilingual_bouquet_name_descriptor decode error\n");
			return;
		}
		dvb_multilingual_bouquet_name_descriptor_names_for_each(dx, cur) {
			iprintf(indent+1,
				"DSC language_code:%.3s bouquet_name:%.*s\n",
				cur->language_code,
				cur->bouquet_name_length,
				dvb_multilingual_bouquet_name_name(cur));
		}
		break;
	}

	case dtag_dvb_multilingual_service_name:
	{
		struct dvb_multilingual_service_name_descriptor *dx;
		struct dvb_multilingual_service_name *cur;

		iprintf(indent, "DSC Decode dvb_multilingual_service_name_descriptor\n");
		dx = dvb_multilingual_service_name_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_multilingual_service_name_descriptor decode error\n");
			return;
		}
		dvb_multilingual_service_name_descriptor_names_for_each(dx, cur) {
			struct dvb_multilingual_service_name_part2 *part2;
			part2 = dvb_multilingual_service_name_part2(cur);

			iprintf(indent+1,
				"DSC language_code:%.3s provider_name:%.*s service_name:%.*s\n",
				cur->language_code,
				cur->service_provider_name_length,
				dvb_multilingual_service_name_service_provider_name(cur),
				part2->service_name_length,
				dvb_multilingual_service_name_service_name(part2));
		}
		break;
	}

	case dtag_dvb_multilingual_component:
	{
		struct dvb_multilingual_component_descriptor *dx;
		struct dvb_multilingual_component *cur;

		iprintf(indent, "DSC Decode dvb_multilingual_component_descriptor\n");
		dx = dvb_multilingual_component_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_multilingual_component_descriptor decode error\n");
			return;
		}
		dvb_multilingual_component_descriptor_components_for_each(dx, cur) {
			iprintf(indent+1,
				"DSC language_code:%.3s bouquet_name:%.*s\n",
				cur->language_code,
				cur->text_description_length,
				dvb_multilingual_component_text_char(cur));
		}
		break;
	}

	case dtag_dvb_private_data_specifier:
	{
		struct dvb_private_data_specifier_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_private_data_specifier_descriptor\n");
		dx = dvb_private_data_specifier_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_private_data_specifier_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC private_data_specifier:0x%08x\n",
			dx->private_data_specifier);
		break;
	}

	case dtag_dvb_service_move:
	{
		struct dvb_service_move_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_service_move_descriptor\n");
		dx = dvb_service_move_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_service_move_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC new_original_network_id:0x%04x new_transport_stream_id:0x%04x new_service_id:0x%04x\n",
			dx->new_original_network_id, dx->new_transport_stream_id, dx->new_service_id);
		break;
	}

	case dtag_dvb_short_smoothing_buffer:
	{
		struct dvb_short_smoothing_buffer_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_short_smoothing_buffer_descriptor\n");
		dx = dvb_short_smoothing_buffer_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_short_smoothing_buffer_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC sb_size:%i sb_leak_rate:%i\n",
			dx->sb_size, dx->sb_leak_rate);
		hexdump(indent, "DSC",
			dvb_short_smoothing_buffer_descriptor_reserved(dx),
			dvb_short_smoothing_buffer_descriptor_reserved_length(dx));
		break;
	}

	case dtag_dvb_frequency_list:
	{
		struct dvb_frequency_list_descriptor *dx;
		uint32_t *freqs;
		int count;
		int i;

		iprintf(indent, "DSC Decode dvb_frequency_list_descriptor\n");
		dx = dvb_frequency_list_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_frequency_list_descriptor decode error\n");
			return;
		}
		freqs = dvb_frequency_list_descriptor_centre_frequencies(dx);
		count = dvb_frequency_list_descriptor_centre_frequencies_count(dx);
		for(i=0; i< count; i++) {
			iprintf(indent+1, "DSC", "%i\n", freqs[i]);
		}
		break;
	}

	case dtag_dvb_partial_transport_stream:
	{
		struct dvb_partial_transport_stream_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_partial_transport_stream_descriptor\n");
		dx = dvb_partial_transport_stream_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_partial_transport_stream_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC peak_rate:%i minimum_overall_smoothing_rate:%i maximum_overall_smoothing_rate:%i\n",
			dx->peak_rate, dx->minimum_overall_smoothing_rate, dx->maximum_overall_smoothing_rate);
		break;
	}

	case dtag_dvb_data_broadcast:
	{
		struct dvb_data_broadcast_descriptor *dx;
		struct dvb_data_broadcast_descriptor_part2 *part2;

		iprintf(indent, "DSC Decode dvb_data_broadcast_descriptor\n");
		dx = dvb_data_broadcast_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_data_broadcast_descriptor decode error\n");
			return;
		}
		part2 = dvb_data_broadcast_descriptor_part2(dx);

		iprintf(indent, "DSC data_broadcast_id:0x%04x component_tag:0x%02x selector:%.*s language_code:%.3s text:%.*s\n",
			dx->data_broadcast_id, dx->component_tag,
			dx->selector_length, dvb_data_broadcast_descriptor_selector(dx),
			part2->language_code,
			part2->text_length, dvb_data_broadcast_descriptor_part2_text(part2));
		break;
	}

	case dtag_dvb_scrambling:
	{
		struct dvb_scrambling_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_scrambling_descriptor\n");
		dx = dvb_scrambling_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_scrambling_descriptor decode error\n");
			return;
		}

		iprintf(indent, "DSC scrambling_mode:0x%02x\n",
			dx->scrambling_mode);
		break;
	}

	case dtag_dvb_data_broadcast_id:
	{
		struct dvb_data_broadcast_id_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_data_broadcast_id_descriptor\n");
		dx = dvb_data_broadcast_id_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_data_broadcast_id_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC data_broadcast_id:0x%04x\n",
			dx->data_broadcast_id);
		hexdump(indent+1, "DSC",
			dvb_data_broadcast_id_descriptor_id_selector_byte(dx),
			dvb_data_broadcast_id_descriptor_id_selector_byte_length(dx));
		break;
	}

	case dtag_dvb_transport_stream:
	{
		struct dvb_transport_stream_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_transport_stream_descriptor\n");
		dx = dvb_transport_stream_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_transport_stream_descriptor decode error\n");
			return;
		}
		hexdump(indent, "DSC",
			dvb_transport_stream_descriptor_data(dx),
			dvb_transport_stream_descriptor_data_length(dx));
		break;
	}

	case dtag_dvb_dsng:
	{
		struct dvb_dsng_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_dsng_descriptor\n");
		dx = dvb_dsng_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_dsng_descriptor decode error\n");
			return;
		}
		hexdump(indent, "DSC",
			dvb_dsng_descriptor_data(dx),
			dvb_dsng_descriptor_data_length(dx));
		break;
	}

	case dtag_dvb_pdc:
	{
		struct dvb_pdc_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_pdc_descriptor\n");
		dx = dvb_pdc_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_pdc_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC programme_id_label:0x%06x\n",
			dx->programme_id_label);
		break;
	}

	case dtag_dvb_ac3:
	{
		struct dvb_ac3_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_ac3_descriptor\n");
		dx = dvb_ac3_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_ac3_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC ac3_type_flag:%i bsid_flag:%i mainid_flag:%i asvc_flag:%i\n",
			dx->ac3_type_flag, dx->bsid_flag, dx->mainid_flag, dx->asvc_flag);
		hexdump(indent+1, "DSC",
			dvb_ac3_descriptor_additional_info(dx),
			dvb_ac3_descriptor_additional_info_length(dx));
		break;
	}

	case dtag_dvb_ancillary_data:
	{
		struct dvb_ancillary_data_descriptor *dx;

		iprintf(indent, "DSC Decode dvb_ancillary_data_descriptor\n");
		dx = dvb_ancillary_data_descriptor_codec(d);
		if (dx == NULL) {
			fprintf(stderr, "DSC XXXX dvb_ancillary_data_descriptor decode error\n");
			return;
		}
		iprintf(indent, "DSC scale_factor_error_check:%i dab_ancillary_data:%i announcement_switching_data:%i extended_ancillary_data:%i dvd_video_ancillary_data:%i\n",
			dx->scale_factor_error_check,
			dx->dab_ancillary_data,
			dx->announcement_switching_data,
			dx->extended_ancillary_data,
			dx->dvd_video_ancillary_data);
		break;
	}

	case dtag_dvb_cell_list:
	case dtag_dvb_cell_frequency_link:
	case dtag_dvb_announcement_support:
	case dtag_dvb_application_signalling:
	case dtag_dvb_adaptation_field_data:
	case dtag_dvb_service_identifier:
	case dtag_dvb_service_availability:
	case dtag_dvb_default_authority:
	case dtag_dvb_related_content:
	case dtag_dvb_tva_id:
	case dtag_dvb_content_identifier:
	case dtag_dvb_time_slice_fec_identifier:
	case dtag_dvb_ecm_repetition_rate:
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
