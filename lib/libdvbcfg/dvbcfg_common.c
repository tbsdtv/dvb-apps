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
#include <dvbcfg_common.h>
#include "dvbcfg_util.h"

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
        region_start = strchr(string, '-');
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
