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

		/* the source code */
		tmpsource.source_id = linepos;
		linepos = dvbcfg_nexttoken(linepos);

		/* the description */
		tmpsource.description = linepos;

		/* create new entry */
		newsource =
		    (struct dvbcfg_source *)
		    malloc(sizeof(struct dvbcfg_source));
		if (newsource == NULL) {
			error = -ENOMEM;
			break;
		}
		memcpy(newsource, &tmpsource,
		       sizeof(struct dvbcfg_source));
		newsource->source_id =
		    dvbcfg_strdupandtrim(tmpsource.source_id);
		newsource->description =
		    dvbcfg_strdupandtrim(tmpsource.description);
		if ((!newsource->source_id) || (!newsource->description)) {
			if (newsource->source_id)
				free(newsource->source_id);
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

	/* open the file */
	out = fopen(config_file, "w");
	if (out == NULL)
		return errno;

	while (sources) {
		fprintf(out, "%s %s\n", sources->source_id,
			sources->description);

		sources = sources->next;
	}

	fclose(out);
	return 0;
}

struct dvbcfg_source *dvbcfg_source_find(struct dvbcfg_source *sources,
					 char *source_id)
{
	while (sources) {
		if (!strcmp(source_id, sources->source_id))
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
	if (tofree->source_id)
		free(tofree->source_id);
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
