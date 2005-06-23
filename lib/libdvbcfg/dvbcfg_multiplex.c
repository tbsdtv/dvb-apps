/**
 * dvbcfg_multiplex configuration file support.
 *
 * Copyright (c) 2005 by Andrew de Quincey <adq_dvb@lidskialf.net>
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
#include "dvbcfg_multiplex.h"
#include "dvbcfg_util.h"


#define LOCATION_SOF 0
#define LOCATION_DVBMULTIPLEXES 1
#define LOCATION_MULTIPLEX 2
#define LOCATION_SERVICE 3

static const struct dvbcfg_setting bandwidth_list [] = {
        { "BANDWIDTH_6_MHZ", BANDWIDTH_6_MHZ },
        { "BANDWIDTH_7_MHZ", BANDWIDTH_7_MHZ },
        { "BANDWIDTH_8_MHZ", BANDWIDTH_8_MHZ },
        { "BANDWIDTH_AUTO",  BANDWIDTH_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting guard_interval_list [] = {
        {"GUARD_INTERVAL_1_16", GUARD_INTERVAL_1_16},
        {"GUARD_INTERVAL_1_32", GUARD_INTERVAL_1_32},
        {"GUARD_INTERVAL_1_4",  GUARD_INTERVAL_1_4},
        {"GUARD_INTERVAL_1_8",  GUARD_INTERVAL_1_8},
        {"GUARD_INTERVAL_AUTO", GUARD_INTERVAL_AUTO},
        { NULL, -1 },
};

static const struct dvbcfg_setting hierarchy_information_list [] = {
        { "HIERARCHY_NONE", HIERARCHY_NONE },
        { "HIERARCHY_1",    HIERARCHY_1 },
        { "HIERARCHY_2",    HIERARCHY_2 },
        { "HIERARCHY_4",    HIERARCHY_4 },
        { "HIERARCHY_AUTO", HIERARCHY_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting constellation_list [] = {
        { "QPSK",     QPSK },
        { "QAM_16",   QAM_16 },
        { "QAM_32",   QAM_32 },
        { "QAM_64",   QAM_64 },
        { "QAM_128",  QAM_128 },
        { "QAM_256",  QAM_256 },
        { "QAM_AUTO", QAM_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting transmission_mode_list [] = {
        { "TRANSMISSION_MODE_2K",   TRANSMISSION_MODE_2K },
        { "TRANSMISSION_MODE_8K",   TRANSMISSION_MODE_8K },
        { "TRANSMISSION_MODE_AUTO", TRANSMISSION_MODE_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting inversion_list[] = {
        { "INVERSION_OFF",  INVERSION_OFF },
        { "INVERSION_ON",   INVERSION_ON },
        { "INVERSION_AUTO", INVERSION_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting fec_list[] = {
        { "FEC_NONE", FEC_NONE },
        { "FEC_1_2",  FEC_1_2 },
        { "FEC_2_3",  FEC_2_3 },
        { "FEC_3_4",  FEC_3_4 },
        { "FEC_4_5",  FEC_4_5 },
        { "FEC_5_6",  FEC_5_6 },
        { "FEC_6_7",  FEC_6_7 },
        { "FEC_7_8",  FEC_7_8 },
        { "FEC_8_9",  FEC_8_9 },
        { "FEC_AUTO", FEC_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting qam_modulation_list[] = {
        { "QAM_16",   QAM_16 },
        { "QAM_32",   QAM_32 },
        { "QAM_64",   QAM_64 },
        { "QAM_128",  QAM_128 },
        { "QAM_256",  QAM_256 },
        { "QAM_AUTO", QAM_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting atsc_modulation_list[] = {
        { "VSB_8",    VSB_8 },
        { "VSB_16",   VSB_16 },
        { "QAM_64",   QAM_64 },
        { "QAM_256",  QAM_256 },
        { "QAM_AUTO", QAM_AUTO },
        { NULL, -1 },
};


static struct dvbcfg_multiplex* parse_multiplex(struct dvbcfg_source** sources,
                                                struct dvbcfg_multiplex** multiplexes,
                                                int create_sources,
                                                char* tmpgmid,
                                                char* tmpdelivery);

static struct dvbcfg_service* parse_service(struct dvbcfg_multiplex* multiplex,
                                            char* tmpusid,
                                            char* tmpname,
                                            char* tmpflags,
                                            char* tmpca_systems,
                                            char* tmpzap_pids,
                                            char* tmppmt_extra);


int dvbcfg_multiplex_load(char *config_file,
                          struct dvbcfg_source** sources,
                          struct dvbcfg_multiplex** multiplexes,
                          int create_sources)
{
        FILE *in;
        char curline[256];
        char *linepos;
        struct dvbcfg_multiplex *newmultiplex;
        char* value;
        int numtokens;
        int error = 0;
        int location = LOCATION_SOF;
        char* tmpgmid = NULL;
        char* tmpdelivery = NULL;
        char* tmpusid = NULL;
        char* tmpname = NULL;
        char* tmpflags = NULL;
        char* tmpca_systems = NULL;
        char* tmpzap_pids = NULL;
        char* tmppmt_extra = NULL;


        /* open the file */
        in = fopen(config_file, "r");
        if (in == NULL)
                return errno;

        while (fgets(curline, sizeof(curline), in)) {
                linepos = curline;

                /* clean any comments/ whitespace */
                if (dvbcfg_cleanline(linepos) == 0)
                        continue;

                /* is it a [section]? */
                if (dvbcfg_issection(linepos, "dvbmultiplexes")) {
                        switch(location) {
                        case LOCATION_SOF:
                                break;

                        default:
                                error = -EINVAL;
                                goto exit;
                        }

                        location = LOCATION_DVBMULTIPLEXES;
                        continue;
                } else if (dvbcfg_issection(linepos, "multiplex")) {
                        switch(location) {
                        case LOCATION_DVBMULTIPLEXES:
                                break;

                        case LOCATION_SERVICE:
                                if (parse_service(newmultiplex, tmpusid, tmpname, tmpflags,
                                                  tmpca_systems, tmpzap_pids, tmppmt_extra) == NULL) {
                                        error = -EINVAL;
                                        goto exit;
                                }
                                break;

                        case LOCATION_MULTIPLEX:
                                newmultiplex = parse_multiplex(sources, multiplexes, create_sources, tmpgmid, tmpdelivery);
                                if (newmultiplex == NULL) {
                                        error = -EINVAL;
                                        goto exit;
                                }
                                break;

                        default:
                                error = -EINVAL;
                                goto exit;
                        }

                        /* tidy up for next one */
                        dvbcfg_freestring(&tmpgmid);
                        dvbcfg_freestring(&tmpdelivery);
                        dvbcfg_freestring(&tmpusid);
                        dvbcfg_freestring(&tmpname);
                        dvbcfg_freestring(&tmpflags);
                        dvbcfg_freestring(&tmpca_systems);
                        dvbcfg_freestring(&tmpzap_pids);
                        dvbcfg_freestring(&tmppmt_extra);
                        location = LOCATION_MULTIPLEX;
                        continue;
                } else if (dvbcfg_issection(linepos, "service")) {
                        switch(location) {
                        case LOCATION_SERVICE:
                                if (parse_service(newmultiplex, tmpusid, tmpname, tmpflags,
                                                  tmpca_systems, tmpzap_pids, tmppmt_extra) == NULL) {
                                        error = -EINVAL;
                                        goto exit;
                                }
                                break;

                        case LOCATION_MULTIPLEX:
                                newmultiplex = parse_multiplex(sources, multiplexes, create_sources, tmpgmid, tmpdelivery);
                                if (newmultiplex == NULL) {
                                        error = -EINVAL;
                                        goto exit;
                                }
                                break;

                        default:
                                error = -EINVAL;
                                goto exit;
                        }

                        /* tidy up for next one */
                        dvbcfg_freestring(&tmpgmid);
                        dvbcfg_freestring(&tmpdelivery);
                        dvbcfg_freestring(&tmpusid);
                        dvbcfg_freestring(&tmpname);
                        dvbcfg_freestring(&tmpflags);
                        dvbcfg_freestring(&tmpca_systems);
                        dvbcfg_freestring(&tmpzap_pids);
                        dvbcfg_freestring(&tmppmt_extra);
                        location = LOCATION_SERVICE;
                        continue;
                }

                /* deal with the keys depending on the file location */
                switch(location) {
                case LOCATION_DVBMULTIPLEXES:
                        if (value = dvbcfg_iskey(linepos, "version")) {
                                if (strcmp(value, "0.1")) {
                                        error = -EINVAL;
                                        goto exit;
                                }
                        } else if (value == dvbcfg_iskey(linepos, "date")) {
                                // ignore this
                        } else {
                                error = -EINVAL;
                                goto exit;
                        }
                        break;

                case LOCATION_MULTIPLEX:
                        if (value = dvbcfg_iskey(linepos, "gmid")) {
                                tmpgmid = dvbcfg_strdupandtrim(value, -1);
                        } else if (value == dvbcfg_iskey(linepos, "delivery")) {
                                tmpdelivery = dvbcfg_strdupandtrim(value, -1);
                        } else {
                                error = -EINVAL;
                                goto exit;
                        }
                        break;

                case LOCATION_SERVICE:
                        if (value = dvbcfg_iskey(linepos, "usid")) {
                                tmpusid = dvbcfg_strdupandtrim(value, -1);
                        } else if (value == dvbcfg_iskey(linepos, "name")) {
                                tmpname = dvbcfg_strdupandtrim(value, -1);
                        } else if (value == dvbcfg_iskey(linepos, "flags")) {
                                tmpflags = dvbcfg_strdupandtrim(value, -1);
                        } else if (value == dvbcfg_iskey(linepos, "ca_systems")) {
                                tmpca_systems = dvbcfg_strdupandtrim(value, -1);
                        } else if (value == dvbcfg_iskey(linepos, "zap_pids")) {
                                tmpzap_pids = dvbcfg_strdupandtrim(value, -1);
                        } else if (value == dvbcfg_iskey(linepos, "pmt_extra")) {
                                tmppmt_extra = dvbcfg_strdupandtrim(value, -1);
                        } else {
                                error = -EINVAL;
                                goto exit;
                        }
                        break;

                default:
                        error = -EINVAL;
                        goto exit;
                }
        }

exit:
        /* tidy up and return */
        dvbcfg_freestring(&tmpgmid);
        dvbcfg_freestring(&tmpdelivery);
        dvbcfg_freestring(&tmpusid);
        dvbcfg_freestring(&tmpname);
        dvbcfg_freestring(&tmpflags);
        dvbcfg_freestring(&tmpca_systems);
        dvbcfg_freestring(&tmpzap_pids);
        dvbcfg_freestring(&tmppmt_extra);
        fclose(in);
        return error;
}

static struct dvbcfg_multiplex* parse_multiplex(struct dvbcfg_source** sources,
                                                struct dvbcfg_multiplex** multiplexes,
                                                int create_sources,
                                                char* tmpgmid,
                                                char* tmpdelivery)
{
        struct dvbcfg_gmid gmid;
        struct dvbcfg_source* source;
        struct dvbcfg_multiplex* multiplex = NULL;
        int numtokens;
        char* linepos;
        int val;

        /* parse the GMID and find the associated dvbcfg_source instance */
        if (dvbcfg_gmid_from_string(tmpgmid, &gmid))
                return NULL;

        source = dvbcfg_source_find2(*sources, &gmid.source_id);
        if (source == NULL) {
                if (!create_sources) {
                        dvbcfg_source_id_free(&gmid.source_id);
                        return NULL;
                }

                source = dvbcfg_source_new2(sources, &gmid.source_id, "???");
                dvbcfg_source_id_free(&gmid.source_id);
                if (source == NULL) {
                        return NULL;
                }
        } else {
                dvbcfg_source_id_free(&gmid.source_id);
        }

        /* create the new multiplex */
        multiplex = dvbcfg_multiplex_new2(multiplexes, source, &gmid.umid);
        if (multiplex == NULL)
                return NULL;

        /* now, parse the location string */
        switch(source->source_id.source_type) {
        case DVBCFG_SOURCETYPE_DVBS:
                numtokens = dvbcfg_tokenise(tmpdelivery, " \t", -1, 1);
                if (numtokens != 5)
                        goto error;
                linepos = tmpdelivery;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        goto error;
                multiplex->delivery.dvb.fe_params.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* inversion */
                if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* polarization */
                if (toupper(linepos[0]) == 'H')
                        multiplex->delivery.dvb.polarization = DVBCFG_POLARIZATION_H;
                else if (toupper(linepos[0]) == 'V')
                        multiplex->delivery.dvb.polarization = DVBCFG_POLARIZATION_V;
                else if (toupper(linepos[0]) == 'L')
                        multiplex->delivery.dvb.polarization = DVBCFG_POLARIZATION_L;
                else if (toupper(linepos[0]) == 'R')
                        multiplex->delivery.dvb.polarization = DVBCFG_POLARIZATION_R;
                else
                        goto error;
                linepos = dvbcfg_nexttoken(linepos);

                /* symbol_rate */
                if (sscanf(linepos, "%i", &val) != 1)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.qpsk.symbol_rate = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* fec_inner */
                if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.qpsk.fec_inner = val;

                break;

        case DVBCFG_SOURCETYPE_DVBC:
                numtokens = dvbcfg_tokenise(tmpdelivery, " \t", -1, 1);
                if (numtokens != 5)
                  goto error;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        goto error;
                multiplex->delivery.dvb.fe_params.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* inversion */
                if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* symbol_rate */
                if (sscanf(linepos, "%i", &val) != 1)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.qam.symbol_rate = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* fec_inner */
                if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.qam.fec_inner = val;

                /* modulation */
                if ((val = dvbcfg_parsesetting(linepos, qam_modulation_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.qam.modulation = val;

                break;

        case DVBCFG_SOURCETYPE_DVBT:
                numtokens = dvbcfg_tokenise(tmpdelivery, " \t", -1, 1);
                if (numtokens != 9)
                        goto error;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        goto error;
                multiplex->delivery.dvb.fe_params.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* inversion */
                if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* bandwidth */
                if ((val = dvbcfg_parsesetting(linepos, bandwidth_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.bandwidth = val;

                /* code_rate_HP */
                if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.code_rate_HP = val;

                /* code_rate_LP */
                if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.code_rate_LP = val;

                /* constellation */
                if ((val = dvbcfg_parsesetting(linepos, constellation_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.constellation = val;

                /* transmission_mode */
                if ((val = dvbcfg_parsesetting(linepos, transmission_mode_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.transmission_mode = val;

                /* guard_interval */
                if ((val = dvbcfg_parsesetting(linepos, guard_interval_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.guard_interval = val;

                /* hierarchy_information */
                if ((val = dvbcfg_parsesetting(linepos, hierarchy_information_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.hierarchy_information = val;

                break;

        case DVBCFG_SOURCETYPE_ATSC:
                numtokens = dvbcfg_tokenise(tmpdelivery, " \t", -1, 1);
                if (numtokens != 3)
                        goto error;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        goto error;
                multiplex->delivery.dvb.fe_params.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* inversion */
                if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* modulation */
                if ((val = dvbcfg_parsesetting(linepos, atsc_modulation_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.vsb.modulation = val;

                break;

        default:
                goto error;
        }

        /* done! */
        return multiplex;

error:
        if (multiplex != NULL)
                dvbcfg_multiplex_free(multiplexes, multiplex);
        return NULL;
}

static struct dvbcfg_service* parse_service(struct dvbcfg_multiplex* multiplex,
                                            char* tmpusid,
                                            char* tmpname,
                                            char* tmpflags,
                                            char* tmpca_systems,
                                            char* tmpzap_pids,
                                            char* tmppmt_extra)
{
  // FIXME
}

int dvbcfg_multiplex_save(char *config_file, struct dvbcfg_multiplex *multiplexes)
{
        FILE *out;
        char* umid;
        char polarization;
        char* source_id;

        /* open the file */
        out = fopen(config_file, "w");
        if (out == NULL)
                return errno;

        fprintf(out, "[dvbmultiplexes]\n");
        fprintf(out, "version=0.1\n");
        fprintf(out, "date=%i\n", time(NULL));
        fprintf(out, "\n");

        while (multiplexes) {

                umid = dvbcfg_umid_to_string(&multiplexes->umid);
                if (umid == NULL)
                  break;

                source_id = dvbcfg_source_id_to_string(&multiplexes->source->source_id);
                if (source_id == NULL) {
                  free(umid);
                  break;
                }

                fprintf(out, "[multiplex]\n");
                fprintf(out, "gmid=%s:%s\n", source_id, umid);
                free(umid);
                free(source_id);

                /* output the delivery */
                fprintf(out, "delivery = ");
                switch(multiplexes->source->source_id.source_type) {
                case DVBCFG_SOURCETYPE_DVBS:
                        switch(multiplexes->delivery.dvb.polarization) {
                        case DVBCFG_POLARIZATION_H:
                                polarization = 'H';
                                break;

                        case DVBCFG_POLARIZATION_V:
                                polarization = 'V';
                                break;

                        case DVBCFG_POLARIZATION_L:
                                polarization = 'L';
                                break;

                        case DVBCFG_POLARIZATION_R:
                                polarization = 'R';
                                break;
                        }

                        fprintf(out, "%i %s %c %i %s\n",
                                multiplexes->delivery.dvb.fe_params.frequency,
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.inversion, inversion_list),
                                polarization,
                                multiplexes->delivery.dvb.fe_params.u.qpsk.symbol_rate,
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.u.qpsk.fec_inner, fec_list));
                        break;

                case DVBCFG_SOURCETYPE_DVBC:
                        fprintf(out, "%i %s %i %s %s\n",
                                multiplexes->delivery.dvb.fe_params.frequency,
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.inversion, inversion_list),
                                multiplexes->delivery.dvb.fe_params.u.qpsk.symbol_rate,
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.u.qam.fec_inner, fec_list),
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.u.qam.modulation, qam_modulation_list));
                        break;

                case DVBCFG_SOURCETYPE_DVBT:
                        fprintf(out, "%i %s %s %s %s %s %s %s %s\n",
                                multiplexes->delivery.dvb.fe_params.frequency,
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.inversion, inversion_list),
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.u.ofdm.bandwidth, bandwidth_list),
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.u.ofdm.code_rate_HP, fec_list),
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.u.ofdm.code_rate_LP, fec_list),
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.u.ofdm.constellation, constellation_list),
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.u.ofdm.transmission_mode,
                                                  transmission_mode_list),
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.u.ofdm.guard_interval, guard_interval_list),
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.u.ofdm.hierarchy_information,
                                                hierarchy_information_list));

                        break;

                case DVBCFG_SOURCETYPE_ATSC:
                        fprintf(out, "%i %s %s\n",
                                multiplexes->delivery.dvb.fe_params.frequency,
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.inversion, inversion_list),
                                dvbcfg_lookupstr(multiplexes->delivery.dvb.fe_params.u.vsb.modulation, atsc_modulation_list));
                        break;
                }
                fprintf(out, "\n");

                // FIXME: output services

                multiplexes = multiplexes->next;
        }

        fclose(out);
        return 0;
}

struct dvbcfg_multiplex* dvbcfg_multiplex_new(struct dvbcfg_multiplex** multiplexes,
                                              struct dvbcfg_source* source,
                                              char* umidstr)
{
        struct dvbcfg_umid umid;

        /* parse the umid */
        if (dvbcfg_umid_from_string(umidstr, &umid)) {
                return NULL;
        }

        return dvbcfg_multiplex_new2(multiplexes, source, &umid);
}

struct dvbcfg_multiplex* dvbcfg_multiplex_new2(struct dvbcfg_multiplex** multiplexes,
                                              struct dvbcfg_source* source,
                                              struct dvbcfg_umid* umid)
{
        struct dvbcfg_multiplex* newmultiplex;
        struct dvbcfg_multiplex* curmultiplex;

        /* create new structure */
        newmultiplex = (struct dvbcfg_multiplex*) malloc(sizeof(struct dvbcfg_multiplex));
        if (newmultiplex == NULL)
                return NULL;
        memset(newmultiplex, 0, sizeof(struct dvbcfg_multiplex));
        newmultiplex->source = source;
        memcpy(&newmultiplex->umid, umid, sizeof(struct dvbcfg_umid));

        /* add it to the list */
        if (*multiplexes == NULL)
                *multiplexes = newmultiplex;
        else {
                curmultiplex = *multiplexes;
                while(curmultiplex->next)
                        curmultiplex = curmultiplex->next;
                curmultiplex->next = newmultiplex;
        }

        return newmultiplex;
}

struct dvbcfg_service* dvbcfg_multiplex_add_service(struct dvbcfg_multiplex* multiplex,
                                                    char* name,
                                                    char* usid,
                                                    uint32_t service_flags)
{
  // FIXME
}

struct dvbcfg_service* dvbcfg_multiplex_add_service2(struct dvbcfg_multiplex* multiplex,
                                                     char* name,
                                                     struct dvbcfg_usid* usid,
                                                     uint32_t service_flags)
{
  // FIXME
}

int dvbcfg_multiplex_remove_service(struct dvbcfg_multiplex* multiplex,
                                    struct dvbcfg_service* service)
{
  // FIXME
}

int dvbcfg_multiplex_add_ca_system(struct dvbcfg_service* service,
                                   uint16_t ca_system_id)
{
  // FIXME
}

int dvbcfg_multiplex_remove_ca_system(struct dvbcfg_service* service,
                                      uint16_t ca_system_id)
{
  // FIXME
}

int dvbcfg_multiplex_add_zap_pid(struct dvbcfg_service* service,
                                 int pid,
                                 int type)
{
  // FIXME
}

int dvbcfg_multiplex_remove_zap_pid(struct dvbcfg_service* service,
                                    int pid,
                                    int type)
{
  // FIXME
}

int dvbcfg_multiplex_add_pmt_pid(struct dvbcfg_service* service,
                                 int pid,
                                 int type)
{
  // FIXME
}

int dvbcfg_multiplex_remove_pmt_pid(struct dvbcfg_service* service,
                                    int pid,
                                    int type)
{
  // FIXME
}

struct dvbcfg_multiplex *dvbcfg_multiplex_find(struct dvbcfg_multiplex *multiplexes, char* gmidstr)
{
        struct dvbcfg_gmid gmid;
        struct dvbcfg_multiplex* multiplex;

        if (dvbcfg_gmid_from_string(gmidstr, &gmid))
                return NULL;

        multiplex = dvbcfg_multiplex_find2(multiplexes, &gmid);

        dvbcfg_source_id_free(&gmid.source_id);
        return multiplex;
}

struct dvbcfg_multiplex *dvbcfg_multiplex_find2(struct dvbcfg_multiplex *multiplexes, struct dvbcfg_gmid* gmid)
{
        while (multiplexes) {
          if (dvbcfg_source_id_equal(&gmid->source_id, &multiplexes->source->source_id, 0) &&
              dvbcfg_umid_equal(&gmid->umid, &multiplexes->umid))
                        return multiplexes;

                multiplexes = multiplexes->next;
        }

        return NULL;
}

struct dvbcfg_service *dvbcfg_multiplex_find_service_in_multiplex(struct dvbcfg_multiplex *multiplex, char* usid)
{
  // FIXME
}

struct dvbcfg_service *dvbcfg_multiplex_find_service_in_multiplex2(struct dvbcfg_multiplex *multiplex,
                                                                   struct dvbcfg_usid* usid)
{
  // FIXME
}

struct dvbcfg_service *dvbcfg_multiplex_find_service(struct dvbcfg_multiplex *multiplexes, char* gsid)
{
  // FIXME
}

struct dvbcfg_service *dvbcfg_multiplex_find_service2(struct dvbcfg_multiplex *multiplexes, struct dvbcfg_gsid* gsid)
{
  // FIXME
}

void dvbcfg_multiplex_free(struct dvbcfg_multiplex **multiplexes,
                           struct dvbcfg_multiplex *tofree)
{
        struct dvbcfg_multiplex *next;
        struct dvbcfg_multiplex *cur;

        next = tofree->next;

        /* free internal structures */
        while(tofree->services)
                dvbcfg_remove_service(tofree, tofree->services);
        free(tofree);

        /* adjust pointers */
        if (*multiplexes == tofree)
                *multiplexes = next;
        else {
                cur = *multiplexes;
                while((cur->next != tofree) && (cur->next))
                        cur = cur->next;
                if (cur->next == tofree)
                        cur->next = next;
        }
}

void dvbcfg_multiplex_free_all(struct dvbcfg_multiplex *multiplexes)
{
        while (multiplexes)
                dvbcfg_multiplex_free(&multiplexes, multiplexes);
}

static int parsesetting(char* text, const struct dvbcfg_setting* settings)
{
        while(settings->name) {
                if (!strcmp(text, settings->name))
                        return settings->value;

                settings++;
        }

        return -1;
}
