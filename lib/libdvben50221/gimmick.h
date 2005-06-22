/*
	en50221 encoder An implementation for libdvb
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

#ifndef __GIMMICK_H__
#define __GIMMICK_H__

#include <stdlib.h>
#include <stdint.h>
#include "en50221_hlci.h"
#include "si.h"


uint16_t do_gimmick(struct en50221_pmt_object *p_en50221_pmt_object, struct service_info *p_si);
uint16_t do_tps_gimmick(struct en50221_pmt_object *p_en50221_pmt_object, struct service_info *p_si);


#endif
