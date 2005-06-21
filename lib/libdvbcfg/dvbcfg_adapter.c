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
#include <errno.h>
#include "dvbcfg_adapter.h"
#include "dvbcfg_util.h"

static void freeentries(struct dvbcfg_adapter *adapter);


int dvbcfg_adapter_load(char *config_file,
                        struct dvbcfg_adapter **adapters)
{
        FILE *in;
        char curline[256];
        char *linepos;
        struct dvbcfg_adapter tmpadapter;
        struct dvbcfg_adapter *curadapter;
        struct dvbcfg_adapter *newadapter;
        struct dvbcfg_adapter_entry *newentry;
        struct dvbcfg_adapter_entry *curentry;
        int numtokens;
        int error = 0;
        int i;

        /* open the file */
        in = fopen(config_file, "r");
        if (in == NULL)
                return errno;

        /* move to the tail entry */
        curadapter = *adapters;
        if (curadapter)
                while (curadapter->next)
                        curadapter = curadapter->next;

        while (fgets(curline, sizeof(curline), in)) {
                linepos = curline;
                memset(&tmpadapter, 0, sizeof(struct dvbcfg_adapter));

                /* clean any comments/ whitespace */
                if (dvbcfg_cleanline(linepos) == 0)
                        continue;

                /* tokenise the line */
                numtokens = dvbcfg_tokenise(linepos, " \t", -1, 1);
                if (numtokens < 1) {
                        continue;
                }

                /* the adapter_id */
                tmpadapter.adapter_id = linepos;
                linepos = dvbcfg_nexttoken(linepos);

                /* the source_ids */
                for (i = 1; i < numtokens; i++) {
                        newentry = (struct dvbcfg_adapter_entry *)
                            malloc(sizeof(struct dvbcfg_adapter_entry));
                        if (newentry == NULL) {
                                error = -ENOMEM;
                                break;
                        }
                        newentry->source_id =
                            dvbcfg_strdupandtrim(linepos, -1);

                        /* hook it into the list */
                        if (!tmpadapter.source_ids) {
                                tmpadapter.source_ids = newentry;
                        } else {
                                curentry->next = newentry;
                        }
                        curentry = newentry;

                        /* next source_id please! */
                        linepos = dvbcfg_nexttoken(linepos);
                }

                /* create new entry */
                newadapter = (struct dvbcfg_adapter *)
                    malloc(sizeof(struct dvbcfg_adapter));
                if (newadapter == NULL) {
                        error = -ENOMEM;
                        break;
                }
                memcpy(newadapter, &tmpadapter,
                       sizeof(struct dvbcfg_adapter));
                newadapter->adapter_id =
                    dvbcfg_strdupandtrim(tmpadapter.adapter_id, -1);
                newadapter->source_ids = tmpadapter.source_ids;
                if (!newadapter->adapter_id) {
                        if (newadapter->adapter_id)
                                free(newadapter->adapter_id);
                        freeentries(newadapter);
                        free(newadapter);
                        error = -ENOMEM;
                        break;
                }

                /* add it into the list */
                if (curadapter) {
                        curadapter->next = newadapter;
                        newadapter->prev = curadapter;
                }
                if (!*adapters)
                        *adapters = newadapter;
                curadapter = newadapter;
        }

        /* tidy up and return */
        if (error) {
                dvbcfg_adapter_free_all(*adapters);
                *adapters = NULL;
        }
        fclose(in);
        return error;
}

int dvbcfg_adapter_save(char *config_file, struct dvbcfg_adapter *adapters)
{
        FILE *out;
        struct dvbcfg_adapter_entry *entry;

        /* open the file */
        out = fopen(config_file, "w");
        if (out == NULL)
                return errno;

        while (adapters) {
                fprintf(out, "%s ", adapters->adapter_id);

                entry = adapters->source_ids;
                while (entry) {
                        fprintf(out, "%s ", entry->source_id);
                        entry = entry->next;
                }
                fprintf(out, "\n");

                adapters = adapters->next;
        }

        fclose(out);
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

int dvbcfg_adapter_supports_source_id(struct dvbcfg_adapter *adapter,
                                      char *source_id)
{
        struct dvbcfg_adapter_entry *entry;

        entry = adapter->source_ids;
        while (entry) {
                if (!strcmp(entry->source_id, source_id))
                        return 1;

                entry = entry->next;
        }

        return 0;
}

struct dvbcfg_adapter *dvbcfg_adapter_find_source_id(struct dvbcfg_adapter
                                                     *adapters,
                                                     char *source_id)
{
        while (adapters) {
                if (dvbcfg_adapter_supports_source_id(adapters, source_id))
                        return adapters;

                adapters = adapters->next;
        }

        return NULL;
}


void dvbcfg_adapter_free(struct dvbcfg_adapter **adapters,
                         struct dvbcfg_adapter *tofree)
{
        struct dvbcfg_adapter *prev;
        struct dvbcfg_adapter *next;

        prev = tofree->prev;
        next = tofree->next;

        /* free internal structures */
        if (tofree->adapter_id)
                free(tofree->adapter_id);
        freeentries(tofree);
        free(tofree);

        /* adjust pointers */
        if (prev == NULL)
                *adapters = next;
        else
                prev->next = next;

        if (next != NULL)
                next->prev = prev;
}

void dvbcfg_adapter_free_all(struct dvbcfg_adapter *adapters)
{
        while (adapters)
                dvbcfg_adapter_free(&adapters, adapters);
}

static void freeentries(struct dvbcfg_adapter *adapter)
{
        struct dvbcfg_adapter_entry *entry;
        struct dvbcfg_adapter_entry *next_entry;

        entry = adapter->source_ids;
        while (entry) {
                next_entry = entry->next;
                free(entry->source_id);
                free(entry);
                entry = next_entry;
        }
}
