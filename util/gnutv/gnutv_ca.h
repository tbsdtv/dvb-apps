/*
	gnutv utility CA functions

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

#ifndef gnutv_CA_H
#define gnutv_CA_H 1

struct gnutv_ca_params {
	int adapter_id;
	int caslot_num;
	int cammenu;
	int moveca;
};

extern void gnutv_ca_start(struct gnutv_ca_params *params);
extern void gnutv_ca_ui(void);
extern void gnutv_ca_stop(void);

extern int gnutv_ca_new_pmt(struct mpeg_pmt_section *pmt);
extern void gnutv_ca_new_dvbtime(time_t dvb_time);

#endif
