/**
 * dvbcfg_sec (i.e. linuxtv sec format) configuration file support.
 *
 * Copyright (c) 2005 by Andrew de Quincey <adq_dvb@lidskialf.net>
 *
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

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "dvbcfg_sec.h"
#include "dvbcfg_utils.h"

int dvbcfg_sec_load(FILE *f,
		    void *private,
		    dvbcfg_sec_callback cb)
{
	struct dvbcfg_sec tmpsec;
	char *linebuf = NULL;
	size_t line_size = 0;
	int len;

	/* process each line */
	while((len = getline(&linebuf, &line_size, f)) > 0) {
		char *line = linebuf;

		/* chop any comments */
		char *hashpos = strchr(line, '#');
		if (hashpos)
			*hashpos = 0;
		char *lineend = line + strlen(line);

		/* trim the line */
		while(*line && isspace(*line))
			line++;
		while((lineend != line) && isspace(*(lineend-1)))
			lineend--;
		*lineend = 0;

		/* skip blank lines */
		if (*line == 0)
			continue;
		memset(&tmpsec, 0, sizeof(tmpsec));

		/* get the sec_id */
		dvbcfg_curtoken(tmpsec.sec_id, sizeof(tmpsec.sec_id), line, ' ');
		if ((line = dvbcfg_nexttoken(line, ' ')) == NULL)
			continue;

		/* the slof */
		if (sscanf(line, "%i", &tmpsec.slof) != 1)
			continue;
		if ((line = dvbcfg_nexttoken(line, ' ')) == NULL)
			continue;

		/* the polarization */
		switch(tolower(line[0])) {
		case 'h':
			tmpsec.polarization = DVBFE_POLARIZATION_H;
			break;
		case 'v':
			tmpsec.polarization = DVBFE_POLARIZATION_V;
			break;
		case 'l':
			tmpsec.polarization = DVBFE_POLARIZATION_L;
			break;
		case 'r':
			tmpsec.polarization = DVBFE_POLARIZATION_R;
			break;
		default:
			continue;
		}
		if ((line = dvbcfg_nexttoken(line, ' ')) == NULL)
			continue;

		/* the LOF */
		if (sscanf(line, "%i", &tmpsec.lof) != 1)
			continue;
		if ((line = dvbcfg_nexttoken(line, ' ')) == NULL)
			continue;

		/* the associated command NOTE: already null terminated */
		strncpy(tmpsec.command, line, sizeof(tmpsec.command)-1);

		// tell caller
		if (cb(private, &tmpsec))
			break;
	}

	if (linebuf)
		free(linebuf);
	return 0;
}

static int dvbcfg_sec_find_callback(void *private, struct dvbcfg_sec *sec);

struct findparams {
	const char *sec_id;
	uint32_t frequency;
	enum dvbfe_polarization polarization;
	struct dvbcfg_sec *sec_dest;
};

int dvbcfg_sec_find(const char *config_file,
		    const char *sec_id,
		    uint32_t frequency,
		    enum dvbfe_polarization polarization,
		    struct dvbcfg_sec *sec)
{
	struct findparams findp;

	// open the file
	FILE *f = fopen(config_file, "r");
	if (f == NULL)
		return -EIO;

	// parse each entry
	memset(sec, 0, sizeof(struct dvbcfg_sec));
	findp.sec_id = sec_id;
	findp.frequency = frequency;
	findp.polarization = polarization;
	findp.sec_dest = sec;
	dvbcfg_sec_load(f, &findp, dvbcfg_sec_find_callback);

	// done
	fclose(f);
	if (sec->sec_id[0])
		return 0;
	return -1;
}

static int dvbcfg_sec_find_callback(void *private, struct dvbcfg_sec *sec)
{
	struct findparams *findp = private;

	if (strcmp(findp->sec_id, sec->sec_id))
		return 0;
	if (findp->frequency > sec->slof)
		return 0;
	if (findp->polarization != sec->polarization)
		return 0;

	memcpy(findp->sec_dest, sec, sizeof(struct dvbcfg_sec));
	return 1;
}

int dvbcfg_sec_save(FILE *f,
		    struct dvbcfg_sec *secs,
		    int count)
{
	int i;
	char polarization = ' ';

	for(i=0; i<count; i++) {
		fprintf(f,  "%s ", secs[i].sec_id);
		fprintf(f,  "%ul ", secs[i].slof);

		switch(secs[i].polarization) {
		case DVBFE_POLARIZATION_H:
			polarization = 'h';
			break;

		case DVBFE_POLARIZATION_V:
			polarization = 'v';
			break;

		case DVBFE_POLARIZATION_L:
			polarization = 'l';
			break;

		case DVBFE_POLARIZATION_R:
			polarization = 'r';
			break;
		}
		fprintf(f,  "%c ", polarization);
		fprintf(f,  "%ul ", secs[i].lof);
		fprintf(f,  "%s", secs[i].command);
		fprintf(f,  "\n");
	}

	return 0;
}
