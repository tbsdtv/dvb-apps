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
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "dvbcfg_multiplex_backend_file.h"
#include "dvbcfg_util.h"

#define DVBCFG_DEFAULT_MULTIPLEX_FILENAME ("DVBCFG_DEFAULT_DIR" "/multiplexes.conf")

#define LOCATION_SOF 0
#define LOCATION_DVBMULTIPLEXES 1
#define LOCATION_MULTIPLEX 2
#define LOCATION_SERVICE 3
#define LOCATION_EOF 4

struct dvbcfg_multiplex_backend_file {
        struct dvbcfg_multiplex_backend api;

        char* filename;
        FILE* inhandle;
        FILE* outhandle;
        struct dvbcfg_source** sources;
        int create_sources;
        int long_delivery;

        int location;
        char* tmpgmid;
        char* tmpdelivery;
        char* tmpusid;
        char* tmpname;
        char* tmpshortname;
        char* tmpprovidername;
        char* tmpflags;
        char* tmpca_systems;
        char* tmpzap_pids;
        char* tmppmt_extra;
};

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

static int get_multiplex(struct dvbcfg_multiplex_backend* backend,
                         struct dvbcfg_multiplex** multiplexes);
static int get_service(struct dvbcfg_multiplex_backend* backend,
                       struct dvbcfg_multiplex* multiplex);
static int put_multiplex(struct dvbcfg_multiplex_backend* backend,
                         struct dvbcfg_multiplex* multiplex);
static int put_service(struct dvbcfg_multiplex_backend* backend,
                       struct dvbcfg_service* service);

static struct dvbcfg_multiplex* parse_multiplex(struct dvbcfg_source** sources,
                                                struct dvbcfg_multiplex** multiplexes,
                                                int create_sources,
                                                char* tmpgmid,
                                                char* tmpdelivery);

static struct dvbcfg_service* parse_service(struct dvbcfg_multiplex* multiplex,
                                            char* tmpusid,
                                            char* tmpname,
                                            char* tmpshortname,
                                            char* tmpprovidername,
                                            char* tmpflags,
                                            char* tmpca_systems,
                                            char* tmpzap_pids,
                                            char* tmppmt_extra);

static int parse_pids(struct dvbcfg_multiplex* multiplex, struct dvbcfg_service* service, char* line, int type);
static void format_pids(FILE* out, char*key, int count, struct dvbcfg_pid* pids);
static void freekeys(struct dvbcfg_multiplex_backend_file* fbackend);


int dvbcfg_multiplex_backend_file_create(const char* filename,
                                         struct dvbcfg_source** sources,
                                         int create_sources,
                                         int long_delivery,
                                         struct dvbcfg_multiplex_backend** backend)
{
        struct dvbcfg_multiplex_backend_file* fbackend;

        fbackend = malloc(sizeof(struct dvbcfg_multiplex_backend_file));
        if (fbackend == NULL)
                return -ENOMEM;

        memset(fbackend, 0, sizeof(struct dvbcfg_multiplex_backend_file));
        fbackend->api.get_multiplex = get_multiplex;
        fbackend->api.get_service = get_service;
        fbackend->api.put_multiplex = put_multiplex;
        fbackend->api.put_service = put_service;

        if (filename) {
                fbackend->filename = strdup(filename);
        } else {
                fbackend->filename = strdup(DVBCFG_DEFAULT_MULTIPLEX_FILENAME);
        }
        if (fbackend->filename == NULL) {
                free(fbackend);
                return -ENOMEM;
        }
        fbackend->sources = sources;
        fbackend->create_sources = create_sources;
        fbackend->location = LOCATION_SOF;
        fbackend->long_delivery = long_delivery;

        *backend = (struct dvbcfg_multiplex_backend*) fbackend;
        return 0;
}

void dvbcfg_multiplex_backend_file_destroy(struct dvbcfg_multiplex_backend* backend)
{
        struct dvbcfg_multiplex_backend_file* fbackend =
                (struct dvbcfg_multiplex_backend_file*) backend;

        if (fbackend->inhandle != NULL)
                fclose(fbackend->inhandle);
        if (fbackend->outhandle != NULL)
                fclose(fbackend->outhandle);
        dvbcfg_freestring(&fbackend->filename);
        freekeys(fbackend);
        free(backend);
}

static int get_multiplex(struct dvbcfg_multiplex_backend* backend,
                         struct dvbcfg_multiplex** multiplexes)
{
        struct dvbcfg_multiplex_backend_file* fbackend =
                (struct dvbcfg_multiplex_backend_file*) backend;
        char curline[256];
        char *linepos;
        char *value;

        /* open the file if necessary */
        if (fbackend->inhandle == NULL) {
                fbackend->inhandle = fopen(fbackend->filename, "r");
                if (fbackend->inhandle == NULL)
                        return -errno;
        }

        /* deal with the file location */
        switch(fbackend->location) {
        case LOCATION_SOF:
        case LOCATION_DVBMULTIPLEXES:
        case LOCATION_MULTIPLEX:
                break;

        case LOCATION_SERVICE:
                return 2;

        case LOCATION_EOF:
                return 1;
        }

        /* keep reading till we get one multiplex */
        while(1) {
                /* read string and deal with EOF */
                if (fgets(curline, sizeof(curline), fbackend->inhandle) == NULL) {
                        switch(fbackend->location) {
                        case LOCATION_DVBMULTIPLEXES:
                        case LOCATION_SOF:
                                break;

                        case LOCATION_MULTIPLEX:
                                if (parse_multiplex(fbackend->sources,
                                                    multiplexes,
                                                    fbackend->create_sources,
                                                    fbackend->tmpgmid,
                                                    fbackend->tmpdelivery) == NULL) {
                                        freekeys(fbackend);
                                        return -EINVAL;
                                }
                                freekeys(fbackend);
                                fbackend->location = LOCATION_EOF;
                                return 0;

                        default:
                                return -EINVAL;
                        }

                        /* end of file */
                        fbackend->location = LOCATION_EOF;
                        return 1;
                }

                /* clean any comments/ whitespace */
                linepos = curline;
                if (dvbcfg_cleanline(linepos) == 0)
                  continue;

                /* is it a [section]? */
                if (dvbcfg_issection(linepos, "multiplexes")) {
                        switch(fbackend->location) {
                        case LOCATION_SOF:
                                break;

                        default:
                                return -EINVAL;
                        }

                        fbackend->location = LOCATION_DVBMULTIPLEXES;
                        continue;
                } else if (dvbcfg_issection(linepos, "m")) {
                        switch(fbackend->location) {
                        case LOCATION_DVBMULTIPLEXES:
                                break;

                        case LOCATION_MULTIPLEX:
                                if (parse_multiplex(fbackend->sources,
                                                    multiplexes,
                                                    fbackend->create_sources,
                                                    fbackend->tmpgmid,
                                                    fbackend->tmpdelivery) == NULL) {
                                        freekeys(fbackend);
                                        return -EINVAL;
                                }
                                freekeys(fbackend);
                                return 0;

                        default:
                                return -EINVAL;
                        }

                        /* we are now in a multiplex */
                        fbackend->location = LOCATION_MULTIPLEX;
                        continue;

                } else if (dvbcfg_issection(linepos, "s")) {
                        /* parse the immediately preceding multiplex */
                        switch(fbackend->location) {
                        case LOCATION_MULTIPLEX:
                                if (parse_multiplex(fbackend->sources,
                                                    multiplexes,
                                                    fbackend->create_sources,
                                                    fbackend->tmpgmid,
                                                    fbackend->tmpdelivery) == NULL) {
                                        freekeys(fbackend);
                                        return -EINVAL;
                                }
                                freekeys(fbackend);
                                fbackend->location = LOCATION_SERVICE;
                                return 0;

                        default:
                                return -EINVAL;
                        }

                        /* we are now in a service */
                        fbackend->location = LOCATION_SERVICE;
                        return 2;
                }

                /* deal with the keys depending on the file location */
                switch(fbackend->location) {
                case LOCATION_DVBMULTIPLEXES:
                        if ((value = dvbcfg_iskey(linepos, "version")) != NULL) {
                                if (strcmp(value, "0.1"))
                                        return -EINVAL;
                        } else {
                                return -EINVAL;
                        }
                        break;

                case LOCATION_MULTIPLEX:
                        if ((value = dvbcfg_iskey(linepos, "gmid")) != NULL) {
                                fbackend->tmpgmid = dvbcfg_strdupandtrim(value, -1);
                        } else if ((value = dvbcfg_iskey(linepos, "d")) != NULL) {
                                fbackend->tmpdelivery = dvbcfg_strdupandtrim(value, -1);
                        } else {
                                return -EINVAL;
                        }
                        break;

                default:
                        return -EINVAL;
                }
        }

        /* end of file */
        return 1;
}

static int get_service(struct dvbcfg_multiplex_backend* backend,
                       struct dvbcfg_multiplex* multiplex)
{
        struct dvbcfg_multiplex_backend_file* fbackend =
                (struct dvbcfg_multiplex_backend_file*) backend;
        char curline[256];
        char *linepos;
        char *value;

        /* deal with the file location */
        switch(fbackend->location) {
        case LOCATION_SERVICE:
                break;

        case LOCATION_SOF:
        case LOCATION_DVBMULTIPLEXES:
                return -EINVAL;

        case LOCATION_MULTIPLEX:
                return 2;

        case LOCATION_EOF:
                return 1;
        }

        /* must have opened file before calling this */
        if (fbackend->inhandle == NULL)
                return -EINVAL;

        /* keep reading till we get one adapter */
        while(1) {
                /* read string and deal with EOF */
                if (fgets(curline, sizeof(curline), fbackend->inhandle) == NULL) {
                        if (parse_service(multiplex,
                                          fbackend->tmpusid,
                                          fbackend->tmpname,
                                          fbackend->tmpshortname,
                                          fbackend->tmpprovidername,
                                          fbackend->tmpflags,
                                          fbackend->tmpca_systems,
                                          fbackend->tmpzap_pids,
                                          fbackend->tmppmt_extra) == NULL) {
                                freekeys(fbackend);
                                return -EINVAL;
                        }

                        /* now at EOF */
                        freekeys(fbackend);
                        fbackend->location = LOCATION_EOF;
                        return 0;
                }

                /* clean any comments/ whitespace */
                linepos = curline;
                if (dvbcfg_cleanline(linepos) == 0)
                        continue;

                /* is it a [section]? */
                if (dvbcfg_issection(linepos, "m")) {
                        /* parse the immediately preceding service */
                        if (parse_service(multiplex,
                                          fbackend->tmpusid,
                                          fbackend->tmpname,
                                          fbackend->tmpshortname,
                                          fbackend->tmpprovidername,
                                          fbackend->tmpflags,
                                          fbackend->tmpca_systems,
                                          fbackend->tmpzap_pids,
                                          fbackend->tmppmt_extra) == NULL) {
                                freekeys(fbackend);
                                return -EINVAL;
                        }

                        /* we are now in a multiplex */
                        freekeys(fbackend);
                        fbackend->location = LOCATION_MULTIPLEX;
                        return 0;

                } else if (dvbcfg_issection(linepos, "s")) {
                        /* parse the immediately preceding service */
                        if (parse_service(multiplex,
                                          fbackend->tmpusid,
                                          fbackend->tmpname,
                                          fbackend->tmpshortname,
                                          fbackend->tmpprovidername,
                                          fbackend->tmpflags,
                                          fbackend->tmpca_systems,
                                          fbackend->tmpzap_pids,
                                          fbackend->tmppmt_extra) == NULL) {
                                freekeys(fbackend);
                                return -EINVAL;
                        }

                        /* we are now in a service */
                        freekeys(fbackend);
                        fbackend->location = LOCATION_SERVICE;
                        return 0;
                }

                /* deal with the keys */
                if ((value = dvbcfg_iskey(linepos, "usid")) != NULL) {
                  fbackend->tmpusid = dvbcfg_strdupandtrim(value, -1);
                } else if ((value = dvbcfg_iskey(linepos, "name")) != NULL) {
                  fbackend->tmpname = dvbcfg_strdupandtrim(value, -1);
                } else if ((value = dvbcfg_iskey(linepos, "sname")) != NULL) {
                  fbackend->tmpshortname = dvbcfg_strdupandtrim(value, -1);
                } else if ((value = dvbcfg_iskey(linepos, "pname")) != NULL) {
                  fbackend->tmpprovidername = dvbcfg_strdupandtrim(value, -1);
                } else if ((value = dvbcfg_iskey(linepos, "flags")) != NULL) {
                  fbackend->tmpflags = dvbcfg_strdupandtrim(value, -1);
                } else if ((value = dvbcfg_iskey(linepos, "ca")) != NULL) {
                  fbackend->tmpca_systems = dvbcfg_strdupandtrim(value, -1);
                } else if ((value = dvbcfg_iskey(linepos, "zap")) != NULL) {
                  fbackend->tmpzap_pids = dvbcfg_strdupandtrim(value, -1);
                } else if ((value = dvbcfg_iskey(linepos, "pmt")) != NULL) {
                  fbackend->tmppmt_extra = dvbcfg_strdupandtrim(value, -1);
                } else {
                  return -EINVAL;
                }
        }

        /* end of file */
        return 1;
}

static int put_multiplex(struct dvbcfg_multiplex_backend* backend,
                         struct dvbcfg_multiplex* multiplex)
{
        struct dvbcfg_multiplex_backend_file* fbackend =
                (struct dvbcfg_multiplex_backend_file*) backend;
        char* umid;
        char* source_id;
        char tmp[256];

        /* open the file if necessary */
        if (fbackend->outhandle == NULL) {
                fbackend->outhandle = fopen(fbackend->filename, "w");
                if (fbackend->outhandle == NULL)
                        return -errno;

                /* output the file header */
                fprintf(fbackend->outhandle, "[multiplexes]\n");
                fprintf(fbackend->outhandle, "version=0.1\n");
                fprintf(fbackend->outhandle, "date=%lu\n", time(NULL));
                fprintf(fbackend->outhandle, "\n");
        }

        umid = dvbcfg_umid_to_string(&multiplex->umid);
        if (umid == NULL)
                return -ENOMEM;

        source_id = dvbcfg_source_id_to_string(&multiplex->source->source_id);
        if (source_id == NULL) {
                free(umid);
                return -ENOMEM;
        }

        fprintf(fbackend->outhandle, "[m]\n");
        fprintf(fbackend->outhandle, "gmid=%s:%s\n", source_id, umid);
        free(umid);
        free(source_id);

        fprintf(fbackend->outhandle, "d=");
        if (dvbcfg_delivery_to_string(multiplex->source->source_id.source_type,
                                      fbackend->long_delivery,
                                      &multiplex->delivery,
                                      tmp,
                                      sizeof(tmp)))
                return -ENOMEM;
        fprintf(fbackend->outhandle, "%s\n", tmp);

        return 0;
}

static int put_service(struct dvbcfg_multiplex_backend* backend,
                       struct dvbcfg_service* service)
{
        struct dvbcfg_multiplex_backend_file* fbackend =
                (struct dvbcfg_multiplex_backend_file*) backend;
        char* usid;
        int i;

        /* the must already be opened! */
        if (fbackend->outhandle == NULL)
                return -EIO;

        usid = dvbcfg_usid_to_string(&service->usid);
        if (usid == NULL)
                return -ENOMEM;

        fprintf(fbackend->outhandle, "[s]\n");
        fprintf(fbackend->outhandle, "usid=%s\n", usid);
        free(usid);
        fprintf(fbackend->outhandle, "name=%s\n", service->name);
        if (service->short_name)
                fprintf(fbackend->outhandle, "sname=%s\n", service->short_name);
        if (service->provider_name)
                fprintf(fbackend->outhandle, "pname=%s\n", service->provider_name);
        fprintf(fbackend->outhandle, "flags=");
        if (service->service_flags & DVBCFG_SERVICE_FLAG_IGNOREPMT)
                fprintf(fbackend->outhandle, "nopmt ");
        fprintf(fbackend->outhandle, "\n");

        if (service->ca_systems_count) {
                fprintf(fbackend->outhandle, "ca=");
                for (i=0; i< service->ca_systems_count; i++) {
                        fprintf(fbackend->outhandle, "0x%x ", service->ca_systems[i]);
                }
                fprintf(fbackend->outhandle, "\n");
        }

        format_pids(fbackend->outhandle, "zap", service->zap_pids_count, service->zap_pids);
        format_pids(fbackend->outhandle, "pmt", service->pmt_extra_count, service->pmt_extra);
        fprintf(fbackend->outhandle, "\n");

        return 0;
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

        /* now, parse the delivery string */
        if (dvbcfg_delivery_from_string(tmpdelivery,
                                        source->source_id.source_type,
                                        &multiplex->delivery))
                goto error;

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
                                            char* tmpshortname,
                                            char* tmpprovidername,
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
                        if (!strcmp(linepos, "nopmt"))
                                flags |= DVBCFG_SERVICE_FLAG_IGNOREPMT;

                        linepos = dvbcfg_nexttoken(linepos);
                }
        }

        /* create the service itself */
        service = dvbcfg_multiplex_add_service(multiplex, tmpname, tmpshortname,
                                               tmpprovidername, tmpusid, flags);
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

static int parse_pids(struct dvbcfg_multiplex* multiplex,
                      struct dvbcfg_service* service, char* line, int parse_type)
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

static void format_pids(FILE* out, char*key, int count, struct dvbcfg_pid* pids)
{
        int i;

        if (count) {
                fprintf(out, "%s=", key);
                for (i=0; i< count; i++) {
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

static void freekeys(struct dvbcfg_multiplex_backend_file* fbackend)
{
        dvbcfg_freestring(&fbackend->tmpgmid);
        dvbcfg_freestring(&fbackend->tmpdelivery);
        dvbcfg_freestring(&fbackend->tmpusid);
        dvbcfg_freestring(&fbackend->tmpname);
        dvbcfg_freestring(&fbackend->tmpshortname);
        dvbcfg_freestring(&fbackend->tmpprovidername);
        dvbcfg_freestring(&fbackend->tmpca_systems);
        dvbcfg_freestring(&fbackend->tmpzap_pids);
        dvbcfg_freestring(&fbackend->tmppmt_extra);
}
