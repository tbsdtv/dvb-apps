/**
 * dvbcfg_utils - dvbcfg misc utilities.
 *
 * Copyright (c) 2006 by Andrew de Quincey <adq_dvb@lidskialf.net>
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef DVBCFG_UTILS_H
#define DVBCFG_UTILS_H 1

struct dvbcfg_setting {
        char *name;
	int value;
};

extern int dvbcfg_parsesetting(char* text, const struct dvbcfg_setting* settings);
extern char* dvbcfg_lookupsetting(int setting, const struct dvbcfg_setting* settings);
extern void dvbcfg_curtoken(char *dest, int len, char *src, int delimiter);
extern char *dvbcfg_nexttoken(char *src, int delimiter);
extern int dvbcfg_issection(char* line, char* sectionname);
extern char* dvbcfg_iskey(char* line, char* keyname);

#endif
