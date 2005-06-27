/*
	libdvbsi, A SI parser implementation for libdvb
	an implementation for the High Level Common Interface

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

#ifndef __BAT_H__
#define __BAT_H__

#include <stdlib.h>
#include <stdint.h>
#include "descriptor.h"


struct bat_loop {
	unsigned transport_stream_id: 16;
	unsigned original_network_id: 16;
	unsigned reserved_1: 4;

	unsigned transport_descriptors_length: 12;
	void *p_descriptor;
};

struct bouquet_association_section {
	unsigned table_id: 8;
	unsigned section_synatx_indicator: 1;
	unsigned reserved_1: 1;
	unsigned reserved_2: 2;
	unsigned section_length: 12;
	unsigned bouquet_id: 16;
	unsigned reserved_3: 2;
	unsigned version_number: 5;
	unsigned current_next_indicator: 1;
	unsigned section_number: 8;
	unsigned last_section_number: 8;
	unsigned reserved_4: 4;

	unsigned bouquet_descriptors_length: 12;
	void *p_descriptor;

	unsigned reserved_5: 4;

	unsigned transport_stream_loop_length: 12;
	struct bat_loop *p_bat_loop;
};

#endif
