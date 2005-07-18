/**
 * dvbcfg_common utilities.
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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <dvbcfg_common.h>
#include "dvbcfg_util.h"

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

char* dvbcfg_source_id_to_string(struct dvbcfg_source_id* source_id)
{
  int len = 3;
        char* str;
        char* ptr;

        if (source_id->source_network)
                len += strlen(source_id->source_network);
        if (source_id->source_region)
                len += strlen(source_id->source_region) + 1;
        if (source_id->source_locale)
                len += strlen(source_id->source_locale) + 1;

        str = malloc(len);
        if (str == NULL)
                return NULL;

        ptr = str;
        ptr += sprintf(ptr, "%c-%s", source_id->source_type, source_id->source_network);
        if (source_id->source_region)
                ptr += sprintf(ptr, "-%s", source_id->source_region);
        if (source_id->source_locale)
                ptr += sprintf(ptr, "-%s", source_id->source_locale);

        return str;
}

int dvbcfg_source_id_from_string(char* string, struct dvbcfg_source_id* source_id)
{
        int network_len = 0;
        int region_len = 0;
        int locale_len = 0;
        char* network_start = NULL;
        char* region_start = NULL;
        char* locale_start = NULL;

        memset(source_id, 0, sizeof(struct dvbcfg_source_id));

        switch(string[0]) {
        case DVBCFG_SOURCETYPE_DVBS:
        case DVBCFG_SOURCETYPE_DVBC:
        case DVBCFG_SOURCETYPE_DVBT:
        case DVBCFG_SOURCETYPE_ATSC:
                break;

        default:
                return -EINVAL;
        }
        source_id->source_type = string[0];

        network_start = strchr(string, '-');
        if (network_start == NULL)
                return -EINVAL;

        network_start++;
        network_len = strlen(network_start);
        region_start = strchr(network_start, '-');
        if (region_start) {
                region_start++;
                region_len = strlen(region_start);
                network_len -= region_len+1;

                locale_start = strchr(region_start, '-');
                if (locale_start) {
                        locale_start++;
                        locale_len = strlen(locale_start);
                        region_len -= locale_len+1;
                }
        }

        if (locale_start) {
                source_id->source_locale = dvbcfg_strdupandtrim(locale_start, locale_len);
                if (source_id->source_locale == NULL)
                        return -ENOMEM;
        }

        if (region_start) {
                source_id->source_region = dvbcfg_strdupandtrim(region_start, region_len);
                if (source_id->source_region == NULL) {
                        if (source_id->source_locale)
                                free(source_id->source_locale);
                        free(source_id->source_region);
                        return -ENOMEM;
                }
        }

        source_id->source_network = dvbcfg_strdupandtrim(network_start, network_len);
        if (source_id->source_network == NULL) {
                if (source_id->source_locale)
                        free(source_id->source_locale);
                if (source_id->source_region)
                        free(source_id->source_region);
                return -ENOMEM;
        }

        return 0;
}

int dvbcfg_source_id_equal(struct dvbcfg_source_id* source_id1, struct dvbcfg_source_id* source_id2, int fuzzy)
{
        if (source_id1->source_type != source_id2->source_type)
                return 0;

        if (!fuzzy) {
                if ((source_id1->source_network == NULL) != (source_id2->source_network == NULL))
                        return 0;
                if ((source_id1->source_region == NULL) != (source_id2->source_region == NULL))
                        return 0;
                if ((source_id1->source_locale == NULL) != (source_id2->source_locale == NULL))
                        return 0;
        }

        if (source_id1->source_network && source_id2->source_network && strcmp(source_id1->source_network, source_id2->source_network))
                return 0;
        if (source_id1->source_region && source_id2->source_region && strcmp(source_id1->source_region, source_id2->source_region))
                return 0;
        if (source_id1->source_locale && source_id2->source_locale && strcmp(source_id1->source_locale, source_id2->source_locale))
                return 0;

        return 1;
}

void dvbcfg_source_id_free(struct dvbcfg_source_id* source_id)
{
        if (source_id->source_network)
                free(source_id->source_network);
        if (source_id->source_region)
                free(source_id->source_region);
        if (source_id->source_locale)
                free(source_id->source_locale);
        memset(source_id, 0, sizeof(source_id));
}

char* dvbcfg_umid_to_string(struct dvbcfg_umid* umid)
{
        char tmp[256];
        int len;

        len = snprintf(tmp, sizeof(tmp), "0x%x:0x%x:0x%x",
                       umid->original_network_id, umid->transport_stream_id, umid->multiplex_differentiator);
        if (len >= sizeof(tmp))
                return NULL;

        return strdup(tmp);
}

int dvbcfg_umid_from_string(char* string, struct dvbcfg_umid* umid)
{
        int val;
        char* ptr;

        ptr = string;
        if (sscanf(ptr, "%i", &val) != 1)
                return -EINVAL;
        umid->original_network_id = val;

        if ((ptr = strchr(ptr+1, ':')) == NULL)
                return -EINVAL;

        if (sscanf(ptr+1, "%i", &val) != 1)
                return -EINVAL;
        umid->transport_stream_id = val;

        if ((ptr = strchr(ptr+1, ':')) == NULL)
                return -EINVAL;

        if (sscanf(ptr+1, "%i", &val) != 1)
                return -EINVAL;
        umid->multiplex_differentiator = val;

        return 0;
}

int dvbcfg_umid_equal(struct dvbcfg_umid* umid1, struct dvbcfg_umid* umid2)
{
        if ((umid1->original_network_id == umid2->original_network_id) &&
            (umid1->transport_stream_id == umid2->transport_stream_id) &&
            (umid1->multiplex_differentiator == umid2->multiplex_differentiator))
                return 1;

        return 0;
}

char* dvbcfg_gmid_to_string(struct dvbcfg_gmid* gmid)
{
        char tmp[256];
        char* sid;
        char* umid;
        int len;

        sid = dvbcfg_source_id_to_string(&gmid->source_id);
        if (sid == NULL)
                return NULL;

        umid = dvbcfg_umid_to_string(&gmid->umid);
        if (umid == NULL) {
                free(sid);
                return NULL;
        }

        len = snprintf(tmp, sizeof(tmp), "%s:%s", sid, umid);
        free(sid);
        free(umid);
        if (len >= sizeof(tmp))
                return NULL;

        return strdup(tmp);
}

int dvbcfg_gmid_from_string(char* string, struct dvbcfg_gmid* gmid)
{
        char* ptr;
        char* tmp;
        int result;

        if ((ptr = strchr(string, ':')) == NULL)
          return -EINVAL;

        tmp = dvbcfg_strdupandtrim(string, ptr - string);
        if ((result = dvbcfg_source_id_from_string(tmp, &gmid->source_id)) != 0) {
                free(tmp);
                return result;
        }
        free(tmp);

        if ((result = dvbcfg_umid_from_string(ptr+1, &gmid->umid))) {
                dvbcfg_source_id_free(&gmid->source_id);
                return result;
        }

        return 0;
}

int dvbcfg_gmid_equal(struct dvbcfg_gmid* gmid1, struct dvbcfg_gmid* gmid2)
{
        if (dvbcfg_source_id_equal(&gmid1->source_id, &gmid2->source_id, 0) &&
            dvbcfg_umid_equal(&gmid1->umid, &gmid2->umid))
                return 1;

        return 0;
}

char* dvbcfg_usid_to_string(struct dvbcfg_usid* usid)
{
        char tmp[256];
        int len;

        len = snprintf(tmp, sizeof(tmp), "0x%x:0x%x", usid->program_number, usid->service_differentiator);
        if (len >= sizeof(tmp))
                return NULL;

        return strdup(tmp);
}

int dvbcfg_usid_from_string(char* string, struct dvbcfg_usid* usid)
{
        int val;
        char* ptr;

        if (sscanf(string, "%i", &val) != 1)
                return -EINVAL;
        usid->program_number = val;

        if ((ptr = strchr(string, ':')) == NULL)
                return -EINVAL;

        if (sscanf(ptr+1, "%i", &val) != 1)
                return -EINVAL;
        usid->service_differentiator = val;

        return 0;
}

int dvbcfg_usid_equal(struct dvbcfg_usid* usid1, struct dvbcfg_usid* usid2)
{
        if ((usid1->program_number == usid2->program_number) &&
            (usid1->service_differentiator == usid2->service_differentiator))
                return 1;

        return 0;
}

char* dvbcfg_gsid_to_string(struct dvbcfg_gsid* gsid)
{
        char* gmids;
        char* usids;
        char* tmp;

        gmids = dvbcfg_gmid_to_string(&gsid->gmid);
        if (gmids == NULL)
                return NULL;
        usids = dvbcfg_usid_to_string(&gsid->usid);
        if (usids == NULL) {
                free(gmids);
                return NULL;
        }
        tmp = malloc(strlen(gmids) + strlen(usids) + 2);
        if (tmp == NULL) {
                free(gmids);
                free(usids);
                return NULL;
        }

        sprintf(tmp, "%s:%s", gmids, usids);

        free(gmids);
        free(usids);
        return tmp;
}

int dvbcfg_gsid_from_string(char* string, struct dvbcfg_gsid* gsid)
{
        char* ptr;
        int result;

        /* find the end of the UMID */
        if ((ptr = strchr(string, ':')) == NULL)
                return -EINVAL;
        if ((ptr = strchr(string, ':')) == NULL)
                return -EINVAL;
        if ((ptr = strchr(string, ':')) == NULL)
                return -EINVAL;
        if ((ptr = strchr(string, ':')) == NULL)
                return -EINVAL;

        /* parse the USID */
        if ((result = dvbcfg_usid_from_string(ptr, &gsid->usid)) != 0)
                return result;

        /* parse the UMID now */
        return dvbcfg_gmid_from_string(string, &gsid->gmid);
}

int dvbcfg_gsid_equal(struct dvbcfg_gsid* gsid1, struct dvbcfg_gsid* gsid2)
{
        if (dvbcfg_gmid_equal(&gsid1->gmid, &gsid2->gmid) &&
            dvbcfg_usid_equal(&gsid1->usid, &gsid2->usid))
                return 1;

        return 0;
}

int dvbcfg_delivery_from_string(char * delivery_str,
                                enum dvbcfg_sourcetype source_type,
                                struct dvbcfg_delivery *delivery)
{
        int numtokens;
        int val;
        char *linepos;
        int long_delivery = 0;

        switch(source_type) {
        case DVBCFG_SOURCETYPE_DVBS:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 5)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.fe_params.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* determine delivery type */
                if (!strncmp(linepos, "INVERSION_", 10))
                        long_delivery = 1;

                /* inversion */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* polarization */
                if (toupper(linepos[0]) == 'H')
                        delivery->u.dvb.polarization = DVBCFG_POLARIZATION_H;
                else if (toupper(linepos[0]) == 'V')
                        delivery->u.dvb.polarization = DVBCFG_POLARIZATION_V;
                else if (toupper(linepos[0]) == 'L')
                        delivery->u.dvb.polarization = DVBCFG_POLARIZATION_L;
                else if (toupper(linepos[0]) == 'R')
                        delivery->u.dvb.polarization = DVBCFG_POLARIZATION_R;
                else
                        return -EINVAL;
                linepos = dvbcfg_nexttoken(linepos);

                /* symbol_rate */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.fe_params.u.qpsk.symbol_rate = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* fec_inner */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.u.qpsk.fec_inner = val;
                break;

        case DVBCFG_SOURCETYPE_DVBC:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 5)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.fe_params.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* determine delivery type */
                if (!strncmp(linepos, "INVERSION_", 10))
                        long_delivery = 1;

                /* inversion */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* symbol_rate */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.fe_params.u.qam.symbol_rate = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* fec_inner */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.u.qam.fec_inner = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* modulation */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, qam_modulation_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.u.qam.modulation = val;
                break;

        case DVBCFG_SOURCETYPE_DVBT:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 9)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.fe_params.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* determine delivery type */
                if (!strncmp(linepos, "INVERSION_", 10))
                        long_delivery = 1;

                /* inversion */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* bandwidth */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, bandwidth_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.u.ofdm.bandwidth = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* code_rate_HP */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.u.ofdm.code_rate_HP = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* code_rate_LP */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.u.ofdm.code_rate_LP = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* constellation */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, constellation_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.u.ofdm.constellation = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* transmission_mode */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, transmission_mode_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.u.ofdm.transmission_mode = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* guard_interval */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, guard_interval_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.u.ofdm.guard_interval = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* hierarchy_information */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, hierarchy_information_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.u.ofdm.hierarchy_information = val;
                break;

        case DVBCFG_SOURCETYPE_ATSC:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 3)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.fe_params.frequency = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* determine delivery type */
                if (!strncmp(linepos, "INVERSION_", 10))
                        long_delivery = 1;

                /* inversion */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, inversion_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* modulation */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, atsc_modulation_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.fe_params.u.vsb.modulation = val;
                break;

        default:
                return -EINVAL;
        }

        /* success! */
        return 0;
}

int dvbcfg_delivery_to_string(enum dvbcfg_sourcetype source_type,
                              int long_delivery,
                              struct dvbcfg_delivery *delivery,
                              char* dest,
                              int destsz)
{
        char polarization = 'H';

        switch(source_type) {
        case DVBCFG_SOURCETYPE_DVBS:
                switch(delivery->u.dvb.polarization) {
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

                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %c %i %s",
                                    delivery->u.dvb.fe_params.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.inversion,
                                                          inversion_list),
                                    polarization,
                                    delivery->u.dvb.fe_params.u.qpsk.symbol_rate,
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.u.qpsk.fec_inner,
                                                          fec_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %c %i %i",
                                    delivery->u.dvb.fe_params.frequency,
                                    delivery->u.dvb.fe_params.inversion,
                                    polarization,
                                    delivery->u.dvb.fe_params.u.qpsk.symbol_rate,
                                    delivery->u.dvb.fe_params.u.qpsk.fec_inner) >= destsz)
                                return -ENOMEM;
                }
                break;

        case DVBCFG_SOURCETYPE_DVBC:
                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %i %s %s",
                                    delivery->u.dvb.fe_params.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.inversion,
                                                          inversion_list),
                                    delivery->u.dvb.fe_params.u.qpsk.symbol_rate,
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.u.qam.fec_inner,
                                                          fec_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.u.qam.modulation,
                                                          qam_modulation_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %i %i %i",
                                     delivery->u.dvb.fe_params.frequency,
                                     delivery->u.dvb.fe_params.inversion,
                                     delivery->u.dvb.fe_params.u.qpsk.symbol_rate,
                                     delivery->u.dvb.fe_params.u.qam.fec_inner,
                                     delivery->u.dvb.fe_params.u.qam.modulation) >= destsz)
                                return -ENOMEM;
                }
                break;

        case DVBCFG_SOURCETYPE_DVBT:
                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %s %s %s %s %s %s %s",
                                    delivery->u.dvb.fe_params.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.inversion,
                                                          inversion_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.u.ofdm.bandwidth,
                                                          bandwidth_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.u.ofdm.code_rate_HP,
                                                          fec_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.u.ofdm.code_rate_LP,
                                                          fec_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.u.ofdm.constellation,
                                                          constellation_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.u.ofdm.transmission_mode,
                                                          transmission_mode_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.u.ofdm.guard_interval,
                                                          guard_interval_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.u.ofdm.hierarchy_information,
                                                          hierarchy_information_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %i %i %i %i %i %i %i",
                                    delivery->u.dvb.fe_params.frequency,
                                    delivery->u.dvb.fe_params.inversion,
                                    delivery->u.dvb.fe_params.u.ofdm.bandwidth,
                                    delivery->u.dvb.fe_params.u.ofdm.code_rate_HP,
                                    delivery->u.dvb.fe_params.u.ofdm.code_rate_LP,
                                    delivery->u.dvb.fe_params.u.ofdm.constellation,
                                    delivery->u.dvb.fe_params.u.ofdm.transmission_mode,
                                    delivery->u.dvb.fe_params.u.ofdm.guard_interval,
                                    delivery->u.dvb.fe_params.u.ofdm.hierarchy_information) >= destsz)
                                return -ENOMEM;
                }
                break;

        case DVBCFG_SOURCETYPE_ATSC:
                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %s",
                                    delivery->u.dvb.fe_params.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.inversion,
                                                         inversion_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.fe_params.u.vsb.modulation,
                                                         atsc_modulation_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %i",
                                     delivery->u.dvb.fe_params.frequency,
                                     delivery->u.dvb.fe_params.inversion,
                                     delivery->u.dvb.fe_params.u.vsb.modulation) >= destsz)
                                return -ENOMEM;
                }
                break;
        }

        return 0;
}
