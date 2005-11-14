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
        { "BANDWIDTH_6_MHZ", DVBFE_BANDWIDTH_6_MHZ },
	{ "BANDWIDTH_7_MHZ", DVBFE_BANDWIDTH_7_MHZ },
	{ "BANDWIDTH_8_MHZ", DVBFE_BANDWIDTH_8_MHZ },
	{ "BANDWIDTH_AUTO",  DVBFE_BANDWIDTH_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting guard_interval_list [] = {
	{"GUARD_INTERVAL_1_16", DVBFE_GUARD_INTERVAL_1_16},
	{"GUARD_INTERVAL_1_32", DVBFE_GUARD_INTERVAL_1_32},
	{"GUARD_INTERVAL_1_4",  DVBFE_GUARD_INTERVAL_1_4},
	{"GUARD_INTERVAL_1_8",  DVBFE_GUARD_INTERVAL_1_8},
	{"GUARD_INTERVAL_AUTO", DVBFE_GUARD_INTERVAL_AUTO},
        { NULL, -1 },
};

static const struct dvbcfg_setting hierarchy_information_list [] = {
	{ "HIERARCHY_NONE", DVBFE_HIERARCHY_NONE },
	{ "HIERARCHY_1",    DVBFE_HIERARCHY_1 },
	{ "HIERARCHY_2",    DVBFE_HIERARCHY_2 },
	{ "HIERARCHY_4",    DVBFE_HIERARCHY_4 },
	{ "HIERARCHY_AUTO", DVBFE_HIERARCHY_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting constellation_list [] = {
	{ "QPSK",     DVBFE_QPSK },
	{ "QAM_16",   DVBFE_QAM_16 },
	{ "QAM_32",   DVBFE_QAM_32 },
	{ "QAM_64",   DVBFE_QAM_64 },
	{ "QAM_128",  DVBFE_QAM_128 },
	{ "QAM_256",  DVBFE_QAM_256 },
	{ "QAM_AUTO", DVBFE_QAM_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting transmission_mode_list [] = {
	{ "TRANSMISSION_MODE_2K",   DVBFE_TRANSMISSION_MODE_2K },
	{ "TRANSMISSION_MODE_8K",   DVBFE_TRANSMISSION_MODE_8K },
	{ "TRANSMISSION_MODE_AUTO", DVBFE_TRANSMISSION_MODE_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting inversion_list[] = {
	{ "INVERSION_OFF",  DVBFE_INVERSION_OFF },
	{ "INVERSION_ON",   DVBFE_INVERSION_ON },
	{ "INVERSION_AUTO", DVBFE_INVERSION_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting fec_list[] = {
	{ "FEC_NONE", DVBFE_FEC_NONE },
	{ "FEC_1_2",  DVBFE_FEC_1_2 },
	{ "FEC_2_3",  DVBFE_FEC_2_3 },
	{ "FEC_3_4",  DVBFE_FEC_3_4 },
	{ "FEC_4_5",  DVBFE_FEC_4_5 },
	{ "FEC_5_6",  DVBFE_FEC_5_6 },
	{ "FEC_6_7",  DVBFE_FEC_6_7 },
	{ "FEC_7_8",  DVBFE_FEC_7_8 },
	{ "FEC_8_9",  DVBFE_FEC_8_9 },
	{ "FEC_AUTO", DVBFE_FEC_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting dvbc_modulation_list[] = {
	{ "QAM_16",   DVBFE_QAM_16 },
	{ "QAM_32",   DVBFE_QAM_32 },
	{ "QAM_64",   DVBFE_QAM_64 },
	{ "QAM_128",  DVBFE_QAM_128 },
	{ "QAM_256",  DVBFE_QAM_256 },
	{ "QAM_AUTO", DVBFE_QAM_AUTO },
        { NULL, -1 },
};

static const struct dvbcfg_setting atsc_modulation_list[] = {
	{ "VSB_8",    DVBFE_VSB_8 },
	{ "VSB_16",   DVBFE_VSB_16 },
	{ "QAM_64",   DVBFE_QAM_64 },
	{ "QAM_256",  DVBFE_QAM_256 },
	{ "QAM_AUTO", DVBFE_QAM_AUTO },
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
        case DVBFE_TYPE_DVBS:
        case DVBFE_TYPE_DVBC:
        case DVBFE_TYPE_DVBT:
        case DVBFE_TYPE_ATSC:
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
				dvbfe_type_t source_type,
                                struct dvbcfg_delivery *delivery)
{
        int numtokens;
        int val;
        char *linepos;
        int long_delivery = 0;

        switch(source_type) {
        case DVBFE_TYPE_DVBS:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 5)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.frequency = val;
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
                delivery->u.dvb.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* polarization */
                if (toupper(linepos[0]) == 'H')
			delivery->u.dvb.u.dvbs.polarization = DVBFE_POLARIZATION_H;
                else if (toupper(linepos[0]) == 'V')
			delivery->u.dvb.u.dvbs.polarization = DVBFE_POLARIZATION_V;
                else if (toupper(linepos[0]) == 'L')
			delivery->u.dvb.u.dvbs.polarization = DVBFE_POLARIZATION_L;
                else if (toupper(linepos[0]) == 'R')
                        delivery->u.dvb.u.dvbs.polarization = DVBFE_POLARIZATION_R;
                else
                        return -EINVAL;
                linepos = dvbcfg_nexttoken(linepos);

                /* symbol_rate */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.u.dvbs.symbol_rate = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* fec_inner */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbs.fec_inner = val;
                break;

        case DVBFE_TYPE_DVBC:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 5)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.frequency = val;
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
                delivery->u.dvb.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* symbol_rate */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.u.dvbc.symbol_rate = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* fec_inner */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbc.fec_inner = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* modulation */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, dvbc_modulation_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbc.modulation = val;
                break;

        case DVBFE_TYPE_DVBT:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 9)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.frequency = val;
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
                delivery->u.dvb.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* bandwidth */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, bandwidth_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.bandwidth = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* code_rate_HP */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.code_rate_HP = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* code_rate_LP */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, fec_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.code_rate_LP = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* constellation */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, constellation_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.constellation = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* transmission_mode */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, transmission_mode_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.transmission_mode = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* guard_interval */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, guard_interval_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.guard_interval = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* hierarchy_information */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, hierarchy_information_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.dvbt.hierarchy_information = val;
                break;

        case DVBFE_TYPE_ATSC:
                numtokens = dvbcfg_tokenise(delivery_str, " \t", -1, 1);
                if (numtokens != 3)
                        return -EINVAL;
                linepos = delivery_str;

                /* frequency */
                if (sscanf(linepos, "%i", &val) != 1)
                        return -EINVAL;
                delivery->u.dvb.frequency = val;
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
                delivery->u.dvb.inversion = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* modulation */
                if (long_delivery) {
                        if ((val = dvbcfg_parsesetting(linepos, atsc_modulation_list)) < 0)
                                return -EINVAL;
                } else {
                        if (sscanf(linepos, "%i", &val) != 1)
                                return -EINVAL;
                }
                delivery->u.dvb.u.atsc.modulation = val;
                break;

        default:
                return -EINVAL;
        }

        /* success! */
        return 0;
}

int dvbcfg_delivery_to_string(dvbfe_type_t source_type,
                              int long_delivery,
                              struct dvbcfg_delivery *delivery,
                              char* dest,
                              int destsz)
{
        char polarization = 'H';

        switch(source_type) {
        case DVBFE_TYPE_DVBS:
                switch(delivery->u.dvb.u.dvbs.polarization) {
                case DVBFE_POLARIZATION_H:
                        polarization = 'H';
                        break;

                case DVBFE_POLARIZATION_V:
                        polarization = 'V';
                        break;

                case DVBFE_POLARIZATION_L:
                        polarization = 'L';
                        break;

                case DVBFE_POLARIZATION_R:
                        polarization = 'R';
                        break;
                }

                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %c %i %s",
                                    delivery->u.dvb.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.inversion,
                                                          inversion_list),
                                    polarization,
                                    delivery->u.dvb.u.dvbs.symbol_rate,
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbs.fec_inner,
                                                          fec_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %c %i %i",
                                    delivery->u.dvb.frequency,
                                    delivery->u.dvb.inversion,
                                    polarization,
                                    delivery->u.dvb.u.dvbs.symbol_rate,
                                    delivery->u.dvb.u.dvbs.fec_inner) >= destsz)
                                return -ENOMEM;
                }
                break;

        case DVBFE_TYPE_DVBC:
                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %i %s %s",
                                    delivery->u.dvb.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.inversion,
                                                          inversion_list),
                                    delivery->u.dvb.u.dvbs.symbol_rate,
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbc.fec_inner,
                                                          fec_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbc.modulation,
                                                          dvbc_modulation_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %i %i %i",
                                     delivery->u.dvb.frequency,
                                     delivery->u.dvb.inversion,
                                     delivery->u.dvb.u.dvbs.symbol_rate,
                                     delivery->u.dvb.u.dvbc.fec_inner,
                                     delivery->u.dvb.u.dvbc.modulation) >= destsz)
                                return -ENOMEM;
                }
                break;

        case DVBFE_TYPE_DVBT:
                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %s %s %s %s %s %s %s",
                                    delivery->u.dvb.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.inversion,
                                                          inversion_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.bandwidth,
                                                          bandwidth_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.code_rate_HP,
                                                          fec_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.code_rate_LP,
                                                          fec_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.constellation,
                                                          constellation_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.transmission_mode,
                                                          transmission_mode_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.guard_interval,
                                                          guard_interval_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.dvbt.hierarchy_information,
                                                          hierarchy_information_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %i %i %i %i %i %i %i",
                                    delivery->u.dvb.frequency,
                                    delivery->u.dvb.inversion,
                                    delivery->u.dvb.u.dvbt.bandwidth,
                                    delivery->u.dvb.u.dvbt.code_rate_HP,
                                    delivery->u.dvb.u.dvbt.code_rate_LP,
                                    delivery->u.dvb.u.dvbt.constellation,
                                    delivery->u.dvb.u.dvbt.transmission_mode,
                                    delivery->u.dvb.u.dvbt.guard_interval,
                                    delivery->u.dvb.u.dvbt.hierarchy_information) >= destsz)
                                return -ENOMEM;
                }
                break;

        case DVBFE_TYPE_ATSC:
                if (long_delivery) {
                        if (snprintf(dest, destsz, "%i %s %s",
                                    delivery->u.dvb.frequency,
                                    dvbcfg_lookupsetting(delivery->u.dvb.inversion,
                                                         inversion_list),
                                    dvbcfg_lookupsetting(delivery->u.dvb.u.atsc.modulation,
                                                         atsc_modulation_list)) >= destsz)
                                return -ENOMEM;
                } else {
                        if (snprintf(dest, destsz, "%i %i %i",
                                     delivery->u.dvb.frequency,
                                     delivery->u.dvb.inversion,
                                     delivery->u.dvb.u.atsc.modulation) >= destsz)
                                return -ENOMEM;
                }
                break;
        }

        return 0;
}
