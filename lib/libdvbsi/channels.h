/*
	channels.conf parser implementation for libdvb

	Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)

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

#ifndef __CHANNELS_H__
#define __CHANNELS_H__

#include <stdlib.h>
#include <unistd.h>
#include <linux/dvb/frontend.h>

struct channel_params {
	char channel[20];
	uint32_t frequency;
	char polarity;
	int inversion;
	uint8_t sat_no;
	int bandwidth;
	uint32_t symbol_rate;
	int fec;
	int code_rate_hp;
	int code_rate_lp;
	int modulation;
	int constellation;
	int transmission_mode;
	int guard_interval;
	int hierarchy;
	uint32_t video_pid;
	uint32_t audio_pid;
	uint32_t service_id;
};

uint16_t parse_channel_list(struct channel_params *p_channel_params, char *channel_list_file, char *channel_name, uint8_t fe_type);


#endif
