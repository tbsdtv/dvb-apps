/**
 * dvbcfg_vdrchannel configuration file support.
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
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "dvbcfg_vdrchannel.h"
#include "dvbcfg_util.h"
#include "dvbcfg_common.h"


static int readaudiopids(char *line, struct dvbcfg_vdrchannel *params,
                         int audio_type);
static void freeaudiostreams(struct dvbcfg_vdrchannel *channel);
static int parse_fe_setting(char *string, char **nextptr,
                            fe_type_t fe_type);
static int set_fe_setting(struct dvbcfg_vdrchannel *channel, int code,
                          int value);


int dvbcfg_vdrchannel_load(const char *config_file,
                           struct dvbcfg_vdrchannel **channels)
{
        FILE *in;
        char curline[256];
        char *linepos;
        char *tmpptr;
        char *ac3pids;
        struct dvbcfg_vdrchannel tmpchannel;
        struct dvbcfg_vdrchannel *curchannel;
        struct dvbcfg_vdrchannel *newchannel;
        char *tmpparamsstring;
        int numtokens;
        int error = 0;

        /* open the file */
        in = fopen(config_file, "r");
        if (in == NULL)
                return -errno;

        /* move to the tail entry */
        curchannel = *channels;
        if (curchannel)
                while (curchannel->next)
                        curchannel = curchannel->next;

        while (fgets(curline, sizeof(curline), in)) {
                linepos = curline;
                memset(&tmpchannel, 0, sizeof(struct dvbcfg_vdrchannel));

                /* clean any comments/ whitespace */
                if (dvbcfg_cleanline(linepos) == 0) {
                        continue;
                }

                /* tokenise the line */
                numtokens = dvbcfg_tokenise(linepos, ":", -1, 0);
                if ((numtokens != 9) && (numtokens != 13)) {
                        continue;
                }

                /* the channel name */
                tmpchannel.channel_name = linepos;
                linepos = dvbcfg_nexttoken(linepos);
                dvbcfg_replacechar(tmpchannel.channel_name, '|', ':');
                tmpptr = tmpchannel.channel_name;
                if (strchr(tmpptr, ',')) {
                        tmpptr = strchr(tmpptr, ',');
                        tmpchannel.short_name = tmpptr + 1;
                        *tmpptr = 0;
                        tmpptr++;
                }
                if ((tmpptr = strchr(tmpptr, ';')) != NULL) {
                        tmpchannel.provider_name = tmpptr + 1;
                        *tmpptr = 0;
                }

                /* the frequency */
                if (sscanf(linepos, "%d", &tmpchannel.fe_params.frequency)
                    != 1)
                        continue;
                linepos = dvbcfg_nexttoken(linepos);

                /* the channel parameters - these must be parsed AFTER the source name */
                tmpparamsstring = linepos;
                linepos = dvbcfg_nexttoken(linepos);

                /* the source name */
                tmpchannel.source_name = linepos;
                linepos = dvbcfg_nexttoken(linepos);

                /* determine the type of frontend required for this channel and initalise
                   the structure with defaults */
                tmpchannel.polarization = DVBCFG_POLARIZATION_H;
                tmpchannel.fe_params.inversion = INVERSION_OFF;
                switch (tmpchannel.source_name[0]) {
                case 'S':
                        tmpchannel.fe_type = FE_QPSK;
                        tmpchannel.fe_params.u.qpsk.fec_inner = FEC_AUTO;
                        tmpchannel.fe_params.frequency *= 1000;
                        break;

                case 'T':
                        tmpchannel.fe_type = FE_OFDM;
                        tmpchannel.fe_params.u.ofdm.bandwidth =
                            BANDWIDTH_AUTO;
                        tmpchannel.fe_params.u.ofdm.code_rate_HP =
                            FEC_AUTO;
                        tmpchannel.fe_params.u.ofdm.code_rate_LP =
                            FEC_AUTO;
                        tmpchannel.fe_params.u.ofdm.constellation =
                            QAM_AUTO;
                        tmpchannel.fe_params.u.ofdm.transmission_mode =
                            TRANSMISSION_MODE_AUTO;
                        tmpchannel.fe_params.u.ofdm.guard_interval =
                            GUARD_INTERVAL_AUTO;
                        tmpchannel.fe_params.u.ofdm.hierarchy_information =
                            HIERARCHY_AUTO;
                        break;

                case 'C':
                        tmpchannel.fe_type = FE_QAM;
                        tmpchannel.fe_params.u.qam.fec_inner = FEC_AUTO;
                        tmpchannel.fe_params.u.qam.modulation = QAM_AUTO;
                        break;
                }

                /* we can now parse the channel parameters now we know the type of source */
                while (tmpparamsstring && *tmpparamsstring) {
                        int val;
                        int code;

                        /* skip any leading space characters */
                        if (isspace(*tmpparamsstring)) {
                                tmpparamsstring++;
                                continue;
                        }

                        /* parse the value */
                        code = toupper(*tmpparamsstring);
                        val =
                            parse_fe_setting(tmpparamsstring,
                                             &tmpparamsstring,
                                             tmpchannel.fe_type);
                        if (val == -1) {
                                break;
                        }

                        /* fill out the structure */
                        if (set_fe_setting(&tmpchannel, code, val))
                                break;
                }

                /* the symbol rate */
                switch (tmpchannel.fe_type) {
                case FE_QPSK:
                        if (sscanf(linepos, "%d", &tmpchannel.fe_params.u.qpsk.symbol_rate) != 1) {
                                continue;
                        }
                        tmpchannel.fe_params.u.qpsk.symbol_rate *= 1000;
                        break;

                case FE_QAM:
                        if (sscanf(linepos, "%d", &tmpchannel.fe_params.u.qam.symbol_rate) != 1) {
                                continue;
                        }
                        break;

                default:
                        break;
                }
                linepos = dvbcfg_nexttoken(linepos);

                /* the video PID & optional PCR PID (VPID[+PCRPID]) */
                if (sscanf(linepos, "%hu", &tmpchannel.video_pid) != 1) {
                        continue;
                }
                if ((tmpptr = strchr(linepos, '+')) != NULL) {
                        if (sscanf(tmpptr + 1, "%hu", &tmpchannel.pcr_pid)
                            != 1) {
                                continue;
                        }
                }
                linepos = dvbcfg_nexttoken(linepos);

                /* the audio PIDs */
                if ((ac3pids = strchr(linepos, ';')) != NULL) {
                        /* we will parse these in a moment */
                        *ac3pids = 0;
                        ac3pids++;
                }
                if (readaudiopids
                    (linepos, &tmpchannel, DVBCFG_VDRCHANNEL_AUDIO_MPEG)) {
                        freeaudiostreams(&tmpchannel);
                        break;
                }
                if (ac3pids) {
                        if (readaudiopids
                            (ac3pids, &tmpchannel,
                             DVBCFG_VDRCHANNEL_AUDIO_DD_AC3)) {
                                freeaudiostreams(&tmpchannel);
                                break;
                        }
                        linepos = ac3pids;
                }
                linepos = dvbcfg_nexttoken(linepos);

                /* the teletext PID */
                if (sscanf(linepos, "%hu", &tmpchannel.teletext_pid) != 1)
                        continue;
                linepos = dvbcfg_nexttoken(linepos);

                /* check for "old format" VDR files */
                if (numtokens == 9) {
                        /* old format service id */
                        if (sscanf(linepos, "%hu", &tmpchannel.service_id)
                            != 1)
                                continue;
                } else {        /* "new format" files */

                        /* the CAIDs */
                        tmpptr = linepos;
                        while (tmpptr
                               && (tmpchannel.num_caids <
                                   DVBCFG_VDRCHANNEL_MAXCAIDS)) {
                                if (sscanf(tmpptr, "%hu", &tmpchannel.caids[tmpchannel.num_caids]) != 1)
                                        break;
                                tmpchannel.num_caids++;

                                tmpptr = strchr(tmpptr, ',');
                                if (!tmpptr)
                                        break;
                                tmpptr++;
                        }
                        linepos = dvbcfg_nexttoken(linepos);

                        /* new format service id */
                        if (sscanf(linepos, "%hu", &tmpchannel.service_id) != 1)
                                continue;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* original network id */
                        if (sscanf(linepos, "%hu", &tmpchannel.original_network_id) != 1)
                                continue;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* transport stream id */
                        if (sscanf(linepos, "%hu", &tmpchannel.transport_stream_id) != 1)
                                continue;
                        linepos = dvbcfg_nexttoken(linepos);

                        /* radio id */
                        if (sscanf(linepos, "%hu", &tmpchannel.radio_id) != 1)
                                continue;
                        linepos = dvbcfg_nexttoken(linepos);
                }

                /* create new entry */
                newchannel = (struct dvbcfg_vdrchannel *)
                    malloc(sizeof(struct dvbcfg_vdrchannel));
                if (newchannel == NULL) {
                        if (tmpchannel.audio_streams)
                                freeaudiostreams(&tmpchannel);
                        error = 1;
                        break;
                }
                memcpy(newchannel, &tmpchannel,
                       sizeof(struct dvbcfg_vdrchannel));
                newchannel->channel_name =
                    dvbcfg_strdupandtrim(tmpchannel.channel_name, -1);
                if (newchannel->provider_name)
                        newchannel->provider_name =
                            dvbcfg_strdupandtrim(tmpchannel.provider_name, -1);
                if (newchannel->short_name)
                        newchannel->short_name =
                            dvbcfg_strdupandtrim(tmpchannel.short_name, -1);
                newchannel->source_name =
                    dvbcfg_strdupandtrim(tmpchannel.source_name, -1);
                if ((!newchannel->channel_name)
                    || (!newchannel->source_name)) {
                        if (newchannel->channel_name)
                                free(newchannel->channel_name);
                        if (newchannel->provider_name)
                                free(newchannel->provider_name);
                        if (newchannel->short_name)
                                free(newchannel->short_name);
                        if (newchannel->source_name)
                                free(newchannel->source_name);
                        if (newchannel->audio_streams)
                                freeaudiostreams(newchannel);
                        free(newchannel);
                        error = -ENOMEM;
                        break;
                }

                /* add it into the list */
                if (curchannel) {
                        curchannel->next = newchannel;
                        newchannel->prev = curchannel;
                }
                if (!*channels)
                        *channels = newchannel;
                curchannel = newchannel;
        }

        /* tidy up and return */
        if (error) {
                dvbcfg_vdrchannel_free_all(*channels);
                *channels = NULL;
        }
        fclose(in);
        return 0;
}

void dvbcfg_vdrchannel_free(struct dvbcfg_vdrchannel **channels,
                            struct dvbcfg_vdrchannel *tofree)
{
        struct dvbcfg_vdrchannel *prev;
        struct dvbcfg_vdrchannel *next;

        prev = tofree->prev;
        next = tofree->next;

        /* free internal structures */
        if (tofree->channel_name)
                free(tofree->channel_name);
        if (tofree->provider_name)
                free(tofree->provider_name);
        if (tofree->short_name)
                free(tofree->short_name);
        if (tofree->source_name)
                free(tofree->source_name);
        if (tofree->audio_streams)
                freeaudiostreams(tofree);
        free(tofree);

        /* adjust pointers */
        if (prev == NULL)
                *channels = next;
        else
                prev->next = next;
        if (next != NULL)
                next->prev = prev;
}

void dvbcfg_vdrchannel_free_all(struct dvbcfg_vdrchannel *channels)
{
        while (channels)
                dvbcfg_vdrchannel_free(&channels, channels);
}







static void freeaudiostreams(struct dvbcfg_vdrchannel *channel)
{
        struct dvbcfg_vdrchannel_audio *curaudiostream =
            channel->audio_streams;
        struct dvbcfg_vdrchannel_audio *nextaudiostream;
        while (curaudiostream) {
                nextaudiostream = curaudiostream->next;
                free(curaudiostream);

                curaudiostream = nextaudiostream;
        }
}

static int readaudiopids(char *line, struct dvbcfg_vdrchannel *channel,
                         int audio_type)
{
        struct dvbcfg_vdrchannel_audio *stream_tail;

        /* find the tail of the streams */
        stream_tail = channel->audio_streams;
        while (stream_tail && stream_tail->next) {
                stream_tail = stream_tail->next;
        }

        /* the normal audio PIDs */
        while (line) {
                int pid;
                char lang[4];
                char *next;
                char *equals;
                struct dvbcfg_vdrchannel_audio *curstream;

                /* if there is another one after this, terminate the string */
                if ((next = strchr(line, ',')) != NULL) {
                        *next = 0;
                }

                /* get the PID */
                if (sscanf(line, "%d", &pid) != 1)
                        break;

                /* if there is an "=LNG" code, grab that */
                memset(lang, 0, sizeof(lang));
                if ((equals = strchr(line, '=')) != NULL) {
                        strncpy(lang, equals + 1, 3);
                }

                /* allocate new structure */
                curstream = (struct dvbcfg_vdrchannel_audio *)
                    malloc(sizeof(struct dvbcfg_vdrchannel_audio));
                if (!curstream) {
                        return -ENOMEM;
                }
                memset(curstream, 0,
                       sizeof(struct dvbcfg_vdrchannel_audio));
                curstream->pid = pid;
                memcpy(curstream->lang, lang, sizeof(curstream->lang));
                curstream->type = audio_type;

                /* add it into the chain. */
                if (stream_tail)
                        stream_tail->next = curstream;
                stream_tail = curstream;
                if (!channel->audio_streams)
                        channel->audio_streams = curstream;

                /* restore the string to what it was */
                if (next) {
                        *next = ',';
                        next++;
                }
                line = next;
        }

        return 0;
}

#define PARSEINT(__RESULT, __STRING, __NEXTPTR) \
    errno = 0; \
    val = strtol(__STRING, __NEXTPTR, 10); \
    if (errno) return -1;

static int parse_fe_setting(char *string, char **nextptr,
                            fe_type_t fe_type)
{
        char *allowed;
        int val;

        /* is this param allowed for this frontend type? */
        switch (fe_type) {
        case FE_QPSK:
                allowed = "CIHVRL";
                break;

        case FE_QAM:
                allowed = "CIM";
                break;

        case FE_OFDM:
                allowed = "BCDGIMTY";
                break;

        default:
                return -1;
        }
        if (!strchr(allowed, toupper(*string)))
                return -1;

        /* parse it! */
        *nextptr = NULL;
        switch (toupper(*string)) {
        case 'B':
                PARSEINT(val, string + 1, nextptr);

                switch (val) {
                case 6:
                        return BANDWIDTH_6_MHZ;
                case 7:
                        return BANDWIDTH_7_MHZ;
                case 8:
                        return BANDWIDTH_8_MHZ;
                case 999:
                        return BANDWIDTH_AUTO;
                default:
                        return -1;
                }
                break;

        case 'C':
        case 'D':
                PARSEINT(val, string + 1, nextptr);

                switch (val) {
                case 0:
                        return FEC_NONE;
                case 12:
                        return FEC_1_2;
                case 23:
                        return FEC_2_3;
                case 34:
                        return FEC_3_4;
                case 45:
                        return FEC_4_5;
                case 56:
                        return FEC_5_6;
                case 67:
                        return FEC_6_7;
                case 78:
                        return FEC_7_8;
                case 89:
                        return FEC_8_9;
                case 999:
                        return FEC_AUTO;
                default:
                        return -1;
                }
                break;

        case 'G':
                PARSEINT(val, string + 1, nextptr);

                switch (val) {
                case 4:
                        return GUARD_INTERVAL_1_4;
                case 8:
                        return GUARD_INTERVAL_1_8;
                case 16:
                        return GUARD_INTERVAL_1_16;
                case 32:
                        return GUARD_INTERVAL_1_32;
                case 999:
                        return GUARD_INTERVAL_AUTO;
                default:
                        return -1;
                }
                break;

        case 'I':
                PARSEINT(val, string + 1, nextptr);
                switch (val) {
                case 0:
                        return INVERSION_OFF;
                case 1:
                        return INVERSION_ON;
                case 999:
                        return INVERSION_AUTO;
                default:
                        return -1;
                }
                break;

        case 'M':
                PARSEINT(val, string + 1, nextptr);
                switch (val) {
                case 0:
                        return QPSK;
                case 16:
                        return QAM_16;
                case 32:
                        return QAM_32;
                case 64:
                        return QAM_64;
                case 128:
                        return QAM_128;
                case 256:
                        return QAM_256;
                case 999:
                        return QAM_AUTO;
                default:
                        return -1;
                }
                break;

        case 'T':
                PARSEINT(val, string + 1, nextptr);
                switch (val) {
                case 2:
                        return TRANSMISSION_MODE_2K;
                case 8:
                        return TRANSMISSION_MODE_8K;
                case 999:
                        return TRANSMISSION_MODE_AUTO;
                default:
                        return -1;
                }
                break;

        case 'Y':
                PARSEINT(val, string + 1, nextptr);
                switch (val) {
                case 0:
                        return HIERARCHY_NONE;
                case 1:
                        return HIERARCHY_1;
                case 2:
                        return HIERARCHY_2;
                case 4:
                        return HIERARCHY_4;
                case 999:
                        return HIERARCHY_AUTO;
                default:
                        return -1;
                }
                break;

        case 'H':
                *nextptr = string + 1;
                return DVBCFG_POLARIZATION_H;

        case 'V':
                *nextptr = string + 1;
                return DVBCFG_POLARIZATION_V;

        case 'L':
                *nextptr = string + 1;
                return DVBCFG_POLARIZATION_L;

        case 'R':
                *nextptr = string + 1;
                return DVBCFG_POLARIZATION_R;

        default:
                return -1;
        }
	return -1;
}

static int set_fe_setting(struct dvbcfg_vdrchannel *channel, int code,
                          int value)
{
        if (code == 'I') {
                channel->fe_params.inversion = value;
                return 0;
        }

        switch (channel->fe_type) {
        case FE_QPSK:
                switch (code) {
                case 'C':
                        channel->fe_params.u.qpsk.fec_inner = value;
                        break;

                case 'H':
                case 'V':
                case 'R':
                case 'L':
                        channel->polarization = value;
                        break;
                }

        case FE_QAM:
                switch (code) {
                case 'C':
                        channel->fe_params.u.qam.fec_inner = value;
                        break;

                case 'M':
                        channel->fe_params.u.qam.modulation = value;
                        break;
                }

        case FE_OFDM:
                switch (code) {
                case 'B':
                        channel->fe_params.u.ofdm.bandwidth = value;
                        break;

                case 'C':
                        channel->fe_params.u.ofdm.code_rate_HP = value;
                        break;

                case 'D':
                        channel->fe_params.u.ofdm.code_rate_LP = value;
                        break;

                case 'G':
                        channel->fe_params.u.ofdm.guard_interval = value;
                        break;

                case 'M':
                        channel->fe_params.u.ofdm.constellation = value;
                        break;

                case 'T':
                        channel->fe_params.u.ofdm.transmission_mode =
                            value;
                        break;

                case 'Y':
                        channel->fe_params.u.ofdm.hierarchy_information =
                            value;
                        break;
                }

        case FE_ATSC:
                return -EINVAL;
        }

        return 0;
}
