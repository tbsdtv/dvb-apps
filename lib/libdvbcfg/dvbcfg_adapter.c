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


int dvbcfg_adapter_load(const char *config_file,
                        struct dvbcfg_source** sources,
                        struct dvbcfg_adapter **adapters,
                        int create_sources)
{
        FILE *in;
        char curline[256];
        char *linepos;
        struct dvbcfg_adapter *newadapter;
        struct dvbcfg_source_id source_id;
        struct dvbcfg_source* source;
        int numtokens;
        int error = 0;
        int i;

        /* open the file */
        in = fopen(config_file, "r");
        if (in == NULL)
                return errno;

        while (fgets(curline, sizeof(curline), in)) {
                linepos = curline;

                /* clean any comments/ whitespace */
                if (dvbcfg_cleanline(linepos) == 0)
                        continue;

                /* tokenise the line */
                numtokens = dvbcfg_tokenise(linepos, " \t", -1, 1);
                if (numtokens < 1) {
                        continue;
                }

                /* create the new adapter */
                newadapter = dvbcfg_adapter_new(adapters, linepos);
                if (newadapter == NULL) {
                        error = -ENOMEM;
                        goto exit;
                }
                linepos = dvbcfg_nexttoken(linepos);

                /* the source_ids */
                for (i = 1; i < numtokens; i++) {
                        if (dvbcfg_source_id_from_string(linepos, &source_id)) {
                                dvbcfg_adapter_free(adapters, newadapter);
                                error = -ENOMEM;
                                goto exit;
                        }

                        /* try to find it */
                        source = dvbcfg_source_find2(*sources, &source_id);
                        if (source == NULL) {
                                if (!create_sources) {
                                        dvbcfg_source_id_free(&source_id);
                                        continue;
                                }

                                source = dvbcfg_source_new2(sources, &source_id, "???");
                                dvbcfg_source_id_free(&source_id);
                                if (source == NULL) {
                                        dvbcfg_adapter_free(adapters, newadapter);
                                        error = -ENOMEM;
                                        goto exit;
                                }

                        } else {
                                dvbcfg_source_id_free(&source_id);
                        }

                        /* add it in */
                        if (dvbcfg_adapter_add_source(newadapter, source)) {
                                dvbcfg_adapter_free(adapters, newadapter);
                                error = -ENOMEM;
                                goto exit;
                        }

                        /* next source_id please! */
                        linepos = dvbcfg_nexttoken(linepos);
                }
        }

exit:
        /* tidy up and return */
        fclose(in);
        return error;
}

int dvbcfg_adapter_save(const char *config_file,
			struct dvbcfg_adapter *adapters)
{
        FILE *out;
        char* source_id;
        int i;

        /* open the file */
        out = fopen(config_file, "w");
        if (out == NULL)
                return errno;

        while (adapters) {
                fprintf(out, "%s ", adapters->adapter_id);

                for(i=0; i< adapters->sources_count; i++) {
                        source_id = dvbcfg_source_id_to_string(&adapters->sources[i]->source_id);
                        if (source_id) {
                                fprintf(out, "%s ", source_id);
                                free(source_id);
                        }
                }
                fprintf(out, "\n");

                adapters = adapters->next;
        }

        fclose(out);
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

        for(i=0; i< adapter->sources_count; i++) {
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

        for(i=0; i< adapter->sources_count; i++) {
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
