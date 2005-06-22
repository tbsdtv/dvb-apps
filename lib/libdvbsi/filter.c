/*
	filter setup, An implementation for libdvb
	an implementation for the High Level Common Interface

	Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)
	Large parts of the code adapted from test_sections.c from dvb-apps/test


	This library is free software; you can redistribute it and/or modify
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <stdint.h>
#include <errno.h>
#include <linux/dvb/dmx.h>
#include "filter.h"

int add_filter(int dmxfd, unsigned long pid, int tid, int tid_ext)
{
	unsigned char filter[DMX_FILTER_SIZE];
	unsigned char mask[DMX_FILTER_SIZE];
	struct dmx_sct_filter_params f;

	memset(filter, '\0', sizeof(filter));
	memset(mask, '\0', sizeof(mask));

	if (tid < 0x100) {
		filter[0] = tid;
		mask[0] = 0xff;
	}

	if (tid_ext) {
		filter[1] = (uint8_t ) ((tid_ext >> 8) & 0xff);
		filter[2] = (uint8_t ) (tid_ext & 0xff);
		mask[1] = 0xff;
		mask[2] = 0xff;
	}

	memset(&f.filter, 0, sizeof(struct dmx_filter));
	f.pid = (uint16_t) pid;
	memcpy(f.filter.filter, filter, DMX_FILTER_SIZE);
	memcpy(f.filter.mask, mask, DMX_FILTER_SIZE);

	f.timeout = 0;

	f.flags = DMX_IMMEDIATE_START | DMX_CHECK_CRC;

	if (ioctl(dmxfd, DMX_SET_FILTER, &f) == -1) {
		perror("DMX_SET_FILTER");
		return 1;
	}

	return dmxfd;
}
