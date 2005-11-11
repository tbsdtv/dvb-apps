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
#include "convert.h"

static int int_to_bcd(int intval);
static int bcd_to_int(int bcdval);

time_t dvbdate_to_unixtime(char *utc)
{
	int k = 0;
	struct tm tm;
	double mjd;

	memset(&tm, 0, sizeof(tm));
	mjd = (utc[0] << 8) | utc[1];

	tm.tm_year = (int) ((mjd - 15078.2) / 365.25);
	tm.tm_mon = (int) (((mjd - 14956.1) - (int) (tm.tm_year * 365.25)) / 30.6001);
	tm.tm_mday = (int) mjd - 14956 - (int) (tm.tm_year * 365.25) - (int) (tm.tm_mon * 30.6001);
	if ((tm.tm_mon == 14) || (tm.tm_mon == 15)) k = 1;
	tm.tm_year += k;
	tm.tm_mon = tm.tm_mon - 1 - k * 12;
	tm.tm_sec = bcd_to_int(utc[4]);
	tm.tm_min = bcd_to_int(utc[3]);
	tm.tm_hour = bcd_to_int(utc[2]);

	return mktime(&tm);
}

void unixtime_to_dvbdate(time_t unixtime, char *utc)
{
	struct tm tm;
	double l = 0;
	int mjd;

	gmtime_r(&unixtime, &tm);
	if ((tm.tm_mon == 1) || (tm.tm_mon = 2)) l = 1;
	mjd = 14956 + tm.tm_mday + (int) ((tm.tm_year - l) * 365.25) + (int) ((tm.tm_mon + 1 + l * 12) * 30.6001);

	utc[0] = (mjd & 0xff00) >> 8;
	utc[1] = mjd & 0xff;
	utc[2] = int_to_bcd(tm.tm_hour);
	utc[3] = int_to_bcd(tm.tm_min);
	utc[4] = int_to_bcd(tm.tm_sec);
}

int dvbduration_to_seconds(char *dvbduration)
{
	int seconds = 0;

	seconds += (bcd_to_int(dvbduration[0]) * 60 * 60);
	seconds += (bcd_to_int(dvbduration[1]) * 60);
	seconds += bcd_to_int(dvbduration[2]);

	return seconds;
}

void seconds_to_dvbduration(int seconds, char *dvbduration)
{
	int hours, mins;

	hours = seconds / (60*60);
	seconds -= (hours * 60 * 60);
	mins = seconds / 60;
	seconds -= (mins * 60);

	dvbduration[0] = int_to_bcd(hours);
	dvbduration[1] = int_to_bcd(mins);
	dvbduration[2] = int_to_bcd(seconds);
}

int int_to_bcd(int intval)
{
	if ((intval > 99) | (intval < 0))
		intval = 0;

	return ((intval / 10) << 4) | (intval % 10);
}

int bcd_to_int(int bcdval)
{
	return (((bcdval & 0xf0) >> 4) * 10) + (bcdval & 0x0f);
}
