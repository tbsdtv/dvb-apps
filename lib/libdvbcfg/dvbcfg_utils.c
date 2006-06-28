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

#include <string.h>
#include <ctype.h>
#include "dvbcfg_utils.h"

int dvbcfg_parsesetting(char* text, const struct dvbcfg_setting* settings)
{
	char tmp[128];

	dvbcfg_curtoken(tmp, sizeof(tmp), text, ':');

	while(settings->name) {
		if (!strcmp(tmp, settings->name))
			return settings->value;

		settings++;
	}

	return -1;
}

char* dvbcfg_lookupsetting(int setting, const struct dvbcfg_setting* settings)
{
	while(settings->name) {
		if (setting == settings->value)
			return settings->name;

		settings++;
	}

	return NULL;
}

void dvbcfg_curtoken(char *dest, int len, char *src, int delimiter)
{
	while((len > 1) && (*src) && (*src != delimiter)) {
		*dest++ = *src++;
		len--;
	}
	*dest = 0;
}

char *dvbcfg_nexttoken(char *src, int delimiter)
{
	while(*src && (*src != delimiter))
		src++;
	if (*src == 0)
		return NULL;

	src++;
	if (*src)
		return src;
	return NULL;
}


int dvbcfg_issection(char* line, char* sectionname)
{
	int len;

	len = strlen(line);
	if (len < 2)
		return 0;

	if ((line[0] != '[') || (line[len-1] != ']'))
		return 0;

	line++;
	while(isspace(*line))
		line++;

	if (strncmp(line, sectionname, strlen(sectionname)))
		return 0;

	return 1;
}

char* dvbcfg_iskey(char* line, char* keyname)
{
	int len = strlen(keyname);

	/* does the key match? */
	if (strncmp(line, keyname, len))
		return NULL;

	/* skip keyname & any whitespace */
	line += len;
	while(isspace(*line))
		line++;

	/* should be the '=' sign */
	if (*line != '=')
		return 0;

	/* more whitespace skipping */
	line++;
	while(isspace(*line))
		line++;

	/* finally, return the value */
	return line;
}
