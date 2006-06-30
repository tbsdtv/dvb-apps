/*
	ZAP utility
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

#ifndef __CA_ZAP_H__
#define __CA_ZAP_H__

#include <stdlib.h>
#include <stdint.h>

#define OUTPUT_TYPE_DECODER 0
#define OUTPUT_TYPE_DECODER_ABYPASS 1
#define OUTPUT_TYPE_DVR 2
#define OUTPUT_TYPE_NULL 3
#define OUTPUT_TYPE_FILE 4
#define OUTPUT_TYPE_UDP 5

extern void new_dvb_pmt(struct mpeg_pmt_section *pmt);
extern void new_dvb_time(time_t dvb_time);

#endif
