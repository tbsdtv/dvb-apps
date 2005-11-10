/*
 * libdvbnet - a DVB network support library
 *
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/param.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "dvbnet.h"

int dvbnet_open(int adapter, int netdeviceid)
{
	char filename[PATH_MAX+1];

	sprintf(filename, "/dev/dvb/adapter%i/net%i", adapter, netdeviceid);

	return open(filename, O_RDWR);
}

int dvbnet_add_interface(int fd, uint16_t pid, int encapsulation)
{
	struct dvb_net_if params;

	memset(&params, 0, sizeof(params));
	params.pid = pid;
	params.feedtype = encapsulation;
	return ioctl(fd, NET_ADD_IF, &params);
}

int dvbnet_get_interface(int fd, int ifnum, struct dvb_net_if* info)
{
	memset(info, 0, sizeof(struct dvb_net_if));
	info->if_num = ifnum;
	return ioctl(fd, NET_GET_IF, info);
}

int dvbnet_remove_interface(int fd, int ifnum)
{
	return ioctl(fd, NET_REMOVE_IF, ifnum);
}
