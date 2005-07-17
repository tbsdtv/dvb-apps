/*
 * libdvb2 - dvb helper and wrapper library
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <dvb/internal.h>
#include <dvb/device.h>

#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>

const char * dvb_devname(enum dvb_device_type device, int card, int sub)
{
	static char devstr[128];

	switch (device) {
		case DVB_FRONTEND:
			snprintf(devstr, sizeof(devstr),
				 "/dev/dvb/adapter%i/frontend%i", card, sub);
			break;

		case DVB_DEMUX:
			snprintf(devstr, sizeof(devstr),
				 "/dev/dvb/adapter%i/demux%i", card, sub);
			break;

		case DVB_DVR:
			snprintf(devstr, sizeof(devstr),
				 "/dev/dvb/adapter%i/dvr%i", card, sub);
			break;

		case DVB_CA:
			snprintf(devstr, sizeof(devstr),
				 "/dev/dvb/adapter%i/ca%i", card, sub);
			break;

		case DVB_AUDIO:
			snprintf(devstr, sizeof(devstr),
				 "/dev/dvb/adapter%i/audio%i", card, sub);
			break;

		case DVB_VIDEO:
			snprintf(devstr, sizeof(devstr),
				 "/dev/dvb/adapter%i/video%i", card, sub);
			break;

		default:
			return NULL;
	}

	return devstr;
}

int dvb_open_device(struct dvb_adapter * adapter,
		    enum dvb_device_type device, int sub)
{
	const char * dev = dvb_devname(device, adapter->num, sub);

	if (dev == NULL)
		return -EINVAL;

	return open(dev, O_RDWR | O_NONBLOCK);
}

#if 0
int dvb_get_fd(dvb_t dvb, enum dvb_device_type device, int sub)
{
	if (dvb->fd[device][sub] < 0) {
		const char * dev = dvb_devname(device, dvb->num, sub);

		if (dev == NULL)
			return -EINVAL;

		dvb->fd[device][sub] = open(dev, O_RDWR | O_NONBLOCK);
	}

	return dvb->fd[device][sub];
}
#endif

