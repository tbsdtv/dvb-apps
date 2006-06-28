/*
	ZAP utility

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

#define _FILE_OFFSET_BITS 64
#define _LARGEFILE_SOURCE 1
#define _LARGEFILE64_SOURCE 1

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/poll.h>
#include <libdvbapi/dvbdemux.h>
#include <libdvbapi/dvbaudio.h>
#include <libdvbcfg/dvbcfg_zapchannel.h>
#include <libdvbcfg/dvbcfg_sec.h>
#include <libucsi/mpeg/section.h>
#include "zap.h"
#include "zap_dvb.h"
#include "zap_ca.h"


static void signal_handler(int _signal);
static void *fileoutputthread_func(void* arg);

static int quit_app = 0;
static int fileoutputthread_shutdown = 0;
static pthread_t fileoutputthread;
static int dvrfd = -1;
static int outfd = -1;

void usage(void)
{
	static const char *_usage = "\n"
		" ZAP: A zapping application\n"
		" Copyright (C) 2004, 2005, 2006 Manu Abraham (manu@kromtek.com)\n"
		" Copyright (C) 2006 Andrew de Quincey (adq_dvb@lidskialf.net)\n\n"
		" usage: zap <options> as follows:\n"
		" -h			help\n"
		" -adapter <id>		adapter to use (default 0)\n"
		" -frontend <id>	frontend to use (default 0)\n"
		" -demux <id>		demux to use (default 0)\n"
		" -caslotnum <id>	ca slot number to use (default 0)\n"
		" -channels <filename>	channels.conf file.\n"
		" -secfile <filename>	Optional sec.conf file.\n"
		" -secid <secid>	ID of the SEC configuration to use, one of:\n"
		"			 * UNIVERSAL (default) - Europe, 10800 to 11800 MHz and 11600 to 12700 Mhz,\n"
		" 						 Dual LO, loband 9750, hiband 10600 MHz.\n"
		"			 * DBS - Expressvu, North America, 12200 to 12700 MHz, Single LO, 11250 MHz.\n"
		"			 * STANDARD - 10945 to 11450 Mhz, Single LO, 10000 Mhz.\n"
		"			 * ENHANCED - Astra, 10700 to 11700 MHz, Single LO, 9750 MHz.\n"
		"			 * C-BAND - Big Dish, 3700 to 4200 MHz, Single LO, 5150 Mhz.\n"
		"			 * One of the values defined in the secfile if supplied"
		" -out :decoder		Output to hardware decoder\n"
		"      :decoderabypass	Output to hardware decoder using audio bypass\n"
		"      :dvr		Output A/V only to dvr device\n"
		"      :dvrfull		Output complete stream to dvr device\n"
		"      :null		Do not output anything\n"
		"      :file <filename>	Output A/V only to file\n"
		"      :filefull <filename>	Output complete stream to file\n"
		" -timeout <secs>	Number of seconds to output channel for (0=>exit immediately after successful tuning)\n"
		" -cammenu		Show the CAM menu\n"
		" -moveca		Move CA descriptors from stream to programme level if possible\n"
		" <channel name>\n";
	fprintf(stderr, "%s\n", _usage);

	exit(1);
}

int main(int argc, char *argv[])
{
	int adapter_id = 0;
	int frontend_id = 0;
	int demux_id = 0;
	int caslot_num = 0;
	char *chanfile = "/etc/channels.conf";
	char *secfile = "/etc/sec.conf";
	char *secid = NULL;
	char *channel_name = NULL;
	int output_type = OUTPUT_TYPE_DECODER;
	char *outfile = NULL;
	int timeout = -1;
	int moveca = 0;
	int cammenu = 0;
	int argpos = 1;
	struct zap_dvb_params zap_dvb_params;
	struct zap_ca_params zap_ca_params;
	int ffaudiofd;

	while(argpos != argc) {
		if (!strcmp(argv[argpos], "-h")) {
			usage();
		} else if (!strcmp(argv[argpos], "-adapter")) {
			if ((argpos - argc) < 2)
				usage();
			if (sscanf(argv[argpos+1], "%i", &adapter_id) != 1)
				usage();
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-frontend")) {
			if ((argpos - argc) < 2)
				usage();
			if (sscanf(argv[argpos+1], "%i", &frontend_id) != 1)
				usage();
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-demux")) {
			if ((argpos - argc) < 2)
				usage();
			if (sscanf(argv[argpos+1], "%i", &demux_id) != 1)
				usage();
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-caslotnum")) {
			if ((argpos - argc) < 2)
				usage();
			if (sscanf(argv[argpos+1], "%i", &caslot_num) != 1)
				usage();
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-channels")) {
			if ((argpos - argc) < 2)
				usage();
			chanfile = argv[argpos+1];
			argpos+=2;

		} else if (!strcmp(argv[argpos], "-secfile")) {
			if ((argpos - argc) < 2)
				usage();
			secfile = argv[argpos+1];
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-secid")) {
			if ((argpos - argc) < 2)
				usage();
			secid = argv[argpos+1];
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-out")) {
			if ((argpos - argc) < 2)
				usage();
			if (!strcmp(argv[argpos+1], ":decoder")) {
				output_type = OUTPUT_TYPE_DECODER;
			} else if (!strcmp(argv[argpos+1], ":decoderabypass")) {
				output_type = OUTPUT_TYPE_DECODER_ABYPASS;
			} else if (!strcmp(argv[argpos+1], ":dvr")) {
				output_type = OUTPUT_TYPE_DVR;
			} else if (!strcmp(argv[argpos+1], ":dvrsi")) {
				output_type = OUTPUT_TYPE_DVR_FULL;
			} else if (!strcmp(argv[argpos+1], ":null")) {
				output_type = OUTPUT_TYPE_NULL;
			} else if (!strcmp(argv[argpos+1], ":file")) {
				output_type = OUTPUT_TYPE_FILE;
				if ((argpos - argc) < 3)
					usage();
				outfile = argv[argpos+2];
				argpos++;
			} else if (!strcmp(argv[argpos+1], ":filefull")) {
				output_type = OUTPUT_TYPE_FILE_FULL;
				if ((argpos - argc) < 3)
					usage();
				outfile = argv[argpos+2];
				argpos++;
			} else {
				usage();
			}
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-timeout")) {
			if ((argpos - argc) < 2)
				usage();
			if (sscanf(argv[argpos+1], "%i", &timeout) != 1)
				usage();
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-moveca")) {
			moveca = 1;
			argpos++;
		} else if (!strcmp(argv[argpos], "-cammenu")) {
			cammenu = 1;
			argpos++;
		} else {
			if ((argpos - argc) != 1)
				usage();
			channel_name = argv[argpos];
			argpos++;
		}
	}

	// find the requested channel
	if (channel_name != NULL) {
		if (dvbcfg_zapchannel_find(chanfile, channel_name, &zap_dvb_params.channel)) {
			fprintf(stderr, "Unable to find requested channel %s\n", channel_name);
			exit(1);
		}
	}

	// find the requested LNB/SEC setting
	if (channel_name != NULL) {
		// use a default with a DVBS card
		if ((secid == NULL) && (zap_dvb_params.channel.fe_type == DVBFE_TYPE_DVBS))
			secid = "UNIVERSAL";

		// look it up if one were supplied
		zap_dvb_params.valid_sec = 0;
		if (secid != NULL) {
			if (dvbcfg_sec_find(secfile, secid,
					&zap_dvb_params.sec)) {
				fprintf(stderr, "Unable to find suitable sec/lnb configuration for channel\n");
				exit(1);
			}
			zap_dvb_params.valid_sec = 1;
		}
	}

	// open the frontend
	zap_dvb_params.fe = dvbfe_open(adapter_id, frontend_id, 0);
	if (zap_dvb_params.fe == NULL) {
		fprintf(stderr, "Failed to open frontend\n");
		exit(1);
	}

	// setup any signals
	signal(SIGINT, signal_handler);

	// start the CA stuff
	zap_ca_params.adapter_id = adapter_id;
	zap_ca_params.caslot_num = caslot_num;
	zap_ca_params.cammenu = cammenu;
	zap_ca_params.moveca = moveca;
	zap_ca_start(&zap_ca_params);

	if (channel_name != NULL) {
		// start the DVB stuff
		zap_dvb_params.adapter_id = adapter_id;
		zap_dvb_params.frontend_id = frontend_id;
		zap_dvb_params.demux_id = demux_id;
		zap_dvb_params.output_type = output_type;
		zap_dvb_start(&zap_dvb_params);

		// setup output
		switch(output_type) {
		case OUTPUT_TYPE_DECODER:
		case OUTPUT_TYPE_DECODER_ABYPASS:
			ffaudiofd = dvbaudio_open(adapter_id, 0);
			if (ffaudiofd < 0) {
				fprintf(stderr, "Failed to open audio device\n");
				exit(1);
			}
			dvbaudio_set_bypass(ffaudiofd, (output_type == OUTPUT_TYPE_DECODER_ABYPASS) ? 1 : 0);
			close(ffaudiofd);
			break;

		case OUTPUT_TYPE_FILE:
		case OUTPUT_TYPE_FILE_FULL:
			// open output file
			outfd = open(outfile, O_WRONLY|O_CREAT|O_LARGEFILE, 0644);
			if (outfd < 0) {
				fprintf(stderr, "Failed to open output file\n");
				exit(1);
			}

			// open dvr device
			dvrfd = dvbdemux_open_dvr(adapter_id, 0, 1, 0);
			if (dvrfd < 0) {
				fprintf(stderr, "Failed to open DVR device\n");
				exit(1);
			}

			pthread_create(&fileoutputthread, NULL, fileoutputthread_func, NULL);
			break;
		}
	}

	// the UI
	time_t start = time(NULL);
	while(!quit_app) {
		// the timeout
		if (timeout != -1) {
			if ((time(NULL) - start) >= timeout)
				break;
		}

		if (cammenu)
			zap_ca_ui();
		else
			usleep(1);
	}

	// shutdown fileoutput thread if necessary
	if (dvrfd != -1) {
		fileoutputthread_shutdown = 1;
		pthread_join(fileoutputthread, NULL);
	}

	// shutdown DVB stuff
	if (channel_name != NULL)
		zap_dvb_stop();

	// shutdown CA stuff
	zap_ca_stop();

	// done
	exit(0);
}

static void signal_handler(int _signal)
{
	(void) _signal;

	if (!quit_app) {
		printf("Shutting down..\n");
		quit_app = 1;
	}
}

static void *fileoutputthread_func(void* arg)
{
	(void)arg;
	uint8_t buf[4096];
	struct pollfd pollfd;

	pollfd.fd = dvrfd;
	pollfd.events = POLLIN|POLLPRI|POLLERR;

	while(!fileoutputthread_shutdown) {
		if (poll(&pollfd, 1, 1000) != 1)
			continue;
		if (pollfd.revents & POLLERR) {
			fprintf(stderr, "DVR device read failure\n");
			return 0;
		}

		int size = read(dvrfd, buf, sizeof(buf));
		if (size < 0) {
			fprintf(stderr, "DVR device read failure\n");
			return 0;
		}

		write(outfd, buf, size);
	}

	return 0;
}
