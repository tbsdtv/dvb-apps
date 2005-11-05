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
#include "dvbcfg_adapter_backend_file.h"
#include "dvbcfg_source.h"
#include "dvbcfg_util.h"

#define DVBCFG_DEFAULT_ADAPTER_FILENAME (DVBCFG_DEFAULT_DIR "/adapters.conf")

struct dvbcfg_adapter_backend_file {
        struct dvbcfg_adapter_backend api;

        char* filename;
        FILE* inhandle;
        FILE* outhandle;
        struct dvbcfg_source** sources;
        int create_sources;
};

static int get_adapter(struct dvbcfg_adapter_backend* backend,
                       struct dvbcfg_adapter** adapters);
static int put_adapter(struct dvbcfg_adapter_backend* backend,
                       struct dvbcfg_adapter* adapter);

int dvbcfg_adapter_backend_file_create(const char* filename,
                                       struct dvbcfg_source** sources,
                                       int create_sources,
                                       struct dvbcfg_adapter_backend** backend)
{
        struct dvbcfg_adapter_backend_file* fbackend;

        fbackend = malloc(sizeof(struct dvbcfg_adapter_backend_file));
        if (fbackend == NULL)
                return -ENOMEM;

        memset(fbackend, 0, sizeof(struct dvbcfg_adapter_backend_file));
        fbackend->api.get = get_adapter;
        fbackend->api.put = put_adapter;

        if (filename) {
                fbackend->filename = strdup(filename);
        } else {
                fbackend->filename = strdup(DVBCFG_DEFAULT_ADAPTER_FILENAME);
        }
        if (fbackend->filename == NULL) {
                free(fbackend);
                return -ENOMEM;
        }
        fbackend->sources = sources;
        fbackend->create_sources = create_sources;

        *backend = (struct dvbcfg_adapter_backend*) fbackend;
        return 0;
}

void dvbcfg_adapter_backend_file_destroy(struct dvbcfg_adapter_backend* backend)
{
        struct dvbcfg_adapter_backend_file* fbackend =
                (struct dvbcfg_adapter_backend_file*) backend;

        if (fbackend->inhandle != NULL)
               fclose(fbackend->inhandle);
        if (fbackend->outhandle != NULL)
                fclose(fbackend->outhandle);
        if (fbackend->filename)
                free(fbackend->filename);
        free(backend);
}

static int get_adapter(struct dvbcfg_adapter_backend* backend,
                       struct dvbcfg_adapter** adapters)
{
        struct dvbcfg_adapter_backend_file* fbackend =
                (struct dvbcfg_adapter_backend_file*) backend;
        char curline[256];
        char *linepos;
        struct dvbcfg_adapter *newadapter;
        struct dvbcfg_source_id source_id;
        struct dvbcfg_source* source;
        int numtokens;
        int i;

        /* open the file if necessary */
        if (fbackend->inhandle == NULL) {
                fbackend->inhandle = fopen(fbackend->filename, "r");
                if (fbackend->inhandle == NULL)
                        return -errno;
        }

        /* keep reading till we get one adapter */
        while (fgets(curline, sizeof(curline), fbackend->inhandle)) {
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
                        return -ENOMEM;
                }
                linepos = dvbcfg_nexttoken(linepos);

                /* the source_ids */
                for (i = 1; i < numtokens; i++) {
                        if (dvbcfg_source_id_from_string(linepos, &source_id)) {
                                dvbcfg_adapter_free(adapters, newadapter);
                                return -ENOMEM;
                        }

                        /* try to find it */
                        source = dvbcfg_source_find2(*(fbackend->sources), &source_id);
                        if (source == NULL) {
                                if (!fbackend->create_sources) {
                                        dvbcfg_source_id_free(&source_id);
                                        continue;
                                }

                                source = dvbcfg_source_new2(fbackend->sources,
                                                            &source_id, "???");
                                dvbcfg_source_id_free(&source_id);
                                if (source == NULL) {
                                        dvbcfg_adapter_free(adapters, newadapter);
                                        return -ENOMEM;
                                }

                        } else {
                                dvbcfg_source_id_free(&source_id);
                        }

                        /* add it in */
                        if (dvbcfg_adapter_add_source(newadapter, source)) {
                                dvbcfg_adapter_free(adapters, newadapter);
                                return -ENOMEM;
                        }

                        /* next source_id please! */
                        linepos = dvbcfg_nexttoken(linepos);
                }

                /* adapter read successfully! */
                return 0;
        }

        /* end of file! */
        return 1;
}

static int put_adapter(struct dvbcfg_adapter_backend* backend,
                       struct dvbcfg_adapter* adapter)
{
        struct dvbcfg_adapter_backend_file* fbackend =
                (struct dvbcfg_adapter_backend_file*) backend;
        char *source_id;
        int i;

        /* open the file if necessary */
        if (fbackend->outhandle == NULL) {
                fbackend->outhandle = fopen(fbackend->filename, "w");
                if (fbackend->outhandle == NULL)
                        return -errno;
        }

        fprintf(fbackend->outhandle, "%s ", adapter->adapter_id);

        for (i=0; i< adapter->sources_count; i++) {
                source_id = dvbcfg_source_id_to_string(&adapter->sources[i]->source_id);
                if (source_id) {
                        fprintf(fbackend->outhandle, "%s ", source_id);
                        free(source_id);
                }
        }
        fprintf(fbackend->outhandle, "\n");

        return 0;
}
