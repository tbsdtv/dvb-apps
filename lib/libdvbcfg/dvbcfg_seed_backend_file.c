/**
 * dvbcfg_seed configuration file support.
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
#include "dvbcfg_seed_backend_file.h"
#include "dvbcfg_source.h"
#include "dvbcfg_util.h"

#define DVBCFG_DEFAULT_SEED_FILENAME (DVBCFG_DEFAULT_DIR "/seeds.conf")

struct dvbcfg_seed_backend_file {
        struct dvbcfg_seed_backend api;

        char* filename;
        FILE* inhandle;
        FILE* outhandle;
        int long_delivery;
	struct dvbcfg_source** sources;
	int create_sources;
};

static int get_seed(struct dvbcfg_seed_backend* backend,
                    struct dvbcfg_seed** seeds);
static int put_seed(struct dvbcfg_seed_backend* backend,
                    struct dvbcfg_seed* seed);

int dvbcfg_seed_backend_file_create(const char* filename,
                                    int long_delivery,
				    struct dvbcfg_source** sources,
				    int create_sources,
                                    struct dvbcfg_seed_backend** backend)
{
        struct dvbcfg_seed_backend_file* fbackend;

        fbackend = malloc(sizeof(struct dvbcfg_seed_backend_file));
        if (fbackend == NULL)
                return -ENOMEM;

        memset(fbackend, 0, sizeof(struct dvbcfg_seed_backend_file));
        fbackend->api.get = get_seed;
        fbackend->api.put = put_seed;

	if (filename) {
		fbackend->filename = strdup(filename);
	} else {
		fbackend->filename = strdup(DVBCFG_DEFAULT_SEED_FILENAME);
	}
	if (fbackend->filename == NULL) {
		free(fbackend);
		return -ENOMEM;
	}
        fbackend->long_delivery = long_delivery;
	fbackend->sources = sources;
	fbackend->create_sources = create_sources;

        *backend = (struct dvbcfg_seed_backend*) fbackend;
        return 0;
}

void dvbcfg_seed_backend_file_destroy(struct dvbcfg_seed_backend* backend)
{
        struct dvbcfg_seed_backend_file* fbackend =
                (struct dvbcfg_seed_backend_file*) backend;

        if (fbackend->inhandle != NULL)
               fclose(fbackend->inhandle);
        if (fbackend->outhandle != NULL)
                fclose(fbackend->outhandle);
        if (fbackend->filename)
                free(fbackend->filename);
        free(backend);
}

static int get_seed(struct dvbcfg_seed_backend* backend,
                    struct dvbcfg_seed** seeds)
{
        struct dvbcfg_seed_backend_file* fbackend =
                (struct dvbcfg_seed_backend_file*) backend;
        char curline[256];
        char *linepos;
	struct dvbcfg_source_id source_id;
	int numtokens;
	struct dvbcfg_source *source;
        struct dvbcfg_delivery delivery;

        /* open the file if necessary */
        if (fbackend->inhandle == NULL) {
                fbackend->inhandle = fopen(fbackend->filename, "r");
                if (fbackend->inhandle == NULL)
                        return -errno;
        }

        /* keep reading till we get one seed */
        while (fgets(curline, sizeof(curline), fbackend->inhandle)) {
                linepos = curline;

                /* clean any comments/ whitespace */
                if (dvbcfg_cleanline(linepos) == 0)
                        continue;

		/* tokenise the line */
		numtokens = dvbcfg_tokenise(linepos, " \t", 1, 1);
		if (numtokens != 2)
			continue;

		/* the source id */
		if (dvbcfg_source_id_from_string(linepos, &source_id))
			return -ENOMEM;

		/* try to find the source */
		source = dvbcfg_source_find2(*(fbackend->sources), &source_id);
		if (source == NULL) {
			if (!fbackend->create_sources) {
				dvbcfg_source_id_free(&source_id);
				continue;
			}

			source = dvbcfg_source_new2(fbackend->sources, &source_id, "???");
			dvbcfg_source_id_free(&source_id);
			if (source == NULL)
				return -ENOMEM;
		} else {
			dvbcfg_source_id_free(&source_id);
		}

                /* parse the delivery */
		linepos = dvbcfg_nexttoken(linepos);
                if (dvbcfg_delivery_from_string(linepos, source->source_id.source_type, &delivery))
                        continue;

                /* add it in! */
                dvbcfg_seed_new(seeds, source, delivery);

                /* delivery read successfully! */
                return 0;
        }

        /* end of file! */
        return 1;
}

static int put_seed(struct dvbcfg_seed_backend* backend,
                    struct dvbcfg_seed* seed)
{
        char tmp[512];
        struct dvbcfg_seed_backend_file* fbackend =
                (struct dvbcfg_seed_backend_file*) backend;
	char *source_id;

        /* open the file if necessary */
        if (fbackend->outhandle == NULL) {
                fbackend->outhandle = fopen(fbackend->filename, "w");
                if (fbackend->outhandle == NULL)
                        return -errno;
        }

        if (dvbcfg_delivery_to_string(seed->source->source_id.source_type,
                                      fbackend->long_delivery,
                                      &seed->delivery, tmp, sizeof(tmp)))
                return -ENOMEM;
	source_id = dvbcfg_source_id_to_string(&seed->source->source_id);

	fprintf(fbackend->outhandle, "%s %s\n", source_id, tmp);

	free(source_id);
        return 0;
}
