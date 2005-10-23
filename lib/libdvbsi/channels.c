/*
	channels.conf parser implementation for libdvb

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

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "channels.h"

typedef struct {
	char *name;
	int value;
} param;

// DVB-C

static const param inversion_list[] = {
	{ "INVERSION_OFF", INVERSION_OFF },
	{ "INVERSION_ON", INVERSION_ON },
	{ "INVERSION_AUTO", INVERSION_AUTO }
};

static const param fec_list[] = {
	{ "FEC_1_2", FEC_1_2 },
	{ "FEC_2_3", FEC_2_3 },
	{ "FEC_3_4", FEC_3_4 },
	{ "FEC_4_5", FEC_4_5 },
	{ "FEC_5_6", FEC_5_6 },
	{ "FEC_6_7", FEC_6_7 },
	{ "FEC_7_8", FEC_7_8 },
	{ "FEC_8_9", FEC_8_9 },
	{ "FEC_AUTO", FEC_AUTO },
	{ "FEC_NONE", FEC_NONE }
};

static const param modulation_list[] = {
	{ "QAM_16", QAM_16 },
	{ "QAM_32", QAM_32 },
	{ "QAM_64", QAM_64 },
	{ "QAM_128", QAM_128 },
	{ "QAM_256", QAM_256 },
	{ "QAM_AUTO", QAM_AUTO }
};

// DVB-T

static const param bandwidth_list [] = {
	{ "BANDWIDTH_6_MHZ", BANDWIDTH_6_MHZ },
	{ "BANDWIDTH_7_MHZ", BANDWIDTH_7_MHZ },
	{ "BANDWIDTH_8_MHZ", BANDWIDTH_8_MHZ }
};

static const param guard_interval_list [] = {
	{"GUARD_INTERVAL_1_16", GUARD_INTERVAL_1_16},
	{"GUARD_INTERVAL_1_32", GUARD_INTERVAL_1_32},
	{"GUARD_INTERVAL_1_4", GUARD_INTERVAL_1_4},
	{"GUARD_INTERVAL_1_8", GUARD_INTERVAL_1_8}
};

static const param hierarchy_list [] = {
	{ "HIERARCHY_1", HIERARCHY_1 },
	{ "HIERARCHY_2", HIERARCHY_2 },
	{ "HIERARCHY_4", HIERARCHY_4 },
	{ "HIERARCHY_NONE", HIERARCHY_NONE }
};

static const param constellation_list [] = {
	{ "QPSK", QPSK },
	{ "QAM_128", QAM_128 },
	{ "QAM_16", QAM_16 },
	{ "QAM_256", QAM_256 },
	{ "QAM_32", QAM_32 },
	{ "QAM_64", QAM_64 }
};

static const param transmission_mode_list [] = {
	{ "TRANSMISSION_MODE_2K", TRANSMISSION_MODE_2K },
	{ "TRANSMISSION_MODE_8K", TRANSMISSION_MODE_8K },
};


#define LIST_SIZE(x) sizeof(x)/sizeof(param)


static int parse_param(char *val, const param *p_list, int list_size)
{
	int i;
	for (i = 0; i < list_size; i++) {
		if (strcasecmp(p_list[i].name, val) == 0)
			return p_list[i].value;
	}
	return -1;
}

static int parse_ter_channel_list(struct channel_params *p_channel_params, char *channel_list_file, char *channel_name, FILE *channel_list_fd)
{
	char buffer[200];
	while ((fgets(buffer, sizeof (buffer), channel_list_fd)) != NULL) {
		strcpy(p_channel_params->channel, (char *) strtok(buffer, ":"));
		p_channel_params->frequency = strtoul(strtok('\0', ":"), NULL, 0 );
		p_channel_params->inversion = parse_param(strtok('\0', ":"), inversion_list, LIST_SIZE(inversion_list));
		if (p_channel_params->inversion < 0) {
			printf("ERROR: inversion field syntax error\n");
			return -1;
		}
		p_channel_params->bandwidth = parse_param(strtok('\0', ":"), bandwidth_list, LIST_SIZE(bandwidth_list));
		if (p_channel_params->bandwidth < 0) {
			printf("ERROR: bandwidth field syntax error\n");
			return -1;
		}
		p_channel_params->code_rate_hp = parse_param(strtok('\0', ":"), fec_list, LIST_SIZE(fec_list));
		if (p_channel_params->code_rate_hp < 0) {
			printf("ERROR: fec_inner field syntax error\n");
			return -1;
		}
		p_channel_params->code_rate_lp = parse_param(strtok('\0', ":"), fec_list, LIST_SIZE(fec_list));
		if (p_channel_params->code_rate_lp < 0) {
			printf("ERROR: fec_outer field syntax error\n");
			return -1;
		}
		p_channel_params->constellation = parse_param(strtok('\0', ":"), constellation_list, LIST_SIZE(constellation_list));
		if (p_channel_params->constellation < 0) {
			printf("ERROR: modulation field syntax error\n");
			return -1;
		}
		p_channel_params->transmission_mode = parse_param(strtok('\0', ":"), transmission_mode_list, LIST_SIZE(transmission_mode_list));
		if (p_channel_params->transmission_mode < 0) {
			printf("ERROR: transmission_mode field syntax error\n");
			return -1;
		}
		p_channel_params->guard_interval = parse_param(strtok('\0', ":"), guard_interval_list, LIST_SIZE(guard_interval_list));
		if (p_channel_params->guard_interval < 0) {
			printf("ERROR: guard_interval field syntax error\n");
			return -1;
		}
		p_channel_params->hierarchy = parse_param(strtok('\0', ":"), hierarchy_list, LIST_SIZE(hierarchy_list));
		if (p_channel_params->hierarchy < 0) {
			printf("ERROR: hierarchy field syntax error\n");
			return -1;
		}
		p_channel_params->video_pid = strtoul(strtok('\0', ":"), NULL, 0 );
		p_channel_params->audio_pid = strtoul(strtok('\0', ":"), NULL, 0 );
		p_channel_params->service_id = strtoul(strtok('\0', ":"), NULL, 0 );
		if (!strcmp(channel_name, p_channel_params->channel)) {
			printf("%s: Channel=[%s], Frequency=[%d], Video=[%d], Audio=[%d], Service=[%d]\n",
				__FUNCTION__, p_channel_params->channel, p_channel_params->frequency, p_channel_params->video_pid, p_channel_params->audio_pid, p_channel_params->service_id);
			break;
		}
	}

	return 0;
}


static int parse_cab_channel_list(struct channel_params *p_channel_params, char *channel_list_file, char *channel_name, FILE *channel_list_fd)
{
	char buffer[200];
	while ((fgets(buffer, sizeof (buffer), channel_list_fd)) != NULL) {
		strcpy(p_channel_params->channel, strtok(buffer, ":"));
		p_channel_params->frequency = strtoul(strtok('\0', ":"), NULL, 0);

		p_channel_params->inversion = parse_param(strtok('\0', ":"), inversion_list, LIST_SIZE(inversion_list));
		if (p_channel_params->inversion < 0) {
			printf("ERROR: inversion field syntax error\n");
			return -1;
		}
		p_channel_params->symbol_rate = strtoul((strtok('\0', ":")), NULL, 0);
		p_channel_params->fec = parse_param(strtok('\0', ":"), fec_list, LIST_SIZE(fec_list));
		if (p_channel_params->fec < 0) {
			printf("ERROR: FEC field syntax error\n");
			return -1;
		}
		p_channel_params->modulation = parse_param(strtok('\0', ":"), modulation_list, LIST_SIZE(modulation_list));
		if (p_channel_params->modulation < 0) {
			printf("ERROR: modulation field syntax error\n");
			return -1;
		}
		p_channel_params->video_pid = strtoul((strtok('\0', ":")), NULL, 0);
		p_channel_params->audio_pid = strtoul((strtok('\0', ":")), NULL, 0);
		p_channel_params->service_id = strtoul((strtok('\0', ":")), NULL, 0); // The old format does not have it !
	}

	return 0;
}


static int parse_sat_channel_list(struct channel_params *p_channel_params, char *channel_list_file, char *channel_name, FILE *channel_list_fd)
{
	int i, entries = 0;
	char buffer[200];
	uint32_t service_id = 0;

	i = 0, entries = 0;

	while ((fgets(buffer, sizeof (buffer), channel_list_fd)) != NULL) {
		/*      Tokenize each   */
		strcpy(p_channel_params->channel, strtok(buffer, ":"));
		p_channel_params->frequency = strtoul((strtok ('\0', ":")), NULL, 0);
		strcpy(&p_channel_params->polarity, strtok('\0', ":"));
		p_channel_params->sat_no = strtoul((strtok('\0', ":")), NULL, 0);
		p_channel_params->symbol_rate = strtoul((strtok ('\0', ":")), NULL, 0);
		p_channel_params->video_pid = strtoul((strtok ('\0', ":")), NULL, 0);
		p_channel_params->audio_pid = strtoul((strtok ('\0', ":")), NULL, 0);
		p_channel_params->service_id = strtoul((strtok ('\0', ":")), NULL, 0);

		if (!strcmp(channel_name, p_channel_params->channel)) {
			service_id = p_channel_params->service_id;
			printf("%s: Channel=[%s], Frequency=[%d], Satellite=[%d], Symbol Rate=[%d], Video=[%d], Audio=[%d], Service=[%d]\n",
				__FUNCTION__, p_channel_params->channel, p_channel_params->frequency, p_channel_params->sat_no,
				p_channel_params->symbol_rate, p_channel_params->video_pid, p_channel_params->audio_pid, p_channel_params->service_id);
			break;
		}
	}

	return 0;
}

uint16_t parse_channel_list(struct channel_params *p_channel_params, char *channel_list_file, char *channel_name, uint8_t fe_type)
{
	FILE *channel_list_fd;

	if ((channel_list_fd = fopen(channel_list_file, "r")) == NULL) {
		printf ("File %s open failed.\n", channel_list_file);
		exit (-1);
	}
	printf("Parsing %s\n", channel_list_file);

	if (fe_type == 1) {
		printf("Satellite frontend\n");
		parse_sat_channel_list(p_channel_params, channel_list_file, channel_name, channel_list_fd);
		printf("Service ID=[%d]\n", p_channel_params->service_id);
	}
	else if (fe_type == 2) {
		printf("Cable frontend\n");
		parse_cab_channel_list(p_channel_params, channel_list_file, channel_name, channel_list_fd);
		printf("Service ID=[%d]\n", p_channel_params->service_id);
	}
	else if (fe_type == 3) {
		printf("Terrestrial frontend\n");
		printf("Channel %s\n", channel_name);
		parse_ter_channel_list(p_channel_params, channel_list_file, channel_name, channel_list_fd);
		printf("Service ID=[%d]\n", p_channel_params->service_id);
	}
	fclose(channel_list_fd);

	return 0;
}
