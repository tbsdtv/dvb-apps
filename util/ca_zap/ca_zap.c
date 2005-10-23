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
#include <string.h>
#include <linux/dvb/frontend.h>

#include "si.h"
#include "channels.h"
#include "pmt.h"
#include "en50221_hlci.h"

#include "ca_zap.h"

void usage(void)
{
	static const char *usage = "\n"
		" CA-ZAP: A High level CA zapping application based upon the HLCI implementation\n"
		" Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)\n\n"
		" usage: ca_zap options\n"
		" -h	help\n"
		" -a	adapter to use\n"
		" -c	channels.conf file\n"
		" -n	channel name to zap\n"
		" -t	frontend type, options: sat/cab/ter\n"
		" -d	demux to use\n"
		" -s	CA module (Slot to use)\n"
		" -f	Frontend to use\n"
		" -m    Move CA descriptors from stream to programme level if possible\n";

	fprintf(stderr, "%s\n", usage);

	exit(1);
}


#define MAX_PATH_LENGTH		80

int main(int argc, char *argv[])
{
	struct service_info *p_si = (struct service_info *) malloc(sizeof (struct service_info));
	struct channel_params *p_channel_params = (struct channel_params *) malloc(sizeof (struct channel_params));
	struct en50221_pmt_object *p_en50221_pmt_object = p_en50221_pmt_object = (struct en50221_pmt_object *) malloc( sizeof (struct en50221_pmt_object));

	char chanfile[MAX_PATH_LENGTH];
	char channel_name[20];
	char fe_type_name[4];
	char adapter[80], frontend_dev[80], demux_dev[80], ca_dev[80];
	uint8_t dev = 0, fe_type = 0, demux = 0, ca = 0, frontend = 0;
	int opt;
	int move_to_programme = 0;

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

			case 't':
				strncpy(fe_type_name, optarg, sizeof(fe_type_name));
				if (strncmp(fe_type_name, "sat", sizeof(fe_type_name)) == 0)
					fe_type = 0x01;
				else if (strncmp(fe_type_name, "cab", sizeof(fe_type_name)) == 0)
					fe_type = 0x02;
				else if (strncmp(fe_type_name, "ter", sizeof(fe_type_name)) == 0)
					fe_type = 0x03;

				else {
					printf(" ERROR: Invalid Frontend type %s\n", fe_type_name);
					exit(-1);
				}
				break;

			case 'd':
				demux = strtoul(optarg, NULL, 0);
				break;

			case 's':
				ca = strtoul(optarg, NULL, 0);
				break;

			case 'f':
				frontend = strtoul(optarg, NULL, 0);
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

	snprintf(adapter, sizeof(adapter), "/dev/dvb/adapter%i", dev);
	snprintf(frontend_dev, sizeof(frontend_dev), "/dev/dvb/adapter%i/frontend%i", dev, frontend);
	snprintf(demux_dev, sizeof(demux_dev), "/dev/dvb/adapter%i/demux%i", dev, demux);
	snprintf(ca_dev, sizeof(ca_dev), "/dev/dvb/adapter%i/ca%i", dev, ca);
	printf("Using Adpater=[%s]\n Frontend=[%s]\n Demux=[%s]\n Slot=[%s]\n", adapter, frontend_dev, demux_dev, ca_dev);

	if (p_channel_params == NULL) {
		printf("Could not allocate memory.\n");
		exit(-1);
	}

	parse_channel_list(p_channel_params, chanfile, channel_name, fe_type);
	parse_si(p_si, p_channel_params, adapter, frontend_dev, demux_dev);
	if (p_en50221_pmt_object != NULL) {
		do_en50221_pmt_object(p_en50221_pmt_object, p_si, ONLY, OK_DESCRAMBLING, move_to_programme);
		write_en50221_pmt_object(p_en50221_pmt_object, ca_dev);

	} else {
		printf("%s: Memory allocation failed\n", __FUNCTION__);
		return -1;
	}

	return 0;
}
