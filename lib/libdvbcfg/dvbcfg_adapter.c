/**
 * dvbcfg_adapter configuration file support.
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
#include <errno.h>
#include "dvbcfg_adapter.h"
#include "dvbcfg_source.h"
#include "dvbcfg_util.h"


int dvbcfg_adapter_load(struct dvbcfg_adapter_backend *backend,
                        struct dvbcfg_adapter **adapters)
{
        int status;
        while(!(status = backend->get(backend, adapters)));

        if (status < 0)
                return status;

        return 0;
}

int dvbcfg_adapter_save(struct dvbcfg_adapter_backend *backend,
                        struct dvbcfg_adapter *adapters)
{
        int status;

        while(adapters) {
                if ((status = backend->put(backend, adapters)) != 0)
                        return status;

                adapters = adapters->next;
        }

        return 0;
}

struct dvbcfg_adapter* dvbcfg_adapter_new(struct dvbcfg_adapter** adapters, char* adapter_id)
{
        struct dvbcfg_adapter* newadapter;
        struct dvbcfg_adapter* curadapter;

        /* create new structure */
        newadapter = (struct dvbcfg_adapter*) malloc(sizeof(struct dvbcfg_adapter));
        if (newadapter == NULL)
                return NULL;
        memset(newadapter, 0, sizeof(struct dvbcfg_adapter));
        newadapter->adapter_id = dvbcfg_strdupandtrim(adapter_id, -1);
        if (newadapter->adapter_id == NULL) {
                free(newadapter);
                return NULL;
        }

        /* add it to the list */
        if (*adapters == NULL)
                *adapters = newadapter;
        else {
                curadapter = *adapters;
                while(curadapter->next)
                        curadapter = curadapter->next;
                curadapter->next = newadapter;
        }

        return newadapter;
}

int dvbcfg_adapter_add_source(struct dvbcfg_adapter* adapter, struct dvbcfg_source* source)
{
        struct dvbcfg_source** tmp = adapter->sources;

        if (adapter->sources == NULL) {
                adapter->sources = (struct dvbcfg_source**) malloc(sizeof(struct dvbcfg_source*));
                if (adapter->sources == NULL)
                        return -ENOMEM;
                adapter->sources[0] = source;
                adapter->sources_count = 1;
        } else {
                adapter->sources = (struct dvbcfg_source**) realloc(adapter->sources,
                                                                    sizeof(struct dvbcfg_source*) * (adapter->sources_count+1));
                if (adapter->sources == NULL) {
                        adapter->sources = tmp;
                        return -ENOMEM;
                }
                adapter->sources[adapter->sources_count++] = source;
        }

        return 0;
}

int dvbcfg_adapter_remove_source(struct dvbcfg_adapter* adapter, struct dvbcfg_source* source)
{
        struct dvbcfg_source** tmp;
        int i;

        if (adapter->sources == NULL)
                return -EINVAL;

        for (i=0; i< adapter->sources_count; i++) {
                if (adapter->sources[i] == source)
                        break;
        }
        if (i >= adapter->sources_count)
                return -EINVAL;

        tmp = (struct dvbcfg_source**) malloc(sizeof(struct dvbcfg_source*) * (adapter->sources_count-1));
        if (tmp == NULL)
                return -ENOMEM;
        memcpy(tmp, adapter->sources, sizeof(struct dvbcfg_source*) * i);
        memcpy(tmp + (sizeof(struct dvbcfg_source*) * i),
               adapter->sources + (sizeof(struct dvbcfg_source*) * (i + 1)),
               sizeof(struct dvbcfg_source*) * (adapter->sources_count - i - 1));

        free(adapter->sources);
        adapter->sources = tmp;
        adapter->sources_count--;

        return 0;
}

struct dvbcfg_adapter *dvbcfg_adapter_find(struct dvbcfg_adapter *adapters,
                                           char *adapter_id)
{
        while (adapters) {
                if (!strcmp(adapter_id, adapters->adapter_id))
                        return adapters;

                adapters = adapters->next;
        }

        return NULL;
}

struct dvbcfg_adapter *dvbcfg_adapter_find_source(struct dvbcfg_adapter *adapters,
                                                  struct dvbcfg_source* source)
{
        while (adapters) {
                if (dvbcfg_adapter_supports_source(adapters, source))
                        return adapters;

                adapters = adapters->next;
        }

        return NULL;
}

int dvbcfg_adapter_supports_source(struct dvbcfg_adapter *adapter, struct dvbcfg_source* source)
{
        int i;

        for (i=0; i< adapter->sources_count; i++) {
                if (source == adapter->sources[i])
                        return 1;
        }

        return 0;
}

void dvbcfg_adapter_free(struct dvbcfg_adapter **adapters,
                         struct dvbcfg_adapter *tofree)
{
        struct dvbcfg_adapter *next;
        struct dvbcfg_adapter *cur;

        next = tofree->next;

        /* free internal structures */
        if (tofree->adapter_id)
                free(tofree->adapter_id);
        if (tofree->sources)
                free(tofree->sources);
        free(tofree);

        /* adjust pointers */
        if (*adapters == tofree)
                *adapters = next;
        else {
                cur = *adapters;
                while((cur->next != tofree) && (cur->next))
                        cur = cur->next;
                if (cur->next == tofree)
                        cur->next = next;
        }
}

void dvbcfg_adapter_free_all(struct dvbcfg_adapter *adapters)
{
        while (adapters)
                dvbcfg_adapter_free(&adapters, adapters);
}
