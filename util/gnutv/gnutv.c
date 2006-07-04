/*
	gnutv utility

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
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <sys/poll.h>
#include <libdvbapi/dvbdemux.h>
#include <libdvbapi/dvbaudio.h>
#include <libdvbcfg/dvbcfg_zapchannel.h>
#include <libdvbcfg/dvbcfg_sec.h>
#include <libucsi/mpeg/section.h>
#include "gnutv.h"
#include "gnutv_dvb.h"
#include "gnutv_ca.h"
#include "gnutv_data.h"


static void signal_handler(int _signal);

static int quit_app = 0;

void usage(void)
{
	static const char *_usage = "\n"
		" gnutv: A gnutvping application\n"
		" Copyright (C) 2004, 2005, 2006 Manu Abraham (manu@kromtek.com)\n"
		" Copyright (C) 2006 Andrew de Quincey (adq_dvb@lidskialf.net)\n\n"
		" usage: gnutv <options> as follows:\n"
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
		"			 * C-MULTI - Big Dish - Multipoint LNBf, 3700 to 4200 MHz,\n"
		"						Dual LO, H:5150MHz, V:5750MHz.\n"
		"			 * One of the sec definitions from the secfile if supplied\n"
		" -out decoder		Output to hardware decoder (default)\n"
		"      decoderabypass	Output to hardware decoder using audio bypass\n"
		"      dvr		Output stream to dvr device\n"
		"      null		Do not output anything\n"
		"      file <filename>	Output stream to file\n"
		"      udp <ip> <port>	Output stream to ip:port using udp\n"
		"      udpif <ip> <port> <interface> Output stream to ip:port using udp forcing the specified interface\n"
		"      rtp <ip> <port>	Output stream to ip:port using udp-rtp\n"
		"      rtpif <ip> <port> <interface> Output stream to ip:port using udp-rtp forcing the specified interface\n"
		" -timeout <secs>	Number of seconds to output channel for (0=>exit immediately after successful tuning, default is to output forever)\n"
		" -cammenu		Show the CAM menu\n"
		" -nomoveca		Do not attempt to move CA descriptors from stream to programme level\n"
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
	char *secfile = NULL;
	char *secid = NULL;
	char *channel_name = NULL;
	int output_type = OUTPUT_TYPE_DECODER;
	char *outfile = NULL;
	struct sockaddr_in outaddr;
	char *outif = NULL;
	int timeout = -1;
	int moveca = 1;
	int cammenu = 0;
	int argpos = 1;
	struct gnutv_dvb_params gnutv_dvb_params;
	struct gnutv_ca_params gnutv_ca_params;
	int ffaudiofd = -1;
	int usertp = 0;

	while(argpos != argc) {
		if (!strcmp(argv[argpos], "-h")) {
			usage();
		} else if (!strcmp(argv[argpos], "-adapter")) {
			if ((argc - argpos) < 2)
				usage();
			if (sscanf(argv[argpos+1], "%i", &adapter_id) != 1)
				usage();
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-frontend")) {
			if ((argc - argpos) < 2)
				usage();
			if (sscanf(argv[argpos+1], "%i", &frontend_id) != 1)
				usage();
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-demux")) {
			if ((argc - argpos) < 2)
				usage();
			if (sscanf(argv[argpos+1], "%i", &demux_id) != 1)
				usage();
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-caslotnum")) {
			if ((argc - argpos) < 2)
				usage();
			if (sscanf(argv[argpos+1], "%i", &caslot_num) != 1)
				usage();
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-channels")) {
			if ((argc - argpos) < 2)
				usage();
			chanfile = argv[argpos+1];
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-secfile")) {
			if ((argc - argpos) < 2)
				usage();
			secfile = argv[argpos+1];
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-secid")) {
			if ((argc - argpos) < 2)
				usage();
			secid = argv[argpos+1];
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-out")) {
			if ((argc - argpos) < 2)
				usage();
			if (!strcmp(argv[argpos+1], "decoder")) {
				output_type = OUTPUT_TYPE_DECODER;
			} else if (!strcmp(argv[argpos+1], "decoderabypass")) {
				output_type = OUTPUT_TYPE_DECODER_ABYPASS;
			} else if (!strcmp(argv[argpos+1], "dvr")) {
				output_type = OUTPUT_TYPE_DVR;
			} else if (!strcmp(argv[argpos+1], "null")) {
				output_type = OUTPUT_TYPE_NULL;
			} else if (!strcmp(argv[argpos+1], "file")) {
				output_type = OUTPUT_TYPE_FILE;
				if ((argc - argpos) < 3)
					usage();
				outfile = argv[argpos+2];
				argpos++;
			} else if ((!strcmp(argv[argpos+1], "udp")) ||
				   (!strcmp(argv[argpos+1], "rtp"))) {
				output_type = OUTPUT_TYPE_UDP;
				if ((argc - argpos) < 4)
					usage();

				if (!strcmp(argv[argpos+1], "rtp"))
					usertp = 1;

				outaddr.sin_family = AF_INET;
				if (!inet_aton(argv[argpos+2], &outaddr.sin_addr))
					usage();
				if ((outaddr.sin_port = atoi(argv[argpos+3])) == 0)
					usage();
				outaddr.sin_port = htons(outaddr.sin_port);
				argpos+=2;
			} else if ((!strcmp(argv[argpos+1], "udpif")) ||
				   (!strcmp(argv[argpos+1], "rtpif"))) {
				output_type = OUTPUT_TYPE_UDP;
				if ((argc - argpos) < 5)
					usage();

				if (!strcmp(argv[argpos+1], "rtpif"))
					usertp = 1;

				outaddr.sin_family = AF_INET;
				if (!inet_aton(argv[argpos+2], &outaddr.sin_addr))
					usage();
				if ((outaddr.sin_port = atoi(argv[argpos+3])) == 0)
					usage();
				outaddr.sin_port = htons(outaddr.sin_port);
				outif = argv[argpos+4];
				argpos+=3;
			} else {
				usage();
			}
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-timeout")) {
			if ((argc - argpos) < 2)
				usage();
			if (sscanf(argv[argpos+1], "%i", &timeout) != 1)
				usage();
			argpos+=2;
		} else if (!strcmp(argv[argpos], "-nomoveca")) {
			moveca = 0;
			argpos++;
		} else if (!strcmp(argv[argpos], "-cammenu")) {
			cammenu = 1;
			argpos++;
		} else {
			if ((argc - argpos) != 1)
				usage();
			channel_name = argv[argpos];
			argpos++;
		}
	}

	// the user didn't select anything!
	if ((channel_name == NULL) && (!cammenu))
		usage();

	// setup any signals
	signal(SIGINT, signal_handler);
	signal(SIGPIPE, SIG_IGN);

	// start the CA stuff
	gnutv_ca_params.adapter_id = adapter_id;
	gnutv_ca_params.caslot_num = caslot_num;
	gnutv_ca_params.cammenu = cammenu;
	gnutv_ca_params.moveca = moveca;
	gnutv_ca_start(&gnutv_ca_params);

	// frontend setup if a channel name was supplied
	if ((!cammenu) && (channel_name != NULL)) {
		// find the requested channel
		if (dvbcfg_zapchannel_find(chanfile, channel_name, &gnutv_dvb_params.channel)) {
			fprintf(stderr, "Unable to find requested channel %s\n", channel_name);
			exit(1);
		}

		// default SEC with a DVBS card
		if ((secid == NULL) && (gnutv_dvb_params.channel.fe_type == DVBFE_TYPE_DVBS))
			secid = "UNIVERSAL";

		// look it up if one were supplied
		gnutv_dvb_params.valid_sec = 0;
		if (secid != NULL) {
			if (dvbcfg_sec_find(secfile, secid,
					&gnutv_dvb_params.sec)) {
				fprintf(stderr, "Unable to find suitable sec/lnb configuration for channel\n");
				exit(1);
			}
			gnutv_dvb_params.valid_sec = 1;
		}

		// open the frontend
		gnutv_dvb_params.fe = dvbfe_open(adapter_id, frontend_id, 0);
		if (gnutv_dvb_params.fe == NULL) {
			fprintf(stderr, "Failed to open frontend\n");
			exit(1);
		}

		// failover decoder to dvr output if decoder not available
		if ((output_type == OUTPUT_TYPE_DECODER) ||
		    (output_type == OUTPUT_TYPE_DECODER_ABYPASS)) {
			ffaudiofd = dvbaudio_open(adapter_id, 0);
			if (ffaudiofd < 0) {
				fprintf(stderr, "Cannot open decoder; defaulting to dvr output\n");
				output_type = OUTPUT_TYPE_DVR;
			}
		}

		// start the DVB stuff
		gnutv_dvb_params.adapter_id = adapter_id;
		gnutv_dvb_params.frontend_id = frontend_id;
		gnutv_dvb_params.demux_id = demux_id;
		gnutv_dvb_params.output_type = output_type;
		gnutv_dvb_start(&gnutv_dvb_params);

		// start the data stuff
		gnutv_data_start(output_type, ffaudiofd, adapter_id, demux_id, outfile, outif, outaddr, usertp);
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
			gnutv_ca_ui();
		else
			usleep(1);
	}

	// stop data handling
	gnutv_data_stop();

	// shutdown DVB stuff
	if (channel_name != NULL)
		gnutv_dvb_stop();

	// shutdown CA stuff
	gnutv_ca_stop();

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
