/**
 * dvbcfg_source configuration file support.
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
#include <errno.h>
#include <string.h>
#include "dvbcfg_source_backend_file.h"
#include "dvbcfg_util.h"

#define DVBCFG_DEFAULT_SOURCE_FILENAME (DVBCFG_DEFAULT_DIR "/sources.conf")

struct dvbcfg_source_backend_file {
        struct dvbcfg_source_backend api;

        char* filename;
        FILE* inhandle;
        FILE* outhandle;
};

static int get_source(struct dvbcfg_source_backend* backend, struct dvbcfg_source** sources);
static int put_source(struct dvbcfg_source_backend* backend, struct dvbcfg_source* source);

int dvbcfg_source_backend_file_create(const char* filename,
                                      struct dvbcfg_source_backend** backend)
{
        struct dvbcfg_source_backend_file* fbackend;

        fbackend = malloc(sizeof(struct dvbcfg_source_backend_file));
        if (fbackend == NULL)
                return -ENOMEM;

        memset(fbackend, 0, sizeof(struct dvbcfg_source_backend_file));
        fbackend->api.get = get_source;
        fbackend->api.put = put_source;

        if (filename) {
                fbackend->filename = strdup(filename);
        } else {
                fbackend->filename = strdup(DVBCFG_DEFAULT_SOURCE_FILENAME);
        }
        if (fbackend->filename == NULL) {
                free(fbackend);
                return -ENOMEM;
        }

        *backend = (struct dvbcfg_source_backend*) fbackend;
        return 0;
}

void dvbcfg_source_backend_file_destroy(struct dvbcfg_source_backend* backend)
{
        struct dvbcfg_source_backend_file* fbackend =
                (struct dvbcfg_source_backend_file*) backend;

        if (fbackend->inhandle != NULL)
                fclose(fbackend->inhandle);
        if (fbackend->outhandle != NULL)
                fclose(fbackend->outhandle);
        if (fbackend->filename)
                free(fbackend->filename);
        free(backend);
}

static int get_source(struct dvbcfg_source_backend* backend,
                      struct dvbcfg_source** sources)
{
        struct dvbcfg_source_backend_file* fbackend =
                (struct dvbcfg_source_backend_file*) backend;
        char curline[256];
        char *linepos;
        char* source_id;
        int numtokens;

        /* open the file if necessary */
        if (fbackend->inhandle == NULL) {
                fbackend->inhandle = fopen(fbackend->filename, "r");
                if (fbackend->inhandle == NULL)
                        return -errno;
        }

        /* keep reading till we get one source */
        while (fgets(curline, sizeof(curline), fbackend->inhandle)) {
                linepos = curline;

                /* clean any comments/ whitespace */
                if (dvbcfg_cleanline(linepos) == 0)
                        continue;

                /* tokenise the line */
                numtokens = dvbcfg_tokenise(linepos, " \t", 1, 1);
                if (numtokens != 2) {
                        continue;
                }

                /* the source_id */
                if (strchr(linepos, ':'))
                        continue;
                source_id = linepos;
                linepos = dvbcfg_nexttoken(linepos);

                /* create the source */
                if (dvbcfg_source_new(sources, source_id, linepos) == NULL) {
                        return -ENOMEM;
                }

                /* source read successfully! */
                return 0;
        }

        /* end of file! */
        return 1;
}

static int put_source(struct dvbcfg_source_backend* backend,
                      struct dvbcfg_source* source)
{
        struct dvbcfg_source_backend_file* fbackend =
                (struct dvbcfg_source_backend_file*) backend;
        char* tmp;

        /* open the file if necessary */
        if (fbackend->outhandle == NULL) {
                fbackend->outhandle = fopen(fbackend->filename, "w");
                if (fbackend->outhandle == NULL)
                        return -errno;
        }

        /* render the source id */
        tmp = dvbcfg_source_id_to_string(&source->source_id);
        if (tmp == NULL)
                return -ENOMEM;

        /* output it! */
        fprintf(fbackend->outhandle, "%s", tmp);
        free(tmp);
        fprintf(fbackend->outhandle, " %s\n", source->description);

        return 0;
}
