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

#include <stdio.h>
#include <errno.h>
#include "dvbcfg_zapchannel.h"
#include "dvbcfg_util.h"
#include "dvbcfg_common.h"


typedef struct {
        char *name;
        int value;
} param;

static const param bandwidth_list [] = {
        { "BANDWIDTH_6_MHZ", BANDWIDTH_6_MHZ },
        { "BANDWIDTH_7_MHZ", BANDWIDTH_7_MHZ },
        { "BANDWIDTH_8_MHZ", BANDWIDTH_8_MHZ },
        { NULL, -1 },
};

static const param guard_interval_list [] = {
        {"GUARD_INTERVAL_1_16", GUARD_INTERVAL_1_16},
        {"GUARD_INTERVAL_1_32", GUARD_INTERVAL_1_32},
        {"GUARD_INTERVAL_1_4", GUARD_INTERVAL_1_4},
        {"GUARD_INTERVAL_1_8", GUARD_INTERVAL_1_8},
        { NULL, -1 },
};

static const param hierarchy_list [] = {
        { "HIERARCHY_1", HIERARCHY_1 },
        { "HIERARCHY_2", HIERARCHY_2 },
        { "HIERARCHY_4", HIERARCHY_4 },
        { "HIERARCHY_NONE", HIERARCHY_NONE },
        { NULL, -1 },
};

static const param constellation_list [] = {
        { "QPSK", QPSK },
        { "QAM_128", QAM_128 },
        { "QAM_16", QAM_16 },
        { "QAM_256", QAM_256 },
        { "QAM_32", QAM_32 },
        { "QAM_64", QAM_64 },
        { NULL, -1 },
};

static const param transmission_mode_list [] = {
        { "TRANSMISSION_MODE_2K", TRANSMISSION_MODE_2K },
        { "TRANSMISSION_MODE_8K", TRANSMISSION_MODE_8K },
        { NULL, -1 },
};

static const param inversion_list[] = {
        { "INVERSION_OFF", INVERSION_OFF },
        { "INVERSION_ON", INVERSION_ON },
        { "INVERSION_AUTO", INVERSION_AUTO },
        { NULL, -1 },
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
        { "FEC_NONE", FEC_NONE },
        { NULL, -1 },
};

static const param qam_modulation_list[] = {
        { "QAM_16", QAM_16 },
        { "QAM_32", QAM_32 },
        { "QAM_64", QAM_64 },
        { "QAM_128", QAM_128 },
        { "QAM_256", QAM_256 },
        { "QAM_AUTO", QAM_AUTO },
        { NULL, -1 },
};

static const param atsc_modulation_list[] = {
        { "8VSB", VSB_8 },
        { "16VSB", VSB_16 },
        { "QAM_64", QAM_64 },
        { "QAM_256", QAM_256 },
        { NULL, -1 },
};

static int parsesetting(char* text, const param* settings);
static void formatsetting(FILE* out, int setting, const param* settings);


int dvbcfg_zapchannel_load(char *config_file, struct dvbcfg_zapchannel **zapchannels)
{
        FILE *in;
        char curline[256];
        char *linepos;
        struct dvbcfg_zapchannel tmpzapchannel;
        struct dvbcfg_zapchannel *curzapchannel;
        struct dvbcfg_zapchannel *newzapchannel;
        int numtokens;
        int error = 0;
        int has_channel_number;
        int val;

        /* open the file */
        in = fopen(config_file, "r");
        if (in == NULL)
                return errno;

        /* move to the tail entry */
        curzapchannel = *zapchannels;
        if (curzapchannel)
                while (curzapchannel->next)
                        curzapchannel = curzapchannel->next;

        while (fgets(curline, sizeof(curline), in)) {
                linepos = curline;
                memset(&tmpzapchannel, 0, sizeof(struct dvbcfg_zapchannel));
                has_channel_number = 0;

                /* clean any comments/ whitespace */
                if (dvbcfg_cleanline(linepos) == 0)
                        continue;

                /* determine frontend type */
                if (strstr(linepos, ":FEC_")) {
                        if (strstr(linepos, ":HIERARCHY_")) {
                                tmpzapchannel.fe_type = FE_OFDM;
                        } else {
                                tmpzapchannel.fe_type = FE_QAM;
                        }
                } else {
                        if (strstr(linepos, "VSB:") || strstr(linepos, ":QAM_")) {
                                tmpzapchannel.fe_type = FE_ATSC;
                        } else {
                                tmpzapchannel.fe_type = FE_QPSK;
                        }
                }

                /* tokenise the line */
                numtokens = dvbcfg_tokenise(linepos, ":", -1, 0);
                if (numtokens < 3)
                        continue;

                /* the name */
                tmpzapchannel.name = linepos;
                linepos = dvbcfg_nexttoken(linepos);

                /* the frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        continue;
                tmpzapchannel.fe_params.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* the frontend settings */
                switch(tmpzapchannel.fe_type) {
                case FE_OFDM:
                        if (numtokens < 12)
                                continue;
                        if (numtokens > 12)
                                has_channel_number = 1;

                        /* inversion */
                        if ((val = parsesetting(linepos, inversion_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.inversion = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* bandwidth */
                        if ((val = parsesetting(linepos, bandwidth_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.u.ofdm.bandwidth = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* FEC HP */
                        if ((val = parsesetting(linepos, fec_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.u.ofdm.code_rate_HP = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* FEC LP */
                        if ((val = parsesetting(linepos, fec_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.u.ofdm.code_rate_LP = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* Constellation */
                        if ((val = parsesetting(linepos, constellation_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.u.ofdm.constellation = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* Transmit mode */
                        if ((val = parsesetting(linepos, transmission_mode_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.u.ofdm.transmission_mode = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* Guard interval */
                        if ((val = parsesetting(linepos, guard_interval_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.u.ofdm.guard_interval = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* hierarchy */
                        if ((val = parsesetting(linepos, hierarchy_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.u.ofdm.hierarchy_information = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        break;

                case FE_QAM:
                        if (numtokens < 8)
                                continue;
                        if (numtokens > 8)
                                has_channel_number = 1;

                        /* inversion */
                        if ((val = parsesetting(linepos, inversion_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.inversion = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* symrate */
                        if (sscanf(linepos, "%i", &val) != 1)
                                continue;
                        tmpzapchannel.fe_params.u.qam.symbol_rate = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* FEC */
                        if ((val = parsesetting(linepos, fec_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.u.qam.fec_inner = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* modulation */
                        if ((val = parsesetting(linepos, qam_modulation_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.u.qam.modulation = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        break;

                case FE_QPSK:
                        if (numtokens < 7)
                                continue;
                        if (numtokens > 7)
                                has_channel_number = 1;

                        /* adjust frequency */
                        tmpzapchannel.fe_params.frequency *= 1000;

                        /* inversion */
                        tmpzapchannel.fe_params.inversion = INVERSION_AUTO;

                        /* fec */
                        tmpzapchannel.fe_params.u.qpsk.fec_inner = FEC_AUTO;

                        /* polarization */
                        if (toupper(linepos[0]) == 'H')
                                tmpzapchannel.polarization = DVBCFG_POLARIZATION_H;
                        else if (toupper(linepos[0]) == 'V')
                                tmpzapchannel.polarization = DVBCFG_POLARIZATION_V;
                        else
                                continue;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* satellite switch position */
                        if (sscanf(linepos, "%i", &val) != 1)
                                continue;
                        tmpzapchannel.satellite_switch = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* symrate */
                        if (sscanf(linepos, "%i", &val) != 1)
                                continue;
                        tmpzapchannel.fe_params.u.qpsk.symbol_rate = val;
                        tmpzapchannel.fe_params.u.qpsk.symbol_rate *= 1000;
                        linepos = dvbcfg_nexttoken(linepos);

                        break;

                case FE_ATSC:
                        if (numtokens < 5)
                                continue;
                        if (numtokens > 5)
                                has_channel_number = 1;

                        /* inversion */
                        tmpzapchannel.fe_params.inversion = INVERSION_AUTO;

                        /* Modulation */
                        if ((val = parsesetting(linepos, atsc_modulation_list)) < 0)
                                continue;
                        tmpzapchannel.fe_params.u.vsb.modulation = val;
                        linepos = dvbcfg_nexttoken(linepos);

                        break;
                }

                /* finally, read in the PIDs */
                if (sscanf(linepos, "%i", &val) != 1)
                        continue;
                tmpzapchannel.video_pid = val;
                linepos = dvbcfg_nexttoken(linepos);
                if (sscanf(linepos, "%i", &val) != 1)
                        continue;
                tmpzapchannel.audio_pid = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* this one may not be present in all config files */
                if (has_channel_number) {
                        if (sscanf(linepos, "%i", &val) != 1)
                                continue;
                        tmpzapchannel.channel_number = val;
                }


                /* create new entry */
                newzapchannel = (struct dvbcfg_zapchannel *) malloc(sizeof(struct dvbcfg_zapchannel));
                if (newzapchannel == NULL) {
                        error = -ENOMEM;
                        break;
                }
                memcpy(newzapchannel, &tmpzapchannel, sizeof(struct dvbcfg_zapchannel));
                newzapchannel->name =
                        dvbcfg_strdupandtrim(tmpzapchannel.name);
                if (!newzapchannel->name) {
                        if (newzapchannel->name)
                                free(newzapchannel->name);
                        free(newzapchannel);
                        error = -ENOMEM;
                        break;
                }

                /* add it into the list */
                if (curzapchannel) {
                        curzapchannel->next = newzapchannel;
                        newzapchannel->prev = curzapchannel;
                }
                if (!*zapchannels)
                        *zapchannels = newzapchannel;
                curzapchannel = newzapchannel;
        }

        /* tidy up and return */
        if (error) {
                dvbcfg_zapchannel_free_all(*zapchannels);
                *zapchannels = NULL;
        }
        fclose(in);
        return error;
}

int dvbcfg_zapchannel_save(char *config_file, struct dvbcfg_zapchannel *zapchannels)
{
        FILE *out;

        /* open the file */
        out = fopen(config_file, "w");
        if (out == NULL)
                return errno;

        while (zapchannels) {
                fprintf(out, "%s:", zapchannels->name);

                switch(zapchannels->fe_type) {
                case FE_OFDM:
                        fprintf(out, "%i:", zapchannels->fe_params.frequency);
                        formatsetting(out, zapchannels->fe_params.inversion, inversion_list);
                        formatsetting(out, zapchannels->fe_params.u.ofdm.bandwidth, bandwidth_list);
                        formatsetting(out, zapchannels->fe_params.u.ofdm.code_rate_HP, fec_list);
                        formatsetting(out, zapchannels->fe_params.u.ofdm.code_rate_LP, fec_list);
                        formatsetting(out, zapchannels->fe_params.u.ofdm.constellation, constellation_list);
                        formatsetting(out, zapchannels->fe_params.u.ofdm.transmission_mode, transmission_mode_list);
                        formatsetting(out, zapchannels->fe_params.u.ofdm.guard_interval, guard_interval_list);
                        formatsetting(out, zapchannels->fe_params.u.ofdm.hierarchy_information, hierarchy_list);
                        break;

                case FE_QAM:
                        fprintf(out, "%i:", zapchannels->fe_params.frequency);
                        formatsetting(out, zapchannels->fe_params.inversion, inversion_list);
                        fprintf(out, "%i:", zapchannels->fe_params.u.qam.symbol_rate);
                        formatsetting(out, zapchannels->fe_params.u.qam.fec_inner, fec_list);
                        formatsetting(out, zapchannels->fe_params.u.qam.modulation, qam_modulation_list);
                        break;

                case FE_QPSK:
                        fprintf(out, "%i:", zapchannels->fe_params.frequency / 1000);
                        switch(zapchannels->polarization) {
                        case DVBCFG_POLARIZATION_H:
                                fprintf(out, "h:");
                                break;

                        case DVBCFG_POLARIZATION_V:
                                fprintf(out, "v:");
                                break;
                        }
                        fprintf(out, "%i:", zapchannels->satellite_switch);
                        fprintf(out, "%i:", zapchannels->fe_params.u.qpsk.symbol_rate / 1000);
                        break;

                case FE_ATSC:
                        fprintf(out, "%i:", zapchannels->fe_params.frequency);
                        formatsetting(out, zapchannels->fe_params.u.vsb.modulation, atsc_modulation_list);
                        break;
                }
                fprintf(out, "%i:%i", zapchannels->video_pid, zapchannels->audio_pid);
                if (zapchannels->channel_number)
                        fprintf(out, ":%i", zapchannels->channel_number);
                fprintf(out, "\n");

                zapchannels = zapchannels->next;
        }

        fclose(out);
        return 0;
}

struct dvbcfg_zapchannel *dvbcfg_zapchannel_find(struct dvbcfg_zapchannel *zapchannels,
                                                 char *name)
{
        while (zapchannels) {
                if (!strcmp(name, zapchannels->name))
                        return zapchannels;

                zapchannels = zapchannels->next;
        }

        return NULL;
}

void dvbcfg_zapchannel_free(struct dvbcfg_zapchannel **zapchannels,
                            struct dvbcfg_zapchannel *tofree)
{
        struct dvbcfg_zapchannel *prev;
        struct dvbcfg_zapchannel *next;

        prev = tofree->prev;
        next = tofree->next;

        /* free internal structures */
        if (tofree->name)
                free(tofree->name);
        free(tofree);

        /* adjust pointers */
        if (prev == NULL)
                *zapchannels = next;
        else
                prev->next = next;

        if (next != NULL)
                next->prev = prev;
}

void dvbcfg_zapchannel_free_all(struct dvbcfg_zapchannel *zapchannels)
{
        while (zapchannels)
                dvbcfg_zapchannel_free(&zapchannels, zapchannels);
}

static int parsesetting(char* text, const param* settings)
{
        while(settings->name) {
                if (!strcmp(text, settings->name))
                        return settings->value;

                settings++;
        }

        return -1;
}

static void formatsetting(FILE* out, int setting, const param* settings)
{
        while(settings->name) {
                if (setting == settings->value) {
                        fprintf(out, "%s:", settings->name);
                        return;
                }

                settings++;
        }
}
