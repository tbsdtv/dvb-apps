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

#ifndef _UCSI_DVB_TYPES_H
#define _UCSI_DVB_TYPES_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <time.h>

typedef uint8_t dvbdate_t[5];
typedef uint8_t dvbduration_t[3];

/**
 * Convert from a 5 byte DVB UTC date to unix time.
 * Note: this functions expects the DVB date in network byte order.
 *
 * @param d Pointer to DVB date.
 * @return The unix timestamp, or -1 if the dvbdate was set to the 'undefined' value
 */
extern time_t dvbdate_to_unixtime(dvbdate_t dvbdate);

/**
 * Convert from a unix timestemp to a 5 byte DVB UTC date.
 * Note: this function will always output the DVB date in
 * network byte order.
 *
 * @param unixtime The unix timestamp, or -1 for the 'undefined' value.
 * @param utc Pointer to 5 byte DVB date.
 */
extern void unixtime_to_dvbdate(time_t unixtime, dvbdate_t dvbdate);

/**
 * Convert from a DVB BCD duration to a number of seconds.
 *
 * @param dvbduration Pointer to 3 byte DVB duration.
 * @return Number of seconds.
 */
extern int dvbduration_to_seconds(dvbduration_t dvbduration);

/**
 * Convert from a number of seconds to a DVB 3 byte BCD duration.
 *
 * @param seconds The number of seconds.
 * @param dvbduration Pointer to 3 byte DVB duration.
 */
extern void seconds_to_dvbduration(int seconds, dvbduration_t dvbduration);

#ifdef __cplusplus
}
#endif

#endif
