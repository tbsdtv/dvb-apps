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
#include "dvbcfg_seed.h"
#include "dvbcfg_util.h"


int dvbcfg_seed_load(struct dvbcfg_seed_backend *backend,
                     struct dvbcfg_seed **seeds)
{
        int status;
        while(!(status = backend->get(backend, seeds)));

        if (status < 0)
                return status;

        return 0;
}

int dvbcfg_seed_save(struct dvbcfg_seed_backend *backend,
                     struct dvbcfg_seed *seeds)
{
        int status;

	while(seeds) {
		if ((status = backend->put(backend, seeds)) != 0)
			return status;

		seeds = seeds->next;
	}

        return 0;
}

struct dvbcfg_seed *dvbcfg_seed_new(struct dvbcfg_seed **seeds, struct dvbcfg_source *source, struct dvbcfg_delivery delivery)
{
	struct dvbcfg_seed* newseed;
	struct dvbcfg_seed* curseed;

	/* create new structure */
	newseed = (struct dvbcfg_seed*) malloc(sizeof(struct dvbcfg_seed));
	if (newseed == NULL)
		return NULL;
	memset(newseed, 0, sizeof(struct dvbcfg_seed));
	newseed->source = source;
	newseed->delivery = delivery;

	/* add it to the list */
	if (*seeds == NULL)
		*seeds = newseed;
	else {
		curseed = *seeds;
		while(curseed->next)
			curseed = curseed->next;
		curseed->next = newseed;
	}

	return newseed;
}

void dvbcfg_seed_free(struct dvbcfg_seed **seeds, struct dvbcfg_seed *tofree)
{
	struct dvbcfg_seed *next;
	struct dvbcfg_seed *cur;

	next = tofree->next;

	/* free internal structures */
	free(tofree);

	/* adjust pointers */
	if (*seeds == tofree)
		*seeds = next;
	else {
		cur = *seeds;
		while((cur->next != tofree) && (cur->next))
			cur = cur->next;
		if (cur->next == tofree)
			cur->next = next;
	}
}

void dvbcfg_seed_free_all(struct dvbcfg_seed *seeds)
{
	while(seeds)
		dvbcfg_seed_free(&seeds, seeds);
}
