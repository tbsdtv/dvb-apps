/*
	CA-ZAP utility
	an implementation for the High Level Common Interface

	Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)

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
#include <dvbcfg/dvbcfg_zapchannel.h>
#include <dvbapi/dvbdemux.h>
#include <dvbapi/dvbca.h>
#include <dvben50221/en50221_app_ca.h>
#include <ucsi/section.h>
#include <ucsi/mpeg/section.h>

#include "ca_zap.h"

static struct section_ext *read_section_ext(char *buf, int buflen, int adapter, int demux, int pid, int table_id);
static void set_cam_hlci(int cafd, char *capmt, int capmtsize);
static void set_cam_link(int cafd, char *capmt, int capmtsize);

void usage(void)
{
	static const char *usage = "\n"
		" CA-ZAP: A CA zapping application for HLCI or Link layer CAMs\n"
		" Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)\n\n"
		" usage: ca_zap options\n"
		" -h	help\n"
		" -a	adapter to use\n"
		" -c	channels.conf file\n"
		" -n	channel name to zap\n"
		" -d	demux to use\n"
		" -s	CA module (Slot to use)\n"
		" -m    Move CA descriptors from stream to programme level if possible\n";

	fprintf(stderr, "%s\n", usage);

	exit(1);
}

int main(int argc, char *argv[])
{
	struct dvbcfg_zapchannel *channels = NULL;
	struct dvbcfg_zapchannel *channel;
	char buf[4096];
	int pmt_pid = -1;
	struct section_ext *section_ext;
	struct mpeg_pat_section *pat;
	struct mpeg_pat_program *cur_program;
	struct mpeg_pmt_section *pmt;

	char chanfile[PATH_MAX];
	char channel_name[20];
	uint8_t dev = 0, demux = 0, ca = 0;
	int opt;
	int move_to_programme = 0;

	chanfile[0] = 0;
	channel_name[0] = 0;
	if (argc < 3)
		usage();
	// a = adapter, c = channels.conf, t = type (S/C/T), d = demux, s = CA device, f = frontend device n = channel name
	while ((opt = getopt(argc, argv, "t:n:h:a:c:d:s:f:m")) != -1) {
		switch (opt) {
			case 'a':
				dev = strtoul(optarg, NULL, 0);
				break;

			case 'c':
				strncpy(chanfile, optarg, sizeof(chanfile));
				break;

			case 'n':
				strncpy(channel_name, optarg, sizeof(channel_name));
				break;

			case 'd':
				demux = strtoul(optarg, NULL, 0);
				break;

			case 's':
				ca = strtoul(optarg, NULL, 0);
				break;

			case 'f':
				// deprecated
				break;

			case 'm':
				move_to_programme = 1;
				break;

			case 'h':
			default:
				usage();
				break;

		}
	}

	// parse the supplied channel list
	if (dvbcfg_zapchannel_load(chanfile, &channels)) {
		fprintf(stderr, "Failed to load channels %s\n", chanfile);
		exit(1);
	}

	// find the requested channel
	if ((channel = dvbcfg_zapchannel_find(channels, channel_name)) == NULL) {
		fprintf(stderr, "Unable to find requested channel %s\n", channel_name);
		exit(1);
	}

	// read the PAT
	section_ext = read_section_ext(buf, sizeof(buf), dev, demux, TRANSPORT_PAT_PID, stag_mpeg_program_association);
	if (section_ext == NULL) {
		fprintf(stderr, "Failed to read PAT\n");
		exit(1);
	}

	// process the PAT to find the requested program's PMT PID
	pat = mpeg_pat_section_codec(section_ext);
	if (pat == NULL) {
		fprintf(stderr, "Bad PAT received\n");
		exit(1);
	}
	mpeg_pat_section_programs_for_each(pat, cur_program) {
		if (cur_program->program_number == channel->channel_number) {
			pmt_pid = cur_program->pid;
			break;
		}
	}
	if (pmt_pid == -1) {
		fprintf(stderr, "Did not find requested program in PAT\n");
		exit(1);
	}

	// read the PMT
	section_ext = read_section_ext(buf, sizeof(buf), dev, demux, pmt_pid, stag_mpeg_program_map);
	if (section_ext == NULL) {
		fprintf(stderr, "Failed to read PMT\n");
		exit(1);
	}
	pmt = mpeg_pmt_section_codec(section_ext);
	if (pmt == NULL) {
		fprintf(stderr, "Bad PMT received\n");
		exit(1);
	}

	// process the PMT into a CAPMT
	uint8_t capmt[4096];
	int size = en50221_ca_format_pmt(pmt, capmt, sizeof(capmt),
					 CA_LIST_MANAGEMENT_ONLY, CA_PMT_CMD_ID_OK_DESCRAMBLING);
	if (size < 0) {
		fprintf(stderr, "Failed to format PMT");
		exit(1);
	}

	// open the CA device and dispatch depending on its type
	int cafd = dvbca_open(dev, ca);
	if (cafd < 0) {
		fprintf(stderr, "Failed to open CA device\n");
		exit(1);
	}
	if (dvbca_get_cam_state(cafd) != DVBCA_CAMSTATE_READY) {
		fprintf(stderr, "CAM not detected in CA slot\n");
		exit(1);
	}
	switch(dvbca_get_interface_type(cafd)) {
	case DVBCA_INTERFACE_LINK:
		set_cam_link(cafd, capmt, size);
		break;

	case DVBCA_INTERFACE_HLCI:
		set_cam_hlci(cafd, capmt, size);
		break;

	default:
		fprintf(stderr, "Unknown CA interface type\n");
		exit(1);
	}

	return 0;
}

static struct section_ext *read_section_ext(char *buf, int buflen, int adapter, int demux, int pid, int table_id)
{
	int demux_fd = -1;
	char filter[18];
	char mask[18];
	int size;
	struct section *section;
	struct section_ext *result = NULL;

	// open the demuxer
	if ((demux_fd = dvbdemux_open_demux(adapter, demux, 0)) < 0) {
		goto exit;
	}

	// create a section filter
	memset(filter, 0, sizeof(filter));
	memset(mask, 0, sizeof(mask));
	filter[0] = table_id;
	mask[0] = 0xFF;
	if (dvbdemux_set_section_filter(demux_fd, pid, filter, mask, 1, 1)) {
		goto exit;
	}

	// read the section
	if ((size = read(demux_fd, buf, buflen)) < 0) {
		goto exit;
	}

	// parse it as a section
	section = section_codec(buf, size);
	if (section == NULL) {
		goto exit;
	}

	// parse it as a section_ext
	result = section_ext_decode(section, 0);

exit:
	if (demux_fd != -1)
		close(demux_fd);
	return result;
}

static void set_cam_hlci(int cafd, char *capmt, int capmtsize) {
	// FIXME
}

static void set_cam_link(int cafd, char *capmt, int capmtsize) {
	// FIXME
}
