/*
 * section and descriptor parser
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

#include <string.h>
#include "types.h"

struct atsc_text *atsc_text_parse(uint8_t *buf, int len)
{
	// FIXME: implement
	return -1;
}

time_t atsctime_to_unixtime(atsctime_t atsc)
{
	// FIXME: implement
	return 0;
}

atsctime_t unixtime_to_atsctime(time_t atsc)
{
	// FIXME: implement
	return 0;
}
