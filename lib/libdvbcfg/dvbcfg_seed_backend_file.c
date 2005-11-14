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

#define DVBCFG_DEFAULT_SEED_DIRECTORY (DVBCFG_DEFAULT_DIR "/seed/")

struct dvbcfg_seed_backend_file {
        struct dvbcfg_seed_backend api;

        char* filename;
        FILE* inhandle;
        FILE* outhandle;
        int long_delivery;

	dvbfe_type_t source_type;
};

static int get_delivery(struct dvbcfg_seed_backend* backend,
                        struct dvbcfg_seed* seed);
static int put_delivery(struct dvbcfg_seed_backend* backend,
                        struct dvbcfg_delivery* delivery);

int dvbcfg_seed_backend_file_create(const char* basename,
                                    const char* filename,
                                    int long_delivery,
				    dvbfe_type_t source_type,
                                    struct dvbcfg_seed_backend** backend)
{
        struct dvbcfg_seed_backend_file* fbackend;
        char tmp[512];

        fbackend = malloc(sizeof(struct dvbcfg_seed_backend_file));
        if (fbackend == NULL)
                return -ENOMEM;

        memset(fbackend, 0, sizeof(struct dvbcfg_seed_backend_file));
        fbackend->api.get = get_delivery;
        fbackend->api.put = put_delivery;

        if (basename == NULL)
                basename = DVBCFG_DEFAULT_SEED_DIRECTORY;

        if (snprintf(tmp, sizeof(tmp), "%s/%s", basename, filename) >= sizeof(tmp))
                return -ENOMEM;

        fbackend->filename = strdup(tmp);
        if (fbackend->filename == NULL) {
                free(fbackend);
                return -ENOMEM;
        }
        fbackend->long_delivery = long_delivery;
        fbackend->source_type = source_type;

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

static int get_delivery(struct dvbcfg_seed_backend* backend,
                        struct dvbcfg_seed* seed)
{
        struct dvbcfg_seed_backend_file* fbackend =
                (struct dvbcfg_seed_backend_file*) backend;
        char curline[256];
        char *linepos;
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

                /* parse the delivery */
                if (dvbcfg_delivery_from_string(linepos, fbackend->source_type, &delivery))
                        continue;

                /* add it in! */
                dvbcfg_seed_add_delivery(seed, delivery);

                /* delivery read successfully! */
                return 0;
        }

        /* end of file! */
        return 1;
}

static int put_delivery(struct dvbcfg_seed_backend* backend,
                        struct dvbcfg_delivery* delivery)
{
        char tmp[512];
        struct dvbcfg_seed_backend_file* fbackend =
                (struct dvbcfg_seed_backend_file*) backend;

        /* open the file if necessary */
        if (fbackend->outhandle == NULL) {
                fbackend->outhandle = fopen(fbackend->filename, "w");
                if (fbackend->outhandle == NULL)
                        return -errno;
        }

        if (dvbcfg_delivery_to_string(fbackend->source_type,
                                      fbackend->long_delivery,
                                      delivery, tmp, sizeof(tmp)))
                return -ENOMEM;

        fprintf(fbackend->outhandle, "%s\n", tmp);

        return 0;
}
