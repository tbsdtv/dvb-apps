/**
 * dvbcfg_diseqc configuration file support.
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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "dvbcfg_diseqc_backend_file.h"
#include "dvbcfg_common.h"
#include "dvbcfg_util.h"

#define DVBCFG_DEFAULT_DISEQC_FILENAME (DVBCFG_DEFAULT_DIR "/diseqcs.conf")

struct dvbcfg_diseqc_backend_file {
        struct dvbcfg_diseqc_backend api;

        char* filename;
        FILE* inhandle;
        FILE* outhandle;
        struct dvbcfg_source** sources;
        int create_sources;
};

static int get_diseqc(struct dvbcfg_diseqc_backend* backend,
                      struct dvbcfg_diseqc** diseqcs);
static int put_diseqc(struct dvbcfg_diseqc_backend* backend,
                      struct dvbcfg_diseqc* diseqc);

int dvbcfg_diseqc_backend_file_create(const char* filename,
                                      struct dvbcfg_source** sources,
                                      int create_sources,
                                      struct dvbcfg_diseqc_backend** backend)
{
        struct dvbcfg_diseqc_backend_file* fbackend;

        fbackend = malloc(sizeof(struct dvbcfg_diseqc_backend_file));
        if (fbackend == NULL)
                return -ENOMEM;

        memset(fbackend, 0, sizeof(struct dvbcfg_diseqc_backend_file));
        fbackend->api.get = get_diseqc;
        fbackend->api.put = put_diseqc;

        if (filename) {
                fbackend->filename = strdup(filename);
        } else {
                fbackend->filename = strdup(DVBCFG_DEFAULT_DISEQC_FILENAME);
        }
        if (fbackend->filename == NULL) {
                free(fbackend);
                return -ENOMEM;
        }
        fbackend->sources = sources;
        fbackend->create_sources = create_sources;

        *backend = (struct dvbcfg_diseqc_backend*) fbackend;
        return 0;
}

void dvbcfg_diseqc_backend_file_destroy(struct dvbcfg_diseqc_backend* backend)
{
        struct dvbcfg_diseqc_backend_file* fbackend =
                (struct dvbcfg_diseqc_backend_file*) backend;

        if (fbackend->inhandle != NULL)
                fclose(fbackend->inhandle);
        if (fbackend->outhandle != NULL)
                fclose(fbackend->outhandle);
        if (fbackend->filename)
                free(fbackend->filename);
        free(backend);
}

static int get_diseqc(struct dvbcfg_diseqc_backend* backend,
                       struct dvbcfg_diseqc** diseqcs)
{
        struct dvbcfg_diseqc_backend_file* fbackend =
            (struct dvbcfg_diseqc_backend_file*) backend;
        char curline[256];
        char *linepos;
        struct dvbcfg_source_id source_id;
        uint32_t slof;
        uint8_t polarization;
        uint32_t lof;
        char *command;
        struct dvbcfg_source *source;
        struct dvbcfg_diseqc *curdiseqc;
        int numtokens;
        int val;

        /* open the file if necessary */
        if (fbackend->inhandle == NULL) {
                fbackend->inhandle = fopen(fbackend->filename, "r");
                if (fbackend->inhandle == NULL)
                        return -errno;
        }

        while (fgets(curline, sizeof(curline), fbackend->inhandle)) {
                linepos = curline;

                /* clean any comments/ whitespace */
                if (dvbcfg_cleanline(linepos) == 0)
                        continue;

                /* tokenise the line */
                numtokens = dvbcfg_tokenise(linepos, " \t", 4, 1);
                if ((numtokens < 4) || (numtokens > 5))
                       continue;

		/* check for wildcard */
		if (strcmp(linepos, "*")) {
			source = NULL;
		} else {
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
		}

                /* find/create the diseqc */
                curdiseqc = dvbcfg_diseqc_find(*diseqcs, source);
                if (curdiseqc == NULL) {
                        /* create the diseqc */
                        curdiseqc = dvbcfg_diseqc_new(diseqcs, source);
                        if (curdiseqc == NULL)
                                return -ENOMEM;
                }
                linepos = dvbcfg_nexttoken(linepos);

                /* the SLOF */
                if (sscanf(linepos, "%d", &val) != 1)
                        continue;
		slof = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* the polarization */
                if (linepos[0] == 'H')
                        polarization = DVBFE_POLARIZATION_H;
                else if (linepos[0] == 'V')
                        polarization = DVBFE_POLARIZATION_V;
                else if (linepos[0] == 'L')
                        polarization = DVBFE_POLARIZATION_L;
                else if (linepos[0] == 'R')
                        polarization = DVBFE_POLARIZATION_R;
                else
                        continue;
                linepos = dvbcfg_nexttoken(linepos);

                /* LOF */
                if (sscanf(linepos, "%d", &val) != 1)
                        continue;
		lof = val;
                linepos = dvbcfg_nexttoken(linepos);

                /* command */
                if (numtokens == 5)
                        command = linepos;
                else
                        command = "";

                /* create a new entry */
                if (dvbcfg_diseqc_add_entry(curdiseqc, slof, polarization, lof, command) == NULL)
                        return -ENOMEM;

                /* loaded successfully */
                return 0;
        }

        /* end of file */
        return 1;
}

static int put_diseqc(struct dvbcfg_diseqc_backend* backend,
                      struct dvbcfg_diseqc* diseqc)
{
        struct dvbcfg_diseqc_backend_file* fbackend =
                (struct dvbcfg_diseqc_backend_file*) backend;
        char polarization = 'H';
        struct dvbcfg_diseqc_entry *entry;
        char* source_id;

        /* open the file if necessary */
        if (fbackend->outhandle == NULL) {
                fbackend->outhandle = fopen(fbackend->filename, "w");
                if (fbackend->outhandle == NULL)
                        return -errno;
        }

	if (diseqc->source == NULL) {
		source_id = "*";
	} else {
		source_id = dvbcfg_source_id_to_string(&diseqc->source->source_id);
		if (source_id == NULL)
			return -ENOMEM;
	}

        entry = diseqc->entries;
        while (entry) {
                switch (entry->polarization) {
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

                fprintf(fbackend->outhandle, "%s %d %c %d %s\n",
                        source_id, entry->slof,
                        polarization, entry->lof,
                        entry->command);

                entry = entry->next;
        }

        free(source_id);

        return 0;
}
