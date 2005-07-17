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

#ifndef _DVB_DEVICE_H_
#define _DVB_DEVICE_H_

#include <dvb/dvb.h>

enum dvb_device_type {
	DVB_FRONTEND,
	DVB_DEMUX,
	DVB_DVR,
	DVB_CA,
	DVB_AUDIO,
	DVB_VIDEO,
};

const char * dvb_devname(enum dvb_device_type device, int card, int sub);

int dvb_open_device(struct dvb_adapter * adapter,
		    enum dvb_device_type device, int sub);

//int dvb_get_fd(dvb_t dvb, enum dvb_device_type device, int sub);

#endif

