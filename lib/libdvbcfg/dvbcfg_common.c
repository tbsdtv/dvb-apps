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
#include <errno.h>
#include <dvbcfg_common.h>
#include "dvbcfg_util.h"

char* dvbcfg_source_id_to_string(struct dvbcfg_source_id* source_id)
{
        int len = 2;
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
        ptr += sprintf(ptr, "%c%s", source_id->source_type, source_id->source_network);
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
        char* region_start = NULL;
        char* locale_start = NULL;
        int len;

        memset(source_id, 0, sizeof(struct dvbcfg_source_id));

        switch(string[0]) {
        case DVBCFG_SOURCETYPE_DVBS:
        case DVBCFG_SOURCETYPE_DVBC:
        case DVBCFG_SOURCETYPE_DVBT:
        case DVBCFG_SOURCETYPE_ATSC:
                break;

        default:
                return NULL;
        }
        source_id->source_type = string[0];
        string++;

        network_len = strlen(string);
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

        source_id->source_network = dvbcfg_strdupandtrim(string, network_len);
        if (source_id->source_network == NULL) {
                if (source_id->source_locale)
                        free(source_id->source_locale);
                if (source_id->source_region)
                        free(source_id->source_region);
                return -ENOMEM;
        }

        return 0;
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
        char* sid;
        int len;

        sid = dvbcfg_source_id_to_string(&umid->source_id);
        if (sid == NULL)
                return NULL;

        len = snprintf(tmp, sizeof(tmp), "%s:0x%x:0x%x:0x%x",
                      sid, umid->original_network_id, umid->transport_stream_id, umid->multiplex_differentiator);
        free(sid);
        if (len >= sizeof(tmp))
                return NULL;

        return strdup(tmp);
}

int dvbcfg_umid_from_string(char* string, struct dvbcfg_umid* umid)
{
        int val;
        char* ptr;
        int result;

        if (result = dvbcfg_source_id_from_string(string, &umid->source_id))
                return result;

        if ((ptr = strchr(string, ':')) == NULL)
                return -EINVAL;

        if (sscanf(ptr+1, "%i", &val) != 1)
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

char* dvbcfg_gsid_to_string(struct dvbcfg_gsid* gsid)
{
        char* umids;
        char* usids;
        char* tmp;

        umids = dvbcfg_umid_to_string(&gsid->umid);
        usids = dvbcfg_usid_to_string(&gsid->usid);
        tmp = malloc(strlen(umids) + strlen(usids) + 2);
        if (tmp == NULL)
                return NULL;

        sprintf(tmp, "%s:%s", umids, usids);

        free(umids);
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
        if (result = dvbcfg_usid_from_string(ptr, &gsid->usid))
                return result;

        /* parse the UMID now */
        return dvbcfg_umid_from_string(string, &gsid->umid);
}
