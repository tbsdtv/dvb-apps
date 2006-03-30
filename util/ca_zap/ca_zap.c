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
#include <signal.h>
#include <pthread.h>
#include <sys/poll.h>
#include <dvbcfg/dvbcfg_zapchannel.h>
#include <dvbapi/dvbdemux.h>
#include <dvbapi/dvbca.h>
#include <dvben50221/en50221_app_ai.h>
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
static int cazap_ai_callback(void *arg, uint8_t slot_id, uint16_t session_number,
			     uint8_t application_type, uint16_t application_manufacturer,
			     uint16_t manufacturer_code, uint8_t menu_string_length,
			     uint8_t *menu_string);

static int cazap_mmi_close_callback(void *arg, uint8_t slot_id, uint16_t session_number,
			     uint8_t cmd_id, uint8_t delay);
static int cazap_mmi_display_control_callback(void *arg, uint8_t slot_id, uint16_t session_number,
			               uint8_t cmd_id, uint8_t mmi_mode);
static int cazap_mmi_enq_callback(void *arg, uint8_t slot_id, uint16_t session_number,
			    uint8_t blind_answer, uint8_t expected_answer_length,
			    uint8_t *text, uint32_t text_size);
static int cazap_mmi_menu_callback(void *arg, uint8_t slot_id, uint16_t session_number,
			    struct en50221_app_mmi_text *title,
			    struct en50221_app_mmi_text *sub_title,
			    struct en50221_app_mmi_text *bottom,
			    uint32_t item_count, struct en50221_app_mmi_text *items,
			    uint32_t item_raw_length, uint8_t *items_raw);


static struct section_ext *read_section_ext(char *buf, int buflen, int adapter, int demux, int pid, int table_id, int timeout);
static void *dvbthread_func(void* arg);
static void *camthread_func(void* arg);
static void signal_handler(int signal);

extern int hlci_init();
extern int hlci_cam_added(int cafd);
extern void hlci_cam_removed();
extern void hlci_poll();
extern void hlci_shutdown();


#define MMI_STATE_CLOSED 0
#define MMI_STATE_OPEN 1
#define MMI_STATE_ENQ 2
#define MMI_STATE_MENU 3

en50221_app_ai ai_resource = NULL;
int ai_session_number = -1;

en50221_app_ca ca_resource = NULL;
int ca_session_number = -1;
int ca_resource_connected = 0;

en50221_app_mmi mmi_resource = NULL;
int mmi_session_number = -1;
int mmi_state = MMI_STATE_CLOSED;
int mmi_enq_blind;
int mmi_enq_length;

int dvb_adapter = 0;
int demux_device = 0;
int ca_device = 0;

int move_to_programme = 0;
int pmt_pid = -1;
int cafd;
int ca_type;
struct dvbcfg_zapchannel *channel;

int camthread_shutdown = 0;
int dvbthread_shutdown = 0;
int quit_app = 0;

void usage(void)
{
	static const char *usage = "\n"
		" CA-ZAP: A CA zapping application\n"
		" Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)\n\n"
		" usage: ca_zap options\n"
		" -h	help\n"
		" -a	adapter to use (default 0)\n"
		" -c	channels.conf file\n"
		" -n	channel name to zap\n"
		" -d	demux to use (default 0)\n"
		" -s	ca slot to use (default 0)\n"
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
				fprintf(stderr, "Failed to init HLCI\n");
				exit(1);
			}
			break;

		default:
			fprintf(stderr, "Unknown CA interface type\n");
			exit(1);
		}

		// hook up the AI callbacks
		if (ai_resource) {
			en50221_app_ai_register_callback(ai_resource, cazap_ai_callback, NULL);
		}

		// hook up the CA callbacks
		if (ca_resource) {
			en50221_app_ca_register_info_callback(ca_resource, cazap_ca_info_callback, NULL);
			en50221_app_ca_register_pmt_reply_callback(ca_resource, cazap_ca_pmt_reply_callback, NULL);
		}

		// hook up the MMI callbacks
		if (mmi_resource) {
			en50221_app_mmi_register_close_callback(mmi_resource, cazap_mmi_close_callback, NULL);
			en50221_app_mmi_register_display_control_callback(mmi_resource, cazap_mmi_display_control_callback, NULL);
			en50221_app_mmi_register_enq_callback(mmi_resource, cazap_mmi_enq_callback, NULL);
			en50221_app_mmi_register_menu_callback(mmi_resource, cazap_mmi_menu_callback, NULL);
			en50221_app_mmi_register_list_callback(mmi_resource, cazap_mmi_menu_callback, NULL);
		}

		// start the cam thread
		pthread_create(&camthread, NULL, camthread_func, NULL);
	}

	// start the DVB thread
	pthread_create(&dvbthread, NULL, dvbthread_func, NULL);

	// make up polling structure for stdin
	struct pollfd pollfd;
	pollfd.fd = 0;
	pollfd.events = POLLIN|POLLPRI|POLLERR;

	// the main loop
	char line[256];
	uint32_t linepos = 0;
	printf("Enter to start CAM menu, or Ctrl-C to quit\n");
	signal(SIGINT, signal_handler);
	while(!quit_app) {
		if (poll(&pollfd, 1, 10) != 1)
			continue;
		if (pollfd.revents & POLLERR)
			continue;

		// read the character
		char c;
		if (read(0, &c, 1) != 1)
			continue;
		if (c == '\r') {
			continue;
		} else if (c == '\n') {

			switch(mmi_state) {
			case MMI_STATE_CLOSED:
			case MMI_STATE_OPEN:
				if (linepos == 0) {
					en50221_app_ai_entermenu(ai_resource, ai_session_number);
				}
				break;

			case MMI_STATE_ENQ:
				if (linepos == 0) {
					en50221_app_mmi_answ(mmi_resource, mmi_session_number, MMI_ANSW_ID_CANCEL, NULL, 0);
				} else {
					en50221_app_mmi_answ(mmi_resource, mmi_session_number,
							     MMI_ANSW_ID_ANSWER, line, linepos);
				}
				mmi_state = MMI_STATE_OPEN;
				break;

			case MMI_STATE_MENU:
				line[linepos] = 0;
				en50221_app_mmi_menu_answ(mmi_resource, mmi_session_number, atoi(line));
				mmi_state = MMI_STATE_OPEN;
				break;
			}
			linepos = 0;
		} else {
			if (linepos < (sizeof(line)-1)) {
				line[linepos++] = c;
			}
		}
	}

	// shutdown DVB stuff
	dvbthread_shutdown = 1;
	pthread_join(dvbthread, NULL);

	// shutdown CAM stuff
	if (cafd != -1) {
		// shutdown the cam thread
		camthread_shutdown = 1;
		pthread_join(camthread, NULL);

		// shutdown the stack
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

static void signal_handler(int signal)
{
	(void) signal;

	if (!quit_app) {
		printf("Shutting down..\n");
		quit_app = 1;
	}
}

static void *camthread_func(void* arg)
{
	(void) arg;

	int cam_state = 0;
	while(!camthread_shutdown) {
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
	struct section_ext *section_ext = NULL;
	struct mpeg_pat_section *pat;
	struct mpeg_pat_program *cur_program;
	(void) arg;

	// FIXME: tune the frontend

	// read the PAT
	while(!dvbthread_shutdown) {
		section_ext = read_section_ext(buf, sizeof(buf), dvb_adapter, demux_device, 0x00, 0x00, 100);
		if (section_ext != NULL) {
			break;
		}
	}
	if (dvbthread_shutdown) {
		return 0;
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
	while(!dvbthread_shutdown) {
        	// read the PMT
		struct section_ext *section_ext = read_section_ext(buf, sizeof(buf), dvb_adapter, demux_device,
								   pmt_pid, stag_mpeg_program_map, 100);
		if (section_ext == NULL) {
			continue;
		}
		struct mpeg_pmt_section *pmt = mpeg_pmt_section_codec(section_ext);
		if (pmt == NULL) {
			fprintf(stderr, "Bad PMT received\n");
			continue;
		}
		if (pmt->head.version_number == pmtversion) {
			continue;
		}

		// FIXME: change PID filters on PMT change

		// set the CA PMT if the CA resource is connected
		if (ca_resource_connected) {

			// translate it into a CA PMT
			int listmgmt = CA_LIST_MANAGEMENT_ONLY;
			if (pmtversion != -1) {
				listmgmt = CA_LIST_MANAGEMENT_UPDATE;
			}
			int size;
			if ((size = en50221_ca_format_pmt(pmt, capmt, sizeof(capmt),
							  move_to_programme,
							  listmgmt,
							  CA_PMT_CMD_ID_OK_DESCRAMBLING)) < 0) {
				fprintf(stderr, "Failed to format CA PMT object\n");
				continue;
			}

			// set it
			if (en50221_app_ca_pmt(ca_resource, ca_session_number, capmt, size)) {
				fprintf(stderr, "Failed to send CA PMT object\n");
				continue;
			}

			// remember the version
			pmtversion = pmt->head.version_number;
		}
	}
	return 0;
}

static struct section_ext *read_section_ext(char *buf, int buflen, int adapter, int demux, int pid, int table_id, int timeout)
{
	int demux_fd = -1;
	char filter[18];
	char mask[18];
	int size;
	struct section *section;
	struct section_ext *result = NULL;
	struct pollfd pollfd;

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

	// poll for data
	pollfd.fd = demux_fd;
	pollfd.events = POLLIN|POLLPRI|POLLERR;
	if (poll(&pollfd, 1, timeout) != 1) {
		return NULL;
	}
	if (pollfd.revents & POLLERR) {
		return NULL;
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






static int cazap_ai_callback(void *arg, uint8_t slot_id, uint16_t session_number,
			    uint8_t application_type, uint16_t application_manufacturer,
			    uint16_t manufacturer_code, uint8_t menu_string_length,
			    uint8_t *menu_string)
{
	(void) arg;
	(void) slot_id;
	(void) session_number;

	printf("Application type: %02x\n", application_type);
	printf("Application manufacturer: %04x\n", application_manufacturer);
	printf("Manufacturer code: %04x\n", manufacturer_code);
	printf("Menu string: %.*s\n", menu_string_length, menu_string);

	return 0;
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

static int cazap_mmi_close_callback(void *arg, uint8_t slot_id, uint16_t session_number,
				    uint8_t cmd_id, uint8_t delay)
{
	(void) arg;
	(void) slot_id;
	(void) session_number;
	(void) cmd_id;
	(void) delay;

	// note: not entirely correct as its supposed to delay if asked
	mmi_state = MMI_STATE_CLOSED;
	return 0;
}

static int cazap_mmi_display_control_callback(void *arg, uint8_t slot_id, uint16_t session_number,
					      uint8_t cmd_id, uint8_t mmi_mode)
{
	struct en502221_app_mmi_display_reply_details reply;
	(void) arg;
	(void) slot_id;

	// don't support any commands but set mode
	if (cmd_id != MMI_DISPLAY_CONTROL_CMD_ID_SET_MMI_MODE) {
		en50221_app_mmi_display_reply(mmi_resource, session_number,
					      MMI_DISPLAY_REPLY_ID_UNKNOWN_CMD_ID, &reply);
		return 0;
	}

	// we only support high level mode
	if (mmi_mode != MMI_MODE_HIGH_LEVEL) {
		en50221_app_mmi_display_reply(mmi_resource, session_number,
					      MMI_DISPLAY_REPLY_ID_UNKNOWN_MMI_MODE, &reply);
		return 0;
	}

	// ack the high level open
	reply.u.mode_ack.mmi_mode = mmi_mode;
	en50221_app_mmi_display_reply(mmi_resource, session_number,
				      MMI_DISPLAY_REPLY_ID_MMI_MODE_ACK, &reply);
	mmi_state = MMI_STATE_OPEN;
	return 0;
}

static int cazap_mmi_enq_callback(void *arg, uint8_t slot_id, uint16_t session_number,
				   uint8_t blind_answer, uint8_t expected_answer_length,
				   uint8_t *text, uint32_t text_size)
{
	(void) arg;
	(void) slot_id;
	(void) session_number;

	printf("%.*s: ", text_size, text);
	fflush(stdout);

	mmi_enq_blind = blind_answer;
	mmi_enq_length = expected_answer_length;
	mmi_state = MMI_STATE_ENQ;
	return 0;
}

static int cazap_mmi_menu_callback(void *arg, uint8_t slot_id, uint16_t session_number,
				   struct en50221_app_mmi_text *title,
				   struct en50221_app_mmi_text *sub_title,
				   struct en50221_app_mmi_text *bottom,
				   uint32_t item_count, struct en50221_app_mmi_text *items,
				   uint32_t item_raw_length, uint8_t *items_raw)
{
	(void) arg;
	(void) slot_id;
	(void) session_number;
	(void) item_raw_length;
	(void) items_raw;

	printf("------------------------------\n");

	if (title->text_length) {
		printf("%.*s\n", title->text_length, title->text);
	}
	if (sub_title->text_length) {
		printf("%.*s\n", sub_title->text_length, sub_title->text);
	}

	uint32_t i;
	printf("0. Quit menu\n");
	for(i=0; i< item_count; i++) {
		printf("%i. %.*s\n", i+1, items[i].text_length, items[i].text);
	}

	if (bottom->text_length) {
		printf("%.*s\n", bottom->text_length, bottom->text);
	}
	printf("Enter option: ");
	fflush(stdout);

	mmi_state = MMI_STATE_MENU;
	return 0;
}
