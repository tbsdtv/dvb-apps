/* femon -- monitor frontend status
 *
 * Copyright (C) 2003 convergence GmbH
 * Johannes Stezenbach <js@convergence.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <stdint.h>
#include <sys/time.h>

#include <libdvbapi/dvbfe.h>

#define FE_STATUS_PARAMS (DVBFE_INFO_LOCKSTATUS|DVBFE_INFO_SIGNAL_STRENGTH|DVBFE_INFO_BER|DVBFE_INFO_SNR|DVBFE_INFO_UNCORRECTED_BLOCKS)

static char *usage_str =
    "\nusage: femon [options]\n"
    "     -h        : human readable output\n"
    "     -a number : use given adapter (default 0)\n"
    "     -f number : use given frontend (default 0)\n"
    "     -c number : samples to take (default 0 = infinite)\n\n";


static void usage(void)
{
	fprintf(stderr, usage_str);
	exit(1);
}


static
int check_frontend (struct dvbfe_handle *fe, int human_readable, unsigned int count)
{
	struct dvbfe_info fe_info;
	unsigned int samples = 0;

	do {
		if (dvbfe_get_info(fe, FE_STATUS_PARAMS, &fe_info, DVBFE_INFO_QUERYTYPE_IMMEDIATE, 0) != FE_STATUS_PARAMS) {
			fprintf(stderr, "Problem retrieving frontend information: %m\n");
		}



		if (human_readable) {
                       printf ("status %c%c%c%c%c | signal %3u%% | snr %3u%% | ber %08x | unc %08x | ",
				fe_info.signal ? 'S' : ' ',
				fe_info.carrier ? 'C' : ' ',
				fe_info.viterbi ? 'V' : ' ',
				fe_info.sync ? 'Y' : ' ',
				fe_info.lock ? 'L' : ' ',
				(fe_info.signal_strength * 100) / 0xffff,
				(fe_info.snr * 100) / 0xffff,
				fe_info.ber,
				fe_info.ucblocks);
		} else {
			printf ("status %c%c%c%c%c | signal %04x | snr %04x | ber %08x | unc %08x | ",
				fe_info.signal ? 'S' : ' ',
				fe_info.carrier ? 'C' : ' ',
				fe_info.viterbi ? 'V' : ' ',
				fe_info.sync ? 'Y' : ' ',
				fe_info.lock ? 'L' : ' ',
				fe_info.signal_strength,
				fe_info.snr,
				fe_info.ber,
				fe_info.ucblocks);
		}

		if (fe_info.lock)
			printf("FE_HAS_LOCK");

		printf("\n");
		fflush(stdout);
		usleep(1000000);
		samples++;
	} while ((!count) || (count-samples));

	return 0;
}


static
int do_mon(unsigned int adapter, unsigned int frontend, int human_readable, unsigned int count)
{
	int result;
	struct dvbfe_handle *fe;
	struct dvbfe_info fe_info;
	char *fe_type = "UNKNOWN";

	fe = dvbfe_open(adapter, frontend, 1);
	if (fe == NULL) {
		perror("opening frontend failed");
		return 0;
	}

	dvbfe_get_info(fe, 0, &fe_info, DVBFE_INFO_QUERYTYPE_IMMEDIATE, 0);
	switch(fe_info.type) {
	case DVBFE_TYPE_DVBS:
		fe_type = "DVBS";
		break;
	case DVBFE_TYPE_DVBC:
		fe_type = "DVBC";
		break;
	case DVBFE_TYPE_DVBT:
		fe_type = "DVBT";
		break;
	case DVBFE_TYPE_ATSC:
		fe_type = "ATSC";
		break;
	}
	printf("FE: %s (%s)\n", fe_info.name, fe_type);

	result = check_frontend (fe, human_readable, count);

	dvbfe_close(fe);

	return result;
}

int main(int argc, char *argv[])
{
	unsigned int adapter = 0, frontend = 0, count = 0;
	int human_readable = 0;
	int opt;

       while ((opt = getopt(argc, argv, "ha:f:c:")) != -1) {
		switch (opt)
		{
		default:
			usage();
			break;
		case 'a':
			adapter = strtoul(optarg, NULL, 0);
			break;
		case 'c':
			count = strtoul(optarg, NULL, 0);
			break;
		case 'f':
			frontend = strtoul(optarg, NULL, 0);
			break;
		case 'h':
			human_readable = 1;
			break;
		}
	}

	do_mon(adapter, frontend, human_readable, count);

	return 0;
}

