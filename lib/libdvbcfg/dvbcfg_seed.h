/**
 * dvbcfg_seed configuration file support.
 *
 * Copyright (c) 2005 by Andrew de Quincey <adq_dvb@lidskialf.net>
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

#ifndef DVBCFG_SEED_H
#define DVBCFG_SEED_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <dvbcfg/dvbcfg_common.h>

/**
 * dvbcfg_seed describes a list of known deliveries which are used to scan and
 * generate a list of multiplexes/services.
 */

/**
 * In-memory representation of a seed.
 */
struct dvbcfg_seed {
        struct dvbcfg_delivery* deliveries;
        int deliveries_count;
};

/**
 * seed backend API.
 */
struct dvbcfg_seed_backend {
        /**
         * Loads a delivery from the backend.
         *
         * @param backend Pointer to the backend structure concerned.
         * @param seeds The seed structure to add deliveries to.
         * @return 0 on success, <0 on error, or 1 on end of file.
         */
        int (*get)(struct dvbcfg_seed_backend* backend,
                   struct dvbcfg_seed* seed);

        /**
         * Stores a single seed to the backend.
         *
         * @param backend Pointer to the backend structure concerned.
         * @param delivery The devlivery to store.
         * @return 0 on success, <0 on error.
         */
        int (*put)(struct dvbcfg_seed_backend* backend,
                   struct dvbcfg_delivery* delivery);
};

/**
 * Initialise a dvbcfg_seed structure
 */
static inline void dvbcfg_seed_init(struct dvbcfg_seed* seed)
{
        memset(seed, 0, sizeof(struct dvbcfg_seed));
}

/**
 * Convenience method to load a seed from a backend.
 *
 * @param backend Backend to use.
 * @param seed The seed to add deliveries into (caller allocated and initialised).
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_seed_load(struct dvbcfg_seed_backend *backend,
                            struct dvbcfg_seed *seed);

/**
 * Convenience method to store a seeds to a backend.
 *
 * @param backend Backend to use.
 * @param seeds The seed to save.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_seed_save(struct dvbcfg_seed_backend *backend,
                            struct dvbcfg_seed *seed);

/**
 * Add a new delivery to a seed.
 *
 * @param seed seed to add to.
 * @param delivery dvbcfg_delivery to add.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbcfg_seed_add_delivery(struct dvbcfg_seed* seed, struct dvbcfg_delivery delivery);

/**
 * Remove a delivery from an seed.
 *
 * @param seed seed to remove from.
 * @param idx Index of the delivery to remove.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbcfg_seed_remove_delivery(struct dvbcfg_seed* seed, int idx);

/**
 * Free contents of a seed (note: does not free the seed itself).
 *
 * @param seed The seed whose contents are to be freed.
 */
extern void dvbcfg_seed_clear(struct dvbcfg_seed *tofree);

#ifdef __cplusplus
}
#endif

#endif
