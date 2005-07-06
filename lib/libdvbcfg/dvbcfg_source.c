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
#include "dvbcfg_source.h"
#include "dvbcfg_util.h"


int dvbcfg_source_load(char *config_file, struct dvbcfg_source **sources)
{
        FILE *in;
        char curline[256];
        char *linepos;
        char* source_id;
        int numtokens;
        int error = 0;

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
                        error = -ENOMEM;
                        goto exit;
                }
        }

exit:
        /* tidy up and return */
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

struct dvbcfg_source* dvbcfg_source_new(struct dvbcfg_source **sources, char* source_idstr, char* description)
{
        struct dvbcfg_source_id source_id;
        struct dvbcfg_source* source;

        if (dvbcfg_source_id_from_string(source_idstr, &source_id)) {
                return NULL;
        }

        source = dvbcfg_source_new2(sources, &source_id, description);

        dvbcfg_source_id_free(&source_id);
        return source;
}

struct dvbcfg_source* dvbcfg_source_new2(struct dvbcfg_source **sources, struct dvbcfg_source_id* source_id, char* description)
{
        struct dvbcfg_source* newsource;
        struct dvbcfg_source* cursource;

        /* create new structure */
        newsource = (struct dvbcfg_source*) malloc(sizeof(struct dvbcfg_source));
        if (newsource == NULL)
                return NULL;
        memset(newsource, 0, sizeof(struct dvbcfg_source));
        newsource->description = dvbcfg_strdupandtrim(description, -1);
        if (newsource->description == NULL) {
                free(newsource);
                return NULL;
        }

        /* parse the source_id */
        newsource->source_id.source_type = source_id->source_type;
        if (source_id->source_network)
                newsource->source_id.source_network = dvbcfg_strdupandtrim(source_id->source_network, -1);
        if (source_id->source_region)
                newsource->source_id.source_region = dvbcfg_strdupandtrim(source_id->source_region, -1);
        if (source_id->source_locale)
                newsource->source_id.source_locale = dvbcfg_strdupandtrim(source_id->source_locale, -1);

        /* check */
        if (((newsource->source_id.source_network == NULL) != (source_id->source_network == NULL)) ||
            ((newsource->source_id.source_region == NULL) != (source_id->source_region == NULL)) ||
            ((newsource->source_id.source_locale == NULL) != (source_id->source_locale == NULL))) {
                dvbcfg_source_id_free(&newsource->source_id);
                free(newsource->description);
                free(newsource);
                return NULL;
        }

        /* add it to the list */
        if (*sources == NULL)
                *sources = newsource;
        else {
                cursource = *sources;
                while(cursource->next)
                        cursource = cursource->next;
                cursource->next = newsource;
        }

        return newsource;
}

struct dvbcfg_source *dvbcfg_source_find(struct dvbcfg_source *sources,
                                         char source_type, char *source_network, char* source_region, char* source_locale)
{
        struct dvbcfg_source_id source_id;

        source_id.source_type = source_type;
        source_id.source_network = source_network;
        source_id.source_region = source_region;
        source_id.source_locale = source_locale;

        return dvbcfg_source_find2(sources, &source_id);
}

struct dvbcfg_source *dvbcfg_source_find2(struct dvbcfg_source *sources, struct dvbcfg_source_id* source_id)
{
        while (sources) {
                if (dvbcfg_source_id_equal(source_id, &sources->source_id, 1))
                        return sources;

                sources = sources->next;
        }

        return NULL;
}


void dvbcfg_source_free(struct dvbcfg_source **sources,
                        struct dvbcfg_source *tofree)
{
        struct dvbcfg_source *next;
        struct dvbcfg_source *cur;

        next = tofree->next;

        /* free internal structures */
        dvbcfg_source_id_free(&tofree->source_id);
        if (tofree->description)
                free(tofree->description);
        free(tofree);

        /* adjust pointers */
        if (*sources == tofree)
                *sources = next;
        else {
                cur = *sources;
                while((cur->next != tofree) && (cur->next))
                        cur = cur->next;
                if (cur->next == tofree)
                        cur->next = next;
        }
}

void dvbcfg_source_free_all(struct dvbcfg_source *sources)
{
        while (sources)
                dvbcfg_source_free(&sources, sources);
}
