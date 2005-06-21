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
#include <errno.h>
#include "dvbcfg_source.h"
#include "dvbcfg_util.h"


int dvbcfg_source_load(char *config_file, struct dvbcfg_source **sources)
{
        FILE *in;
        char curline[256];
        char *linepos;
        struct dvbcfg_source tmpsource;
        struct dvbcfg_source *cursource;
        struct dvbcfg_source *newsource;
        int numtokens;
        int error = 0;

        /* open the file */
        in = fopen(config_file, "r");
        if (in == NULL)
                return errno;

        /* move to the tail entry */
        cursource = *sources;
        if (cursource)
                while (cursource->next)
                        cursource = cursource->next;

        while (fgets(curline, sizeof(curline), in)) {
                linepos = curline;
                memset(&tmpsource, 0, sizeof(struct dvbcfg_source));

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
                        return;
                if (dvbcfg_source_id_from_string(linepos, &tmpsource.source_id))
                        continue;
                linepos = dvbcfg_nexttoken(linepos);

                /* the description */
                tmpsource.description = linepos;

                /* create new entry */
                newsource = (struct dvbcfg_source *)
                    malloc(sizeof(struct dvbcfg_source));
                if (newsource == NULL) {
                        error = -ENOMEM;
                        break;
                }
                memcpy(newsource, &tmpsource, sizeof(struct dvbcfg_source));
                newsource->description = dvbcfg_strdupandtrim(tmpsource.description, -1);

                if (!newsource->description) {
                        dvbcfg_source_id_free(&tmpsource.source_id);
                        if (newsource->description)
                                free(newsource->description);
                        free(newsource);
                        error = -ENOMEM;
                        break;
                }

                /* add it into the list */
                if (cursource) {
                        cursource->next = newsource;
                        newsource->prev = cursource;
                }
                if (!*sources)
                        *sources = newsource;
                cursource = newsource;
        }

        /* tidy up and return */
        if (error) {
                dvbcfg_source_free_all(*sources);
                *sources = NULL;
        }
        fclose(in);
        return error;
}

int dvbcfg_source_save(char *config_file, struct dvbcfg_source *sources)
{
        FILE *out;
        char* tmp;

        /* open the file */
        out = fopen(config_file, "w");
        if (out == NULL)
                return errno;

        while (sources) {
                tmp = dvbcfg_source_id_to_string(&sources->source_id);
                if (tmp == NULL)
                        break;

                fprintf(out, "%s", tmp);
                free(tmp);
                fprintf(out, " %s\n", sources->description);

                sources = sources->next;
        }

        fclose(out);
        return 0;
}

struct dvbcfg_source *dvbcfg_source_find(struct dvbcfg_source *sources,
                                         char source_type, char *source_network, char* source_region, char* source_locale)
{
        struct dvbcfg_source_id source_id;

        source_id.source_type = source_type;
        source_id.source_network = source_network;
        source_id.source_region = source_region;
        source_id.source_locale = source_locale;


        while (sources) {
                if (dvbcfg_source_id_equal(&source_id, &sources->source_id, 1))
                        return sources;

                sources = sources->next;
        }

        return NULL;
}

void dvbcfg_source_free(struct dvbcfg_source **sources,
                        struct dvbcfg_source *tofree)
{
        struct dvbcfg_source *prev;
        struct dvbcfg_source *next;

        prev = tofree->prev;
        next = tofree->next;

        /* free internal structures */
        dvbcfg_source_id_free(&tofree->source_id);
        if (tofree->description)
                free(tofree->description);
        free(tofree);

        /* adjust pointers */
        if (prev == NULL)
                *sources = next;
        else
                prev->next = next;

        if (next != NULL)
                next->prev = prev;
}

void dvbcfg_source_free_all(struct dvbcfg_source *sources)
{
        while (sources)
                dvbcfg_source_free(&sources, sources);
}
