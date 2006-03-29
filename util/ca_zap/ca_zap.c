/*
	CA-ZAP utility

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
#include <pthread.h>
#include <dvbcfg/dvbcfg_zapchannel.h>
#include <dvbapi/dvbdemux.h>
#include <dvbapi/dvbca.h>
#include <dvben50221/en50221_app_ca.h>
#include <dvben50221/en50221_app_mmi.h>
#include <ucsi/section.h>
#include <ucsi/mpeg/section.h>
#include "ca_zap.h"
#include "ca_zap_llci.h"
#include "ca_zap_hlci.h"



static int cazap_ca_info_callback(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t ca_id_count, uint16_t *ca_ids);
static int cazap_ca_pmt_reply_callback(void *arg, uint8_t slot_id, uint16_t session_number,
				       struct en50221_app_pmt_reply *reply, uint32_t reply_size);

static struct section_ext *read_section_ext(char *buf, int buflen, int adapter, int demux, int pid, int table_id);
static void *dvbthread_func(void* arg);
static void *camthread_func(void* arg);

extern int hlci_init();
extern int hlci_cam_added(int cafd);
extern void hlci_cam_removed();
extern void hlci_poll();
extern void hlci_shutdown();


#define MMI_STATE_CLOSED 0
#define MMI_STATE_OPEN 0
#define MMI_STATE_ENQ 0
#define MMI_STATE_MENU 0


en50221_app_ca ca_resource = NULL;
int ca_session_number = -1;
int ca_resource_connected = 0;

en50221_app_mmi mmi_resource = NULL;
int mmi_session_number = -1;
int mmi_state;

int dvb_adapter = 0;
int demux_device = 0;
int ca_device = 0;
int pmt_pid = -1;
int cafd;
int ca_type;
struct dvbcfg_zapchannel *channel;

void usage(void)
{
	static const char *usage = "\n"
		" CA-ZAP: A CA zapping application\n"
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
	pthread_t dvbthread;
	pthread_t camthread;
	char chanfile[PATH_MAX];
	char channel_name[20];
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
			dvb_adapter = strtoul(optarg, NULL, 0);
			break;

		case 'c':
			strncpy(chanfile, optarg, sizeof(chanfile));
			break;

		case 'n':
			strncpy(channel_name, optarg, sizeof(channel_name));
			break;

		case 'd':
			demux_device = strtoul(optarg, NULL, 0);
			break;

		case 's':
			ca_device = strtoul(optarg, NULL, 0);
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

	// initialise the CA stack
	cafd = dvbca_open(dvb_adapter, ca_device);
	if (cafd != -1) {
		ca_type = dvbca_get_interface_type(cafd);
		switch(ca_type) {
		case DVBCA_INTERFACE_LINK:
			if (llci_init()) {
				fprintf(stderr, "Failed to init LLCI\n");
				exit(1);
			}
			break;

		case DVBCA_INTERFACE_HLCI:
			if (hlci_init()) {
				fprintf(stderr, "Failed to init LLCI\n");
				exit(1);
			}
			break;

		default:
			fprintf(stderr, "Unknown CA interface type\n");
			exit(1);
		}

		// hook up the CA callbacks
		en50221_app_ca_register_info_callback(ca_resource, cazap_ca_info_callback, NULL);
		en50221_app_ca_register_pmt_reply_callback(ca_resource, cazap_ca_pmt_reply_callback, NULL);

		// FIXME: hook up the MMI callbacks

		// start the cam thread
		pthread_create(&camthread, NULL, camthread_func, NULL);
	}

	// start the DVB thread
	pthread_create(&dvbthread, NULL, dvbthread_func, NULL);

	// the main loop
	while(1) {
		// FIXME: do the UI
		sleep(1);
	}

	// FIXME: shutdown DVB stuff

	// shutdown CAM stuff
	if (cafd != -1) {
		// FIXME: kill thread

		switch(ca_type) {
		case DVBCA_INTERFACE_LINK:
			llci_shutdown();
			break;

		case DVBCA_INTERFACE_HLCI:
			hlci_shutdown();
			break;
		}
		close(cafd);
	}

	// done
	exit(0);
}

static void *camthread_func(void* arg)
{
	(void) arg;

	int cam_state = 0;
	while(1) {
		// monitor the cam state
		switch(dvbca_get_cam_state(cafd)) {
		case DVBCA_CAMSTATE_MISSING:
			if (cam_state) {
				switch(ca_type) {
					case DVBCA_INTERFACE_LINK:
						llci_cam_removed();
						break;

					case DVBCA_INTERFACE_HLCI:
						hlci_cam_removed();
						break;
				}
				cam_state = 0;
			}
			break;

		case DVBCA_CAMSTATE_READY:
			if (!cam_state) {
				switch(ca_type) {
					case DVBCA_INTERFACE_LINK:
						llci_cam_added(cafd);
						break;

					case DVBCA_INTERFACE_HLCI:
						hlci_cam_added(cafd);
						break;
				}
				cam_state = 1;
			}
			break;
		}

		// run the stack
		switch(ca_type) {
		case DVBCA_INTERFACE_LINK:
			llci_poll();
			break;

		case DVBCA_INTERFACE_HLCI:
			hlci_poll();
			break;
		}
	}

	return 0;
}


static void *dvbthread_func(void* arg)
{
	char capmt[4096];
	int pmtversion = -1;
	char buf[4096];
	struct section_ext *section_ext;
	struct mpeg_pat_section *pat;
	struct mpeg_pat_program *cur_program;
	(void) arg;

	// FIXME: tune the frontend?

	// read the PAT
	section_ext = read_section_ext(buf, sizeof(buf), dvb_adapter, demux_device, TRANSPORT_PAT_PID, stag_mpeg_program_association);
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

	// PMT monitoring loop
	while(1) {
        	// read the PMT
		struct section_ext *section_ext = read_section_ext(buf, sizeof(buf), dvb_adapter, 0, pmt_pid, stag_mpeg_program_map);
		if (section_ext == NULL) {
			fprintf(stderr, "Failed to read PMT\n");
			exit(1);
		}
		struct mpeg_pmt_section *pmt = mpeg_pmt_section_codec(section_ext);
		if (pmt == NULL) {
			fprintf(stderr, "Bad PMT received\n");
			exit(1);
		}
		if (pmt->head.version_number == pmtversion) {
			continue;
		}

		// FIXME: change PID filters on PMT change?

		// set the CA PMT if the CA resource is connected
		if (ca_resource_connected) {

			// translate it into a CA PMT
			int listmgmt = CA_LIST_MANAGEMENT_ONLY;
			if (pmtversion != -1) {
				listmgmt = CA_LIST_MANAGEMENT_UPDATE;
			}
			int size;
			if ((size = en50221_ca_format_pmt(pmt, capmt, sizeof(capmt), listmgmt,
							CA_PMT_CMD_ID_OK_DESCRAMBLING)) < 0) {
				fprintf(stderr, "Failed to format CA PMT object\n");
				exit(1);
			}

			// set it
			if (en50221_app_ca_pmt(ca_resource, ca_session_number, capmt, size)) {
				fprintf(stderr, "Failed to send CA PMT object\n");
				exit(1);
			}
		}

		// remember the version
		pmtversion = pmt->head.version_number;
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







static int cazap_ca_info_callback(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t ca_id_count, uint16_t *ca_ids)
{
	(void) arg;
	(void) slot_id;
	(void) session_number;

	printf("CAM supports the following ca system ids:\n");
	uint32_t i;
	for(i=0; i< ca_id_count; i++) {
		printf("  0x%04x\n", ca_ids[i]);
	}
	ca_resource_connected = 1;
	return 0;
}

static int cazap_ca_pmt_reply_callback(void *arg, uint8_t slot_id, uint16_t session_number,
				      struct en50221_app_pmt_reply *reply, uint32_t reply_size)
{
	(void) arg;
	(void) slot_id;
	(void) session_number;
	(void) reply;
	(void) reply_size;

	return 0;
}
