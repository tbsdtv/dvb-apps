/*
	ZAP utility DVB functions

	Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)
	Copyright (C) 2006 Andrew de Quincey (adq_dvb@lidskialf.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as
	published by the Free Software Foundation; either version 2.1 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <pthread.h>
#include <sys/poll.h>
#include <libdvbcfg/dvbcfg_zapchannel.h>
#include <libdvbapi/dvbdemux.h>
#include <libucsi/section.h>
#include <libucsi/mpeg/section.h>
#include <libucsi/dvb/section.h>
#include "zap.h"
#include "zap_dvb.h"

#define FE_STATUS_PARAMS (DVBFE_INFO_LOCKSTATUS|DVBFE_INFO_SIGNAL_STRENGTH|DVBFE_INFO_BER|DVBFE_INFO_SNR|DVBFE_INFO_UNCORRECTED_BLOCKS)

static int dvbthread_shutdown = 0;
static pthread_t dvbthread;

static void *dvbthread_func(void* arg);
static int create_section_filter(int adapter, int demux, uint16_t pid, uint8_t table_id);
static void process_pat(int pat_fd, struct zap_dvb_params *params,
			int *pat_version, int *pmt_fd, int *pmt_version, struct pollfd *pollfd);
static void process_tdt(int tdt_fd);
static void process_pmt(int pmt_fd, int *pmt_version);


int zap_dvb_start(struct zap_dvb_params *params)
{
	pthread_create(&dvbthread, (void*) params, dvbthread_func, NULL);
	return 0;
}

void zap_dvb_stop(void)
{
	dvbthread_shutdown = 1;
	pthread_join(dvbthread, NULL);
}

static void *dvbthread_func(void* arg)
{
	int tune_state = 0;

	int pat_fd = -1;
	int pat_version = -1;

	int pmt_fd = -1;
	int pmt_version = -1;

	int tdt_fd = -1;

	struct pollfd pollfds[3];

	struct zap_dvb_params *params = (struct zap_dvb_params *) arg;


	// create PAT filter
	if ((pat_fd = create_section_filter(params->adapter_id, params->demux_id,
	     TRANSPORT_PAT_PID, stag_mpeg_program_association)) < 0) {
		fprintf(stderr, "Failed to create PAT section filter\n");
		exit(1);
	}
	pollfds[0].fd = pat_fd;
	pollfds[0].events = POLLIN|POLLPRI|POLLERR;

	// create TDT filter
	if ((tdt_fd = create_section_filter(params->adapter_id, params->demux_id, TRANSPORT_TDT_PID, stag_dvb_time_date)) < 0) {
		fprintf(stderr, "Failed to create TDT section filter\n");
		exit(1);
	}
	pollfds[1].fd = tdt_fd;
	pollfds[1].events = POLLIN|POLLPRI|POLLERR;

	// zero PMT filter
	pollfds[2].fd = 0;
	pollfds[2].events = 0;

	// the DVB loop
	while(!dvbthread_shutdown) {
		// tune frontend + monitor lock status
		if (tune_state == 0) {
			// get the type of frontend
			struct dvbfe_info result;
			memset(&result, 0, sizeof(result));
			dvbfe_get_info(params->fe, 0, &result);

			// do sec for DVBS
			if (result.type == DVBFE_TYPE_DVBS) {
				if (dvbfe_sec_command(params->fe, params->sec.command)) {
					fprintf(stderr, "Failed to execute SEC command %s\n", params->sec.command);
					exit(1);
				}
				params->channel.fe_params.frequency -= params->sec.lof;
			}

			// set the frontend params
			if (dvbfe_set(params->fe, &params->channel.fe_params, 0)) {
				fprintf(stderr, "Failed to set frontend\n");
				exit(1);
			}

			// restore the frequency in the channel in case we need it later
			if (result.type == DVBFE_TYPE_DVBS) {
				params->channel.fe_params.frequency += params->sec.lof;
			}

			tune_state++;
		} else if (tune_state == 1) {
			struct dvbfe_info result;
			memset(&result, 0, sizeof(result));
			dvbfe_get_info(params->fe, FE_STATUS_PARAMS, &result);

			printf ("status %c%c%c%c%c | signal %04x | snr %04x | ber %08x | unc %08x |\r",
				result.signal ? 'S' : ' ',
				result.carrier ? 'C' : ' ',
				result.viterbi ? 'V' : ' ',
				result.sync ? 'Y' : ' ',
				result.lock ? 'L' : ' ',
				result.signal_strength,
				result.snr,
				result.ber,
				result.ucblocks);
			fflush(stdout);

			if (result.lock) {
				tune_state++;
				printf("\n");
			} else {
				usleep(500000);
			}
		}

		// is there SI data?
		int count = poll(pollfds, 3, 100);
		if (count < 0) {
			fprintf(stderr, "Poll error\n");
			break;
		}
		if (count == 0) {
			continue;
		}

		// PAT
		if (pollfds[0].revents & (POLLIN|POLLPRI)) {
			process_pat(pat_fd, params, &pat_version, &pmt_fd, &pmt_version, &pollfds[2]);
		}

		// TDT
		if (pollfds[1].revents & (POLLIN|POLLPRI)) {
			process_tdt(tdt_fd);
		}

		//  PMT
		if (pollfds[2].revents & (POLLIN|POLLPRI)) {
			process_pmt(pmt_fd, &pmt_version);
		}
	}

	// close demuxers
	if (pat_fd != -1)
		close(pat_fd);
	if (pmt_fd != -1)
		close(pmt_fd);
	if (tdt_fd != -1)
		close(tdt_fd);

	return 0;
}

static int create_section_filter(int adapter, int demux, uint16_t pid, uint8_t table_id)
{
	int demux_fd = -1;
	char filter[18];
	char mask[18];

	// open the demuxer
	if ((demux_fd = dvbdemux_open_demux(adapter, demux, 0)) < 0) {
		return -1;
	}

	// create a section filter
	memset(filter, 0, sizeof(filter));
	memset(mask, 0, sizeof(mask));
	filter[0] = table_id;
	mask[0] = 0xFF;
	if (dvbdemux_set_section_filter(demux_fd, pid, filter, mask, 1, 1)) {
		close(demux_fd);
		return -1;
	}

	// done
	return demux_fd;
}

static void process_pat(int pat_fd, struct zap_dvb_params *params,
			int *pat_version, int *pmt_fd, int *pmt_version, struct pollfd *pollfd)
{
	int size;
	char sibuf[4096];

	// read the section
	if ((size = read(pat_fd, sibuf, sizeof(sibuf))) < 0) {
		return;
	}

	// parse section
	struct section *section = section_codec(sibuf, size);
	if (section == NULL) {
		return;
	}

	// parse section_ext
	struct section_ext *section_ext = section_ext_decode(section, 0);
	if (section_ext == NULL) {
		return;
	}
	if (*pat_version == section_ext->version_number) {
		return;
	}

	// parse PAT
	struct mpeg_pat_section *pat = mpeg_pat_section_codec(section_ext);
	if (pat == NULL) {
		return;
	}

	// try and find the requested program
	struct mpeg_pat_program *cur_program;
	mpeg_pat_section_programs_for_each(pat, cur_program) {
		if (cur_program->program_number == params->channel.channel_number) {
			// close old PMT fd
			if (*pmt_fd != -1)
				close(*pmt_fd);

			// create PMT filter
			if ((*pmt_fd = create_section_filter(params->adapter_id, params->demux_id,
							     cur_program->pid, stag_mpeg_program_map)) < 0) {
				return;
			}
			pollfd->fd = *pmt_fd;
			pollfd->events = POLLIN|POLLPRI|POLLERR;

			// we have a new PMT pid
			*pmt_version = -1;
			break;
		}
	}

	// remember the PAT version
	*pat_version = section_ext->version_number;
}

static void process_tdt(int tdt_fd)
{
	int size;
	char sibuf[4096];

	// read the section
	if ((size = read(tdt_fd, sibuf, sizeof(sibuf))) < 0) {
		return;
	}

	// parse section
	struct section *section = section_codec(sibuf, size);
	if (section == NULL) {
		return;
	}

	// parse TDT
	struct dvb_tdt_section *tdt = dvb_tdt_section_codec(section);
	if (tdt == NULL) {
		return;
	}

	// done
	new_dvb_time(dvbdate_to_unixtime(tdt->utc_time));
}

static void process_pmt(int pmt_fd, int *pmt_version)
{
	int size;
	char sibuf[4096];

	// read the section
	if ((size = read(pmt_fd, sibuf, sizeof(sibuf))) < 0) {
		return;
	}

	// parse section
	struct section *section = section_codec(sibuf, size);
	if (section == NULL) {
		return;
	}

	// parse section_ext
	struct section_ext *section_ext = section_ext_decode(section, 0);
	if (section_ext == NULL) {
		return;
	}
	if (*pmt_version == section_ext->version_number) {
		return;
	}

	// parse PMT
	struct mpeg_pmt_section *pmt = mpeg_pmt_section_codec(section_ext);
	if (pmt == NULL) {
		return;
	}

	// FIXME: update PID filters on PMT change

	// inform the main app of the new PMT
	new_dvb_pmt(pmt);
}
