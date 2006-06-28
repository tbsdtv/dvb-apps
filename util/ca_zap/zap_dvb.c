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
static int create_decoder_filter(int adapter, int demux, uint16_t pid, int pestype);
static int create_dvr_filter(int adapter, int demux, uint16_t pid);

static void process_pat(int pat_fd, struct zap_dvb_params *params,
			int *pat_version, int *pmt_fd, int *pmt_fd_dvrout, int *pmt_version, struct pollfd *pollfd);
static void process_tdt(int tdt_fd);
static void process_pmt(int pmt_fd, struct zap_dvb_params *params, int *pmt_version);
static void decoder_pmt(struct zap_dvb_params *params, struct mpeg_pmt_section *pmt);
static void dvr_pmt(struct zap_dvb_params *params, struct mpeg_pmt_section *pmt);
static void dvr_pmt_full(struct zap_dvb_params *params, struct mpeg_pmt_section *pmt);
static void append_pid_fd(int pid, int fd);
static void free_pid_fds(void);

struct pid_fd {
	int pid;
	int fd;
};
struct pid_fd *pid_fds = NULL;
int pid_fds_count = 0;

int zap_dvb_start(struct zap_dvb_params *params)
{
	pid_fds = NULL;
	pid_fds_count = 0;

	pthread_create(&dvbthread, (void*) params, dvbthread_func, NULL);
	return 0;
}

void zap_dvb_stop(void)
{
	dvbthread_shutdown = 1;
	pthread_join(dvbthread, NULL);
	free_pid_fds();
}

static void *dvbthread_func(void* arg)
{
	int tune_state = 0;

	int pat_fd = -1;
	int pat_fd_dvrout = -1;
	int pat_version = -1;

	int pmt_fd = -1;
	int pmt_fd_dvrout = -1;
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

	// output PAT to DVR if requested
	switch(params->output_type) {
	case OUTPUT_TYPE_DVR_FULL:
	case OUTPUT_TYPE_FILE_FULL:
		pat_fd_dvrout = create_dvr_filter(params->adapter_id, params->demux_id, TRANSPORT_PAT_PID);
	}

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
			uint32_t frequency_adjust = 0;
			/*
			// FIXME: implement this
			if (result.type == DVBFE_TYPE_DVBS) {
				if (dvbfe_sec_command(params->fe, params->sec.command)) {
					fprintf(stderr, "Failed to execute SEC command %s\n", params->sec.command);
					exit(1);
				}
				frequency_adjust = params->sec.lof;
			}
			*/

			// set the frontend params
			params->channel.fe_params.frequency -= frequency_adjust;
			if (dvbfe_set(params->fe, &params->channel.fe_params, 0)) {
				fprintf(stderr, "Failed to set frontend\n");
				exit(1);
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
			process_pat(pat_fd, params, &pat_version, &pmt_fd, &pmt_fd_dvrout, &pmt_version, &pollfds[2]);
		}

		// TDT
		if (pollfds[1].revents & (POLLIN|POLLPRI)) {
			process_tdt(tdt_fd);
		}

		//  PMT
		if (pollfds[2].revents & (POLLIN|POLLPRI)) {
			process_pmt(pmt_fd, params, &pmt_version);
		}
	}

	// close demuxers
	if (pat_fd != -1)
		close(pat_fd);
	if (pat_fd_dvrout != -1)
		close(pat_fd_dvrout);
	if (pmt_fd != -1)
		close(pmt_fd);
	if (pmt_fd_dvrout != -1)
		close(pmt_fd_dvrout);
	if (tdt_fd != -1)
		close(tdt_fd);

	return 0;
}

static int create_section_filter(int adapter, int demux, uint16_t pid, uint8_t table_id)
{
	int demux_fd = -1;
	uint8_t filter[18];
	uint8_t mask[18];

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

static int create_decoder_filter(int adapter, int demux, uint16_t pid, int pestype)
{
	int demux_fd = -1;

	// open the demuxer
	if ((demux_fd = dvbdemux_open_demux(adapter, demux, 0)) < 0) {
		return -1;
	}

	// create a section filter
	if (dvbdemux_set_pes_filter(demux_fd, pid, DVBDEMUX_INPUT_FRONTEND, DVBDEMUX_OUTPUT_DECODER, pestype, 1)) {
		close(demux_fd);
		return -1;
	}

	// done
	return demux_fd;
}

static int create_dvr_filter(int adapter, int demux, uint16_t pid)
{
	int demux_fd = -1;

	// open the demuxer
	if ((demux_fd = dvbdemux_open_demux(adapter, demux, 0)) < 0) {
		return -1;
	}

	// create a section filter
	if (dvbdemux_set_pid_filter(demux_fd, pid, DVBDEMUX_INPUT_FRONTEND, DVBDEMUX_OUTPUT_DVR, 1)) {
		close(demux_fd);
		return -1;
	}

	// done
	return demux_fd;
}

static void process_pat(int pat_fd, struct zap_dvb_params *params,
			int *pat_version, int *pmt_fd, int *pmt_fd_dvrout, int *pmt_version, struct pollfd *pollfd)
{
	int size;
	uint8_t sibuf[4096];

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

			// output PMT to DVR if requested
			switch(params->output_type) {
			case OUTPUT_TYPE_DVR_FULL:
			case OUTPUT_TYPE_FILE_FULL:
				if (*pmt_fd_dvrout != -1)
					close(*pmt_fd_dvrout);
				*pmt_fd_dvrout = create_dvr_filter(params->adapter_id, params->demux_id, cur_program->pid);
			}

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
	uint8_t sibuf[4096];

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

static void process_pmt(int pmt_fd, struct zap_dvb_params *params, int *pmt_version)
{
	int size;
	uint8_t sibuf[4096];

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

	// close all old PID FDs
	free_pid_fds();

	// deal with the PMT appropriately
	switch(params->output_type) {
	case OUTPUT_TYPE_DECODER:
	case OUTPUT_TYPE_DECODER_ABYPASS:
		decoder_pmt(params, pmt);
		break;

	case OUTPUT_TYPE_DVR:
	case OUTPUT_TYPE_FILE:
		dvr_pmt(params, pmt);
		break;

	case OUTPUT_TYPE_DVR_FULL:
	case OUTPUT_TYPE_FILE_FULL:
		dvr_pmt_full(params, pmt);
		break;
	}

	// inform the main app of the new PMT
	new_dvb_pmt(pmt);
}

static void decoder_pmt(struct zap_dvb_params *params, struct mpeg_pmt_section *pmt)
{
	int audio_pid = -1;
	int video_pid = -1;
	struct mpeg_pmt_stream *cur_stream;
	mpeg_pmt_section_streams_for_each(pmt, cur_stream) {
		switch(cur_stream->stream_type) {
		case 1:
		case 2: // video
			video_pid = cur_stream->pid;
			break;

		case 3:
		case 4: // audio
			audio_pid = cur_stream->pid;
			break;
		}
	}

	if (audio_pid != -1) {
		int fd = create_decoder_filter(params->adapter_id, params->demux_id, audio_pid, DVBDEMUX_PESTYPE_AUDIO);
		if (fd < 0) {
			fprintf(stderr, "Unable to create dvr filter for PID %i\n", audio_pid);
		} else {
			append_pid_fd(audio_pid, fd);
		}
	}
	if (video_pid != -1) {
		int fd = create_decoder_filter(params->adapter_id, params->demux_id, audio_pid, DVBDEMUX_PESTYPE_VIDEO);
		if (fd < 0) {
			fprintf(stderr, "Unable to create dvr filter for PID %i\n", audio_pid);
		} else {
			append_pid_fd(audio_pid, fd);
		}
	}
	int fd = create_decoder_filter(params->adapter_id, params->demux_id, pmt->pcr_pid, DVBDEMUX_PESTYPE_PCR);
	if (fd < 0) {
		fprintf(stderr, "Unable to create dvr filter for PID %i\n", pmt->pcr_pid);
	} else {
		append_pid_fd(pmt->pcr_pid, fd);
	}
}

static void dvr_pmt(struct zap_dvb_params *params, struct mpeg_pmt_section *pmt)
{
	int audio_pid = -1;
	int video_pid = -1;
	struct mpeg_pmt_stream *cur_stream;
	mpeg_pmt_section_streams_for_each(pmt, cur_stream) {
		switch(cur_stream->stream_type) {
		case 1:
		case 2: // video
			video_pid = cur_stream->pid;
			break;

		case 3:
		case 4: // audio
			audio_pid = cur_stream->pid;
			break;
		}
	}

	if (audio_pid != -1) {
		int fd = create_dvr_filter(params->adapter_id, params->demux_id, audio_pid);
		if (fd < 0) {
			fprintf(stderr, "Unable to create dvr filter for PID %i\n", audio_pid);
		} else {
			append_pid_fd(audio_pid, fd);
		}
	}
	if (video_pid != -1) {
		int fd = create_dvr_filter(params->adapter_id, params->demux_id, video_pid);
		if (fd < 0) {
			fprintf(stderr, "Unable to create dvr filter for PID %i\n", video_pid);
		} else {
			append_pid_fd(video_pid, fd);
		}
	}
}

static void dvr_pmt_full(struct zap_dvb_params *params, struct mpeg_pmt_section *pmt)
{
	struct mpeg_pmt_stream *cur_stream;
	mpeg_pmt_section_streams_for_each(pmt, cur_stream) {
		int fd = create_dvr_filter(params->adapter_id, params->demux_id, cur_stream->pid);
		if (fd < 0) {
			fprintf(stderr, "Unable to create dvr filter for PID %i\n", cur_stream->pid);
		} else {
			append_pid_fd(cur_stream->pid, fd);
		}
	}
}

static void append_pid_fd(int pid, int fd)
{
	struct pid_fd *tmp;
	if ((tmp = realloc(pid_fds, (pid_fds_count +1) * sizeof(struct pid_fd))) == NULL) {
		fprintf(stderr, "Out of memory when adding a new pid_fd\n");
		exit(1);
	}
	tmp[pid_fds_count].pid = pid;
	tmp[pid_fds_count].fd = fd;
	pid_fds_count++;
	pid_fds = tmp;
}

static void free_pid_fds()
{
	if (pid_fds_count) {
		int i;
		for(i=0; i< pid_fds_count; i++) {
			close(pid_fds[i].fd);
		}
	}
	if (pid_fds)
		free(pid_fds);

	pid_fds_count = 0;
	pid_fds = NULL;
}
