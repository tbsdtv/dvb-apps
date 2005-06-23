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
#include <stdlib.h>
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

static int parse_pids(struct dvbcfg_multiplex* multiplex, struct dvbcfg_service* service, char* line, int type);
static void format_pids(FILE* out, char*key, int count, struct dvbcfg_pid* pids);
static int add_pid(int* count, struct dvbcfg_pid** pids, int pid, int type);
static int remove_pid(int* count, struct dvbcfg_pid** pids, int pid, int type);


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
        int versionok = 0;
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

        while (1) {
                if (fgets(curline, sizeof(curline), in) == NULL) {
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
                        }
                        break;
                }
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
                                if (!versionok) {
                                        error = -EINVAL;
                                        goto exit;
                                }
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
                                versionok = 1;
                        } else if (value = dvbcfg_iskey(linepos, "date")) {
                                // ignore this
                        } else {
                                error = -EINVAL;
                                goto exit;
                        }
                        break;

                case LOCATION_MULTIPLEX:
                        if (value = dvbcfg_iskey(linepos, "gmid")) {
                                tmpgmid = dvbcfg_strdupandtrim(value, -1);
                        } else if (value = dvbcfg_iskey(linepos, "delivery")) {
                                tmpdelivery = dvbcfg_strdupandtrim(value, -1);
                        } else {
                                error = -EINVAL;
                                goto exit;
                        }
                        break;

                case LOCATION_SERVICE:
                        if (value = dvbcfg_iskey(linepos, "usid")) {
                                tmpusid = dvbcfg_strdupandtrim(value, -1);
                        } else if (value = dvbcfg_iskey(linepos, "name")) {
                                tmpname = dvbcfg_strdupandtrim(value, -1);
                        } else if (value = dvbcfg_iskey(linepos, "flags")) {
                                tmpflags = dvbcfg_strdupandtrim(value, -1);
                        } else if (value = dvbcfg_iskey(linepos, "ca_systems")) {
                                tmpca_systems = dvbcfg_strdupandtrim(value, -1);
                        } else if (value = dvbcfg_iskey(linepos, "zap_pids")) {
                                tmpzap_pids = dvbcfg_strdupandtrim(value, -1);
                        } else if (value = dvbcfg_iskey(linepos, "pmt_extra")) {
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

        /* validate nonoptional parameters */
        if ((tmpgmid == NULL) || (tmpdelivery == NULL))
          return NULL;

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

                /* symbol_rate */
                if (sscanf(linepos, "%i", &val) != 1)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.qam.symbol_rate = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* fec_inner */
                if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.qam.fec_inner = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* modulation */
                if ((val = dvbcfg_parsesetting(linepos, qam_modulation_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.qam.modulation = val;

                break;

        case DVBCFG_SOURCETYPE_DVBT:
                numtokens = dvbcfg_tokenise(tmpdelivery, " \t", -1, 1);
                if (numtokens != 9)
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

                /* bandwidth */
                if ((val = dvbcfg_parsesetting(linepos, bandwidth_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.bandwidth = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* code_rate_HP */
                if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.code_rate_HP = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* code_rate_LP */
                if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.code_rate_LP = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* constellation */
                if ((val = dvbcfg_parsesetting(linepos, constellation_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.constellation = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* transmission_mode */
                if ((val = dvbcfg_parsesetting(linepos, transmission_mode_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.transmission_mode = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* guard_interval */
                if ((val = dvbcfg_parsesetting(linepos, guard_interval_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.guard_interval = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* hierarchy_information */
                if ((val = dvbcfg_parsesetting(linepos, hierarchy_information_list)) < 0)
                        goto error;
                multiplex->delivery.dvb.fe_params.u.ofdm.hierarchy_information = val;

                break;

        case DVBCFG_SOURCETYPE_ATSC:
                numtokens = dvbcfg_tokenise(tmpdelivery, " \t", -1, 1);
                if (numtokens != 3)
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
        uint32_t flags = 0;
        struct dvbcfg_service* service;
        int numtokens;
        char* linepos;
        int val;

        /* validate nonoptional parameters */
        if ((tmpname == NULL) || (tmpusid == NULL))
                return NULL;

        /* parse the flags */
        if (tmpflags != NULL) {
                linepos = tmpflags;
                numtokens = dvbcfg_tokenise(linepos, " \t", -1, 1);
                while(numtokens--) {
                        if (!strcmp(linepos, "ignorepmt"))
                                flags |= DVBCFG_SERVICE_FLAG_IGNOREPMT;

                        linepos = dvbcfg_nexttoken(linepos);
                }
        }

        /* create the service itself */
        service = dvbcfg_multiplex_add_service(multiplex, tmpname, tmpusid, flags);
        if (service == NULL)
                return NULL;

        /* parse the CA systems */
        if (tmpca_systems != NULL) {
                linepos = tmpca_systems;
                numtokens = dvbcfg_tokenise(linepos, " \t", -1, 1);
                while(numtokens--) {
                        if (sscanf(linepos, "%i", &val) == 1) {
                                if (dvbcfg_multiplex_add_ca_system(service, val)) {
                                        dvbcfg_multiplex_remove_service(multiplex, service);
                                        return NULL;
                                }
                        }

                        linepos = dvbcfg_nexttoken(linepos);
                }
        }

        /* parse the zap pids */
        if (parse_pids(multiplex, service, tmpzap_pids, 0)) {
                dvbcfg_multiplex_remove_service(multiplex, service);
                return NULL;
        }

        /* parse the pmt extra */
        if (parse_pids(multiplex, service, tmppmt_extra, 1)) {
                dvbcfg_multiplex_remove_service(multiplex, service);
                return NULL;
        }

        return service;
}

static int parse_pids(struct dvbcfg_multiplex* multiplex, struct dvbcfg_service* service, char* line, int parse_type)
{
        int pid;
        int type;
        char* linepos;
        int numtokens;

        if (line == NULL)
                return 0;

        linepos = line;
        numtokens = dvbcfg_tokenise(linepos, " \t:", -1, 1);
        if ((numtokens % 2) != 0)
                return 0;

        while(numtokens) {
                /* the pid */
                if (sscanf(linepos, "%i", &pid) != 1)
                        break;
                linepos = dvbcfg_nexttoken(linepos);

                /* the type */
                if (sscanf(linepos, "%i", &type) != 1) {
                        if (!strcmp(linepos, "_ac3"))
                                type = DVBCFG_PIDTYPE_AC3;
                        else if (!strcmp(linepos, "_dts"))
                                type = DVBCFG_PIDTYPE_DTS;
                        else if (!strcmp(linepos, "_tt"))
                                type = DVBCFG_PIDTYPE_TT;
                        else if (!strcmp(linepos, "_pcr"))
                                type = DVBCFG_PIDTYPE_PCR;
                        else
                                break;
                }
                linepos = dvbcfg_nexttoken(linepos);

                /* add it in */
                switch(parse_type) {
                case 0:
                        if (dvbcfg_multiplex_add_zap_pid(service, pid, type)) {
                                return -ENOMEM;
                        }
                        break;

                case 1:
                        if (dvbcfg_multiplex_add_pmt_extra(service, pid, type)) {
                                return -ENOMEM;
                        }
                        break;
                }

                /* have process two tokens */
                numtokens-=2;
        }

        return 0;
}

int dvbcfg_multiplex_save(char *config_file, struct dvbcfg_multiplex *multiplexes)
{
        FILE *out;
        char* umid;
        char* usid;
        char polarization;
        char* source_id;
        struct dvbcfg_service* service;
        int i;

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
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.inversion, inversion_list),
                                polarization,
                                multiplexes->delivery.dvb.fe_params.u.qpsk.symbol_rate,
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.u.qpsk.fec_inner, fec_list));
                        break;

                case DVBCFG_SOURCETYPE_DVBC:
                        fprintf(out, "%i %s %i %s %s\n",
                                multiplexes->delivery.dvb.fe_params.frequency,
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.inversion, inversion_list),
                                multiplexes->delivery.dvb.fe_params.u.qpsk.symbol_rate,
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.u.qam.fec_inner, fec_list),
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.u.qam.modulation, qam_modulation_list));
                        break;

                case DVBCFG_SOURCETYPE_DVBT:
                        fprintf(out, "%i %s %s %s %s %s %s %s %s\n",
                                multiplexes->delivery.dvb.fe_params.frequency,
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.inversion, inversion_list),
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.u.ofdm.bandwidth, bandwidth_list),
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.u.ofdm.code_rate_HP, fec_list),
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.u.ofdm.code_rate_LP, fec_list),
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.u.ofdm.constellation, constellation_list),
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.u.ofdm.transmission_mode,
                                                  transmission_mode_list),
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.u.ofdm.guard_interval, guard_interval_list),
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.u.ofdm.hierarchy_information,
                                                hierarchy_information_list));

                        break;

                case DVBCFG_SOURCETYPE_ATSC:
                        fprintf(out, "%i %s %s\n",
                                multiplexes->delivery.dvb.fe_params.frequency,
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.inversion, inversion_list),
                                dvbcfg_lookupsetting(multiplexes->delivery.dvb.fe_params.u.vsb.modulation, atsc_modulation_list));
                        break;
                }
                fprintf(out, "\n");

                /* output services */
                service = multiplexes->services;
                while(service) {

                        usid = dvbcfg_usid_to_string(&service->usid);
                        if (usid == NULL)
                                break;

                        fprintf(out, "[service]\n");
                        fprintf(out, "usid=%s\n", usid);
                        fprintf(out, "name=%s\n", service->name);
                        free(usid);
                        fprintf(out, "flags=");
                        if (service->service_flags & DVBCFG_SERVICE_FLAG_IGNOREPMT)
                                fprintf(out, "ignorepmt ");
                        fprintf(out, "\n");

                        if (service->ca_systems_count) {
                                fprintf(out, "ca_systems=");
                                for(i=0; i< service->ca_systems_count; i++) {
                                        fprintf(out, "0x%x ", service->ca_systems[i]);
                                }
                                fprintf(out, "\n");
                        }

                        format_pids(out, "zap_pids", service->zap_pids_count, service->zap_pids);
                        format_pids(out, "pmt_extra", service->pmt_extra_count, service->pmt_extra);
                        fprintf(out, "\n");

                        service = service->next;
                }


                multiplexes = multiplexes->next;
        }

        fclose(out);
        return 0;
}

static void format_pids(FILE* out, char*key, int count, struct dvbcfg_pid* pids)
{
        int i;

        if (count) {
                fprintf(out, "%s=", key);
                for(i=0; i< count; i++) {
                        fprintf(out, "0x%x:", pids[i].pid);

                        if ((pids[i].type & 0xff00) == 0) {
                                fprintf(out, "0x%x", pids[i].type);
                        } else {
                                switch(pids[i].type) {
                                case DVBCFG_PIDTYPE_AC3:
                                        fprintf(out, "_ac3");
                                        break;

                                case DVBCFG_PIDTYPE_DTS:
                                        fprintf(out, "_dts");
                                        break;

                                case DVBCFG_PIDTYPE_TT:
                                        fprintf(out, "_tt");
                                        break;

                                case DVBCFG_PIDTYPE_PCR:
                                        fprintf(out, "_pcr");
                                        break;
                                }
                        }
                        fprintf(out, " ");
                }
                fprintf(out, "\n");
        }
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
                                                    char* usidstr,
                                                    uint32_t service_flags)
{
        struct dvbcfg_usid usid;
        struct dvbcfg_service* service;

        if (dvbcfg_usid_from_string(usidstr, &usid))
               return NULL;

        return dvbcfg_multiplex_add_service2(multiplex, name, &usid, service_flags);
}

struct dvbcfg_service* dvbcfg_multiplex_add_service2(struct dvbcfg_multiplex* multiplex,
                                                     char* name,
                                                     struct dvbcfg_usid* usid,
                                                     uint32_t service_flags)
{
        struct dvbcfg_service* service;
        struct dvbcfg_service* curservice;

        service = (struct dvbcfg_service*) malloc(sizeof(struct dvbcfg_service));
        if (service == NULL)
                return NULL;
        memset(service, 0, sizeof(struct dvbcfg_service));

        memcpy(&service->usid, usid, sizeof(struct dvbcfg_usid));
        service->name = dvbcfg_strdupandtrim(name, -1);
        service->service_flags = service_flags;

        /* add it to the list */
        if (multiplex->services == NULL)
                multiplex->services = service;
        else {
                curservice = multiplex->services;
                while(curservice->next)
                        curservice = curservice->next;
                curservice->next = service;
        }

        return service;
}

void dvbcfg_multiplex_remove_service(struct dvbcfg_multiplex* multiplex,
                                    struct dvbcfg_service* service)
{
        struct dvbcfg_service *next;
        struct dvbcfg_service *cur;

        next = service->next;

        /* free internal structures */
        if (service->name)
                free(service->name);
        free(service->ca_systems);
        free(service->zap_pids);
        free(service->pmt_extra);
        free(service);

        /* adjust pointers */
        if (multiplex->services == service)
                multiplex->services = next;
        else {
                cur = multiplex->services;
                while((cur->next != service) && (cur->next))
                        cur = cur->next;
                if (cur->next == service)
                        cur->next = next;
        }
}

int dvbcfg_multiplex_add_ca_system(struct dvbcfg_service* service,
                                   uint16_t ca_system_id)
{
        uint16_t* tmp;
        int i;

        /* check it isn't already there */
        for(i=0; i< service->ca_systems_count; i++) {
                if (service->ca_systems[i] == ca_system_id)
                        return 0;
        }

        if (service->ca_systems_count == 0) {
                service->ca_systems = (uint16_t*) malloc(sizeof(uint16_t));
                if (service->ca_systems == NULL)
                        return -ENOMEM;
                service->ca_systems[0] = ca_system_id;
                service->ca_systems_count = 1;
        } else {
                tmp = service->ca_systems;
                service->ca_systems = (uint16_t*) realloc(service->ca_systems, sizeof(uint16_t) * (service->ca_systems_count + 1));
                if (service->ca_systems == NULL) {
                        service->ca_systems = tmp;
                        return -ENOMEM;
                }
                service->ca_systems[service->ca_systems_count++] = ca_system_id;
        }

        return 0;
}

int dvbcfg_multiplex_remove_ca_system(struct dvbcfg_service* service,
                                      uint16_t ca_system_id)
{
        uint16_t* tmp;
        int i;

        if (service->ca_systems == NULL)
                return -EINVAL;

        for(i=0; i< service->ca_systems_count; i++) {
                if (service->ca_systems[i] == ca_system_id)
                        break;
        }
        if (i >= service->ca_systems_count)
                return -EINVAL;

        tmp = (uint16_t*) malloc(sizeof(uint16_t) * (service->ca_systems_count-1));
        if (tmp == NULL)
                return -ENOMEM;
        memcpy(tmp, service->ca_systems, sizeof(uint16_t) * i);
        memcpy(tmp + (sizeof(uint16_t) * i),
              service->ca_systems + (sizeof(uint16_t) * (i + 1)),
              sizeof(uint16_t) * (service->ca_systems_count - i - 1));

        free(service->ca_systems);
        service->ca_systems = tmp;
        service->ca_systems_count--;

        return 0;
}

int dvbcfg_multiplex_add_zap_pid(struct dvbcfg_service* service,
                                 int pid,
                                 int type)
{
        return add_pid(&service->zap_pids_count, &service->zap_pids, pid, type);
}

int dvbcfg_multiplex_remove_zap_pid(struct dvbcfg_service* service,
                                    int pid,
                                    int type)
{
        return remove_pid(&service->zap_pids_count, &service->zap_pids, pid, type);
}

int dvbcfg_multiplex_add_pmt_extra(struct dvbcfg_service* service,
                                 int pid,
                                 int type)
{
        return add_pid(&service->pmt_extra_count, &service->pmt_extra, pid, type);
}

int dvbcfg_multiplex_remove_pmt_extra(struct dvbcfg_service* service,
                                      int pid,
                                      int type)
{
        return remove_pid(&service->pmt_extra_count, &service->pmt_extra, pid, type);
}

static int add_pid(int* count, struct dvbcfg_pid** pids, int pid, int type) {
        struct dvbcfg_pid* tmp;
        int i;

        /* check it isn't already there */
        for(i=0; i< *count; i++) {
                if (((*pids)[i].pid == pid) && ((*pids[i]).type == type))
                        return 0;
        }

        if (*count == 0) {
                *pids = (struct dvbcfg_pid*) malloc(sizeof(struct dvbcfg_pid));
                if (*pids == NULL)
                        return -ENOMEM;
                (*pids)[0].pid = pid;
                (*pids)[0].type = type;
                *count = 1;
        } else {
                tmp = *pids;
                *pids = (struct dvbcfg_pid*) realloc(*pids, sizeof(struct dvbcfg_pid) * (*count + 1));
                if (*pids == NULL) {
                        *pids = tmp;
                        return -ENOMEM;
                }
                (*pids)[*count].pid = pid;
                (*pids)[*count].type = type;
                (*count)++;
        }

        return 0;
}

static int remove_pid(int* count, struct dvbcfg_pid** pids, int pid, int type)
{
        struct dvbcfg_pid* tmp;
        int i;

        if (*pids == NULL)
                return -EINVAL;

        if ((pid == -1) && (type == -1)) {
                free(*pids);
                *pids = NULL;
                *count = 0;
                return 0;
        }

        /* we keep removing until there are no more matches */
        while(1) {
                /* does the pid/type combo exist? */
                for(i=0; i< *count; i++) {
                        if (((pid == -1) || ((*pids)[i].pid == pid)) &&
                            ((type == -1) || ((*pids)[i].type == type)))
                                break;
                }
                if (i >= *count)
                        break;

                tmp = (struct dvbcfg_pid*) malloc(sizeof(struct dvbcfg_pid) * (*count-1));
                if (tmp == NULL)
                        return -ENOMEM;
                memcpy(tmp, *pids, sizeof(struct dvbcfg_pid) * i);
                memcpy(tmp + (sizeof(struct dvbcfg_pid) * i),
                      *pids + (sizeof(struct dvbcfg_pid) * (i + 1)),
                      sizeof(struct dvbcfg_pid) * (*count - i - 1));

                free(*pids);
                *pids = tmp;
                (*count)--;
        }

        return 0;
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

struct dvbcfg_service *dvbcfg_multiplex_find_service_in_multiplex(struct dvbcfg_multiplex *multiplex, char* usidstr)
{
        struct dvbcfg_usid usid;

        if (dvbcfg_usid_from_string(usidstr, &usid))
                return NULL;

        return dvbcfg_multiplex_find_service_in_multiplex2(multiplex, &usid);
}

struct dvbcfg_service *dvbcfg_multiplex_find_service_in_multiplex2(struct dvbcfg_multiplex *multiplex,
                                                                   struct dvbcfg_usid* usid)
{
        struct dvbcfg_service* service;

        service = multiplex->services;
        while(service) {
                if (dvbcfg_usid_equal(&service->usid, usid))
                        return service;

                service = service->next;
        }

        return NULL;
}

struct dvbcfg_service *dvbcfg_multiplex_find_service(struct dvbcfg_multiplex *multiplexes, char* gsidstr)
{
        struct dvbcfg_gsid gsid;
        struct dvbcfg_service* service;

        if (dvbcfg_gsid_from_string(gsidstr, &gsid))
                return NULL;

        service = dvbcfg_multiplex_find_service2(multiplexes, &gsid);
        dvbcfg_source_id_free(&gsid.gmid.source_id);

        return service;
}

struct dvbcfg_service *dvbcfg_multiplex_find_service2(struct dvbcfg_multiplex *multiplexes, struct dvbcfg_gsid* gsid)
{
        struct dvbcfg_multiplex* multiplex;

        multiplex = dvbcfg_multiplex_find2(multiplexes, &gsid->gmid);
        if (multiplex == NULL)
                return NULL;

        return dvbcfg_multiplex_find_service_in_multiplex2(multiplex, &gsid->usid);
}

void dvbcfg_multiplex_free(struct dvbcfg_multiplex **multiplexes,
                           struct dvbcfg_multiplex *tofree)
{
        struct dvbcfg_multiplex *next;
        struct dvbcfg_multiplex *cur;

        next = tofree->next;

        /* free internal structures */
        while(tofree->services)
                dvbcfg_multiplex_remove_service(tofree, tofree->services);
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

void dvbcfg_multiplex_postprocess(struct dvbcfg_multiplex** multiplexes)
{
        struct dvbcfg_multiplex* curmultiplex;
        struct dvbcfg_service* curservice;
        struct dvbcfg_multiplex* testmultiplex;
        struct dvbcfg_service* testservice;
        int i;

        /* first of all, set all differentiators to 0 */
        curmultiplex = *multiplexes;
        while(curmultiplex) {
                curmultiplex->umid.multiplex_differentiator = 0;

                curservice = curmultiplex->services;
                while(curservice) {
                        curservice->usid.service_differentiator = 0;
                        curservice = curservice->next;
                }

                curmultiplex = curmultiplex->next;
        }

        /* next, filter out any duplicate multiplexes */
        curmultiplex = *multiplexes;
        while(curmultiplex) {

restart_dupefilter:
                testmultiplex = *multiplexes;
                while(testmultiplex) {
                        if (testmultiplex != curmultiplex) {
                                if (dvbcfg_source_id_equal(&testmultiplex->source->source_id, &curmultiplex->source->source_id, 0) &&
                                    dvbcfg_umid_equal(&testmultiplex->umid, &curmultiplex->umid) &&
                                    (dvbcfg_multiplex_calculate_differentiator(testmultiplex) == dvbcfg_multiplex_calculate_differentiator(curmultiplex))) {
                                        dvbcfg_multiplex_free(multiplexes, testmultiplex);
                                        goto restart_dupefilter;
                                }
                        }

                        testmultiplex = testmultiplex->next;
                }

                curmultiplex = curmultiplex->next;
        }

        /* now, differentiate each multiplex and service where necessary */
        curmultiplex = *multiplexes;
        while(curmultiplex) {

                /* do the multiplex */
restart_differentiator:
                testmultiplex = *multiplexes;
                while(testmultiplex) {
                        if (testmultiplex != curmultiplex) {
                                if (dvbcfg_source_id_equal(&testmultiplex->source->source_id, &curmultiplex->source->source_id, 0) &&
                                    dvbcfg_umid_equal(&testmultiplex->umid, &curmultiplex->umid)) {
                                        testmultiplex->umid.multiplex_differentiator = dvbcfg_multiplex_calculate_differentiator(testmultiplex);
                                        goto restart_differentiator;
                                }
                        }

                        testmultiplex = testmultiplex->next;
                }

                /* now differentiate the services for this multiplex */
                curservice = curmultiplex->services;
                while(curservice) {

restart_differentiator_service:
                        i = 0;
                        testservice = curmultiplex->services;
                        while(testservice) {
                                if (testservice != curservice) {
                                        if (dvbcfg_usid_equal(&testservice->usid, &curservice->usid)) {
                                                testservice->usid.service_differentiator = i;
                                                goto restart_differentiator_service;
                                        }
                                }

                                i++;
                                testservice = testservice->next;
                        }

                        curservice = curservice->next;
                }

                /* next multiplex! */
                curmultiplex = curmultiplex->next;
        }
}

uint32_t dvbcfg_multiplex_calculate_differentiator(struct dvbcfg_multiplex* multiplex)
{
        uint32_t tmp;

        switch(multiplex->source->source_id.source_type) {
        case DVBCFG_SOURCETYPE_DVBS:
                tmp = multiplex->delivery.dvb.fe_params.frequency / (multiplex->delivery.dvb.fe_params.u.qpsk.symbol_rate / 1000);
                tmp <<= 2;
                tmp |= (multiplex->delivery.dvb.polarization & 3);
                return tmp;

        case DVBCFG_SOURCETYPE_DVBC:
                return multiplex->delivery.dvb.fe_params.frequency / multiplex->delivery.dvb.fe_params.u.qam.symbol_rate;

        case DVBCFG_SOURCETYPE_DVBT:
                switch(multiplex->delivery.dvb.fe_params.u.ofdm.bandwidth) {
                case BANDWIDTH_8_MHZ:
                        tmp = 8000000;
                        break;

                case BANDWIDTH_7_MHZ:
                        tmp = 7000000;
                        break;

                case BANDWIDTH_AUTO:
                case BANDWIDTH_6_MHZ:
                        tmp = 6000000;
                        break;
                }
                return multiplex->delivery.dvb.fe_params.frequency / tmp;

        case DVBCFG_SOURCETYPE_ATSC:
                return multiplex->delivery.dvb.fe_params.frequency / 6000000;
        }

        return 0;
}
