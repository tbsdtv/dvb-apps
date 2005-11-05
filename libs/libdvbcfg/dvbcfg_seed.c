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
                     struct dvbcfg_seed *seed)
{
        int status;
        while(!(status = backend->get(backend, seed)));

        if (status < 0)
                return status;

        return 0;
}

int dvbcfg_seed_save(struct dvbcfg_seed_backend *backend,
                     struct dvbcfg_seed *seed)
{
        int status;
        int i;

        for (i=0; i< seed->deliveries_count; i++) {
                if ((status = backend->put(backend, &seed->deliveries[i])) != 0)
                        return status;
        }

        return 0;
}

int dvbcfg_seed_add_delivery(struct dvbcfg_seed* seed, struct dvbcfg_delivery delivery)
{
        struct dvbcfg_delivery* tmp = seed->deliveries;

        if (seed->deliveries == NULL) {
                seed->deliveries = (struct dvbcfg_delivery*) malloc(sizeof(struct dvbcfg_delivery));
                if (seed->deliveries == NULL)
                        return -ENOMEM;
                memcpy(seed->deliveries, &delivery, sizeof(struct dvbcfg_delivery));
                seed->deliveries_count = 1;
        } else {
                seed->deliveries = (struct dvbcfg_delivery*)
                    realloc(seed->deliveries, sizeof(struct dvbcfg_delivery) * (seed->deliveries_count+1));
                if (seed->deliveries == NULL) {
                        seed->deliveries = tmp;
                        return -ENOMEM;
                }
                memcpy(&seed->deliveries[seed->deliveries_count++], &delivery, sizeof(struct dvbcfg_delivery));
        }

        return 0;
}

int dvbcfg_seed_remove_delivery(struct dvbcfg_seed* seed, int idx)
{
        struct dvbcfg_delivery* tmp;

        if (seed->deliveries == NULL)
                return -EINVAL;

        tmp = (struct dvbcfg_delivery*) malloc(sizeof(struct dvbcfg_delivery) * (seed->deliveries_count-1));
        if (tmp == NULL)
                return -ENOMEM;
        memcpy(tmp, seed->deliveries, sizeof(struct dvbcfg_delivery) * idx);
        memcpy(tmp + (sizeof(struct dvbcfg_delivery) * idx),
               seed->deliveries + (sizeof(struct dvbcfg_delivery) * (idx + 1)),
               sizeof(struct dvbcfg_delivery) * (seed->deliveries_count - idx - 1));

        free(seed->deliveries);
        seed->deliveries = tmp;
        seed->deliveries_count--;

        return 0;
}

void dvbcfg_seed_clear(struct dvbcfg_seed *tofree)
{
        /* free internal structures */
        if (tofree->deliveries)
                free(tofree->deliveries);
        memset(tofree, 0, sizeof(struct dvbcfg_seed));
}
