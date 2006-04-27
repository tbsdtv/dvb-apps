/**
 * dvbcfg_zapchannel (i.e. linuxtv *zap format) configuration file support.
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
#include "dvbcfg_zapchannel.h"

struct dvbcfg_setting {
        char *name;
	int value;
};

static const struct dvbcfg_setting bandwidth_list [] = {
	{ "BANDWIDTH_6_MHZ", DVBFE_DVBT_BANDWIDTH_6_MHZ },
	{ "BANDWIDTH_7_MHZ", DVBFE_DVBT_BANDWIDTH_7_MHZ },
	{ "BANDWIDTH_8_MHZ", DVBFE_DVBT_BANDWIDTH_8_MHZ },
        { NULL, -1 },
};

static const struct dvbcfg_setting guard_interval_list [] = {
	{"GUARD_INTERVAL_1_16", DVBFE_DVBT_GUARD_INTERVAL_1_16},
	{"GUARD_INTERVAL_1_32", DVBFE_DVBT_GUARD_INTERVAL_1_32},
	{"GUARD_INTERVAL_1_4", DVBFE_DVBT_GUARD_INTERVAL_1_4},
	{"GUARD_INTERVAL_1_8", DVBFE_DVBT_GUARD_INTERVAL_1_8},
        { NULL, -1 },
};

static const struct dvbcfg_setting hierarchy_list [] = {
	{ "HIERARCHY_1", DVBFE_DVBT_HIERARCHY_1 },
	{ "HIERARCHY_2", DVBFE_DVBT_HIERARCHY_2 },
	{ "HIERARCHY_4", DVBFE_DVBT_HIERARCHY_4 },
	{ "HIERARCHY_NONE", DVBFE_DVBT_HIERARCHY_NONE },
        { NULL, -1 },
};

static const struct dvbcfg_setting constellation_list [] = {
	{ "QPSK", DVBFE_DVBT_CONST_QPSK },
	{ "QAM_128", DVBFE_DVBT_CONST_QAM_128 },
	{ "QAM_16", DVBFE_DVBT_CONST_QAM_16 },
	{ "QAM_256", DVBFE_DVBT_CONST_QAM_256 },
	{ "QAM_32", DVBFE_DVBT_CONST_QAM_32 },
	{ "QAM_64", DVBFE_DVBT_CONST_QAM_64 },
        { NULL, -1 },
};

static const struct dvbcfg_setting transmission_mode_list [] = {
	{ "TRANSMISSION_MODE_2K", DVBFE_DVBT_TRANSMISSION_MODE_2K },
	{ "TRANSMISSION_MODE_8K", DVBFE_DVBT_TRANSMISSION_MODE_8K },
        { NULL, -1 },
};

static const struct dvbcfg_setting inversion_list[] = {
	{ "INVERSION_OFF", DVBFE_INVERSION_OFF },
	{ "INVERSION_ON", DVBFE_INVERSION_ON },
	{ "INVERSION_AUTO", DVBFE_INVERSION_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting fec_list[] = {
	{ "FEC_1_2", DVBFE_FEC_1_2 },
	{ "FEC_2_3", DVBFE_FEC_2_3 },
	{ "FEC_3_4", DVBFE_FEC_3_4 },
	{ "FEC_4_5", DVBFE_FEC_4_5 },
	{ "FEC_5_6", DVBFE_FEC_5_6 },
	{ "FEC_6_7", DVBFE_FEC_6_7 },
	{ "FEC_7_8", DVBFE_FEC_7_8 },
	{ "FEC_8_9", DVBFE_FEC_8_9 },
	{ "FEC_AUTO", DVBFE_FEC_AUTO },
	{ "FEC_NONE", DVBFE_FEC_NONE },
        { NULL, -1 },
};

static const struct dvbcfg_setting qam_modulation_list[] = {
	{ "QAM_16", DVBFE_DVBC_MOD_QAM_16 },
	{ "QAM_32", DVBFE_DVBC_MOD_QAM_32 },
	{ "QAM_64", DVBFE_DVBC_MOD_QAM_64 },
	{ "QAM_128", DVBFE_DVBC_MOD_QAM_128 },
	{ "QAM_256", DVBFE_DVBC_MOD_QAM_256 },
	{ "QAM_AUTO", DVBFE_DVBC_MOD_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting atsc_modulation_list[] = {
	{ "8VSB", DVBFE_ATSC_MOD_VSB_8 },
	{ "16VSB", DVBFE_ATSC_MOD_VSB_16 },
	{ "QAM_64", DVBFE_ATSC_MOD_QAM_64 },
	{ "QAM_256", DVBFE_ATSC_MOD_QAM_256 },
        { NULL, -1 },
};

static int parsesetting(char* text, const struct dvbcfg_setting* settings);
static char* lookupsetting(int setting, const struct dvbcfg_setting* settings);
static void curtoken(char *dest, int len, char *src, int delimiter);
static char *nexttoken(char *src, int delimiter);


int dvbcfg_zapchannel_load(FILE *f,
			   void *private,
			   dvbcfg_zapchannel_callback cb)
{
	struct dvbcfg_zapchannel tmpzapchannel;
	char *linebuf = NULL;
	size_t line_size = 0;
	int len;
	int val;

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
		memset(&tmpzapchannel, 0, sizeof(tmpzapchannel));

		/* determine frontend type */
		if (strstr(line, ":FEC_")) {
			if (strstr(line, ":HIERARCHY_")) {
				tmpzapchannel.fe_type = DVBFE_TYPE_DVBT;
			} else {
				tmpzapchannel.fe_type = DVBFE_TYPE_DVBC;
			}
		} else {
			if (strstr(line, "VSB:") || strstr(line, ":QAM_")) {
				tmpzapchannel.fe_type = DVBFE_TYPE_ATSC;
			} else {
				tmpzapchannel.fe_type = DVBFE_TYPE_DVBS;
			}
		}

		/* get the name */
		curtoken(tmpzapchannel.name, sizeof(tmpzapchannel.name), line, ':');
		if ((line = nexttoken(line, ':')) == NULL)
			continue;

		/* the frequency */
		if (sscanf(line, "%i", &tmpzapchannel.fe_params.frequency) != 1)
			continue;
		if ((line = nexttoken(line, ':')) == NULL)
			continue;

		/* the frontend settings */
		switch(tmpzapchannel.fe_type) {
		case DVBFE_TYPE_DVBT:
			/* inversion */
			if ((val = parsesetting(line, inversion_list)) < 0)
				continue;
			tmpzapchannel.fe_params.inversion = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* bandwidth */
			if ((val = parsesetting(line, bandwidth_list)) < 0)
				continue;
			tmpzapchannel.fe_params.u.dvbt.bandwidth = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* FEC HP */
			if ((val = parsesetting(line, fec_list)) < 0)
				continue;
			tmpzapchannel.fe_params.u.dvbt.code_rate_HP = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* FEC LP */
			if ((val = parsesetting(line, fec_list)) < 0)
				continue;
			tmpzapchannel.fe_params.u.dvbt.code_rate_LP = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* Constellation */
			if ((val = parsesetting(line, constellation_list)) < 0)
				continue;
			tmpzapchannel.fe_params.u.dvbt.constellation = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* Transmit mode */
			if ((val = parsesetting(line, transmission_mode_list)) < 0)
				continue;
			tmpzapchannel.fe_params.u.dvbt.transmission_mode = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* Guard interval */
			if ((val = parsesetting(line, guard_interval_list)) < 0)
				continue;
			tmpzapchannel.fe_params.u.dvbt.guard_interval = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* hierarchy */
			if ((val = parsesetting(line, hierarchy_list)) < 0)
				continue;
			tmpzapchannel.fe_params.u.dvbt.hierarchy_information = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;
			break;

		case DVBFE_TYPE_DVBC:
			/* inversion */
			if ((val = parsesetting(line, inversion_list)) < 0)
				continue;
			tmpzapchannel.fe_params.inversion = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* symrate */
			if (sscanf(line, "%i", &val) != 1)
				continue;
			tmpzapchannel.fe_params.u.dvbc.symbol_rate = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* FEC */
			if ((val = parsesetting(line, fec_list)) < 0)
				continue;
			tmpzapchannel.fe_params.u.dvbc.fec_inner = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* modulation */
			if ((val = parsesetting(line, qam_modulation_list)) < 0)
				continue;
			tmpzapchannel.fe_params.u.dvbc.modulation = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;
			break;

		case DVBFE_TYPE_DVBS:
			/* adjust frequency */
			tmpzapchannel.fe_params.frequency *= 1000;

			/* inversion */
			tmpzapchannel.fe_params.inversion = DVBFE_INVERSION_AUTO;

			/* fec */
			tmpzapchannel.fe_params.u.dvbs.fec_inner = DVBFE_FEC_AUTO;

			/* polarization */
			if (toupper(line[0]) == 'H')
				tmpzapchannel.fe_params.u.dvbs.polarization = DVBFE_POLARIZATION_H;
			else if (toupper(line[0]) == 'V')
				tmpzapchannel.fe_params.u.dvbs.polarization = DVBFE_POLARIZATION_V;
			if (toupper(line[0]) == 'L')
				tmpzapchannel.fe_params.u.dvbs.polarization = DVBFE_POLARIZATION_L;
			else if (toupper(line[0]) == 'R')
				tmpzapchannel.fe_params.u.dvbs.polarization = DVBFE_POLARIZATION_R;
			else
				continue;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* satellite switch position */
			if (sscanf(line, "%i", &val) != 1)
				continue;
			tmpzapchannel.satellite_switch = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			/* symrate */
			if (sscanf(line, "%i", &val) != 1)
				continue;
			tmpzapchannel.fe_params.u.dvbs.symbol_rate = val;
			tmpzapchannel.fe_params.u.dvbs.symbol_rate *= 1000;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;
			break;

		case DVBFE_TYPE_ATSC:
			/* inversion */
			tmpzapchannel.fe_params.inversion = DVBFE_INVERSION_AUTO;

			/* Modulation */
			if ((val = parsesetting(line, atsc_modulation_list)) < 0)
				continue;
			tmpzapchannel.fe_params.u.atsc.modulation = val;
			if ((line = nexttoken(line, ':')) == NULL)
				continue;

			break;
		}

		/* finally, read in the PIDs */
		if (sscanf(line, "%i", &tmpzapchannel.video_pid) != 1)
			continue;
		if ((line = nexttoken(line, ':')) == NULL)
			continue;
		if (sscanf(line, "%i", &tmpzapchannel.audio_pid) != 1)
			continue;

		/* this one may not be present in all config files */
		if ((line = nexttoken(line, ':')) != NULL) {
			if (sscanf(line, "%i", &tmpzapchannel.channel_number) != 1)
				continue;
		}

		// tell caller
		if (cb(private, &tmpzapchannel))
			break;
	}

	if (linebuf)
		free(linebuf);
	return 0;
}

static int dvbcfg_zapchannel_find_callback(void *private, struct dvbcfg_zapchannel *channel);

struct findparams {
	char *channel_name;
	struct dvbcfg_zapchannel *channel_dest;
};

int dvbcfg_zapchannel_find(const char *config_file,
			   char *channel_name,
			   struct dvbcfg_zapchannel *channel)
{
	struct findparams findp;

	// open the file
	FILE *f = fopen(config_file, "r");
	if (f == NULL)
		return -EIO;

	// parse each entry
	memset(channel, 0, sizeof(struct dvbcfg_zapchannel));
	findp.channel_name = channel_name;
	findp.channel_dest = channel;
	dvbcfg_zapchannel_load(f, &findp, dvbcfg_zapchannel_find_callback);

	// done
	fclose(f);
	if (channel->name[0])
		return 0;
	return -1;
}

static int dvbcfg_zapchannel_find_callback(void *private, struct dvbcfg_zapchannel *channel)
{
	struct findparams *findp = private;

	if (strcmp(findp->channel_name, channel->name))
		return 0;

	memcpy(findp->channel_dest, channel, sizeof(struct dvbcfg_zapchannel));
	return 1;
}

int dvbcfg_zapchannel_save(FILE *f,
			   struct dvbcfg_zapchannel *channels,
			   int count)
{
	int i;
	char polarization = ' ';

	for(i=0; i<count; i++) {
		fprintf(f,  "%s:", channels[i].name);

		switch(channels[i].fe_type) {
		case DVBFE_TYPE_DVBT:
			fprintf(f,  "%i:%s:%s:%s:%s:%s:%s:%s:%s:",
				channels[i].fe_params.frequency,
				lookupsetting(channels[i].fe_params.inversion, inversion_list),
				lookupsetting(channels[i].fe_params.u.dvbt.bandwidth, bandwidth_list),
				lookupsetting(channels[i].fe_params.u.dvbt.code_rate_HP, fec_list),
				lookupsetting(channels[i].fe_params.u.dvbt.code_rate_LP, fec_list),
				lookupsetting(channels[i].fe_params.u.dvbt.constellation, constellation_list),
				lookupsetting(channels[i].fe_params.u.dvbt.transmission_mode, transmission_mode_list),
				lookupsetting(channels[i].fe_params.u.dvbt.guard_interval, guard_interval_list),
				lookupsetting(channels[i].fe_params.u.dvbt.hierarchy_information, hierarchy_list));
			break;

		case DVBFE_TYPE_DVBC:
			fprintf(f,  "%i:%s:%i:%s:%s:",
				channels[i].fe_params.frequency,
				lookupsetting(channels[i].fe_params.inversion, inversion_list),
				channels[i].fe_params.u.dvbc.symbol_rate,
				lookupsetting(channels[i].fe_params.u.dvbc.fec_inner, fec_list),
				lookupsetting(channels[i].fe_params.u.dvbc.modulation, qam_modulation_list));
			break;

		case DVBFE_TYPE_DVBS:
			switch(channels[i].fe_params.u.dvbs.polarization) {
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
			fprintf(f,  "%i:%c:%i:%i:",
				channels[i].fe_params.frequency / 1000,
				polarization,
				channels[i].satellite_switch,
				channels[i].fe_params.u.dvbs.symbol_rate / 1000);
			break;

		case DVBFE_TYPE_ATSC:
			fprintf(f,  "%i:%s:",
				channels[i].fe_params.frequency,
				lookupsetting(channels[i].fe_params.u.atsc.modulation, atsc_modulation_list));
			break;
		}
		fprintf(f,  "%i:%i", channels[i].video_pid, channels[i].audio_pid);
		if (channels[i].channel_number)
			fprintf(f,  ":%i", channels[i].channel_number);
		fprintf(f,  "\n");
	}

	return 0;
}

static int parsesetting(char* text, const struct dvbcfg_setting* settings)
{
	char tmp[128];

	curtoken(tmp, sizeof(tmp), text, ':');

	while(settings->name) {
		if (!strcmp(tmp, settings->name))
			return settings->value;

		settings++;
	}

	return -1;
}

static char* lookupsetting(int setting, const struct dvbcfg_setting* settings)
{
	while(settings->name) {
		if (setting == settings->value)
			return settings->name;

		settings++;
	}

	return NULL;
}

static void curtoken(char *dest, int len, char *src, int delimiter)
{
	while((len > 1) && (*src) && (*src != delimiter)) {
		*dest++ = *src++;
		len--;
	}
	*dest = 0;
}

static char *nexttoken(char *src, int delimiter)
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
