/**
 * dvbcfg_source configuration file support.
 *
 * Copyright (c) 2005 by Andrew de Quincey <adq_dvb@lidskialf.net>
 *
 * Credits go to Klaus Schmidinger's VDR for coming up with this really
 * great idea.
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

#ifndef DVBCFG_SOURCE_H
#define DVBCFG_SOURCE_H 1

#include <cfg/dvbcfg_common.h>

/**
 * dvbcfg_source are standardised unique IDs defined for all DVB transmitters
 * (as there is no other real standard).
 */

/**
 * In-memory representation of a single source.
 */
struct dvbcfg_source {
        struct dvbcfg_source_id source_id;
        char *description;

        struct dvbcfg_source *next;     /* NULL=> this is the last entry */
};

/**
 * Source backend API.
 */
struct dvbcfg_source_backend {
        /**
         * Loads a single source from the backend and add to the supplied list.
         *
         * @param backend Pointer to the backend structure concerned.
         * @param sources Pointer to the list of sources.
         * @return 0 on success, <0 on error, or 1 on end of file.
         */
        int (*get)(struct dvbcfg_source_backend* backend,
                   struct dvbcfg_source** sources);

        /**
         * Stores a single source to the backend.
         *
         * @param backend Pointer to the backend structure concerned.
         * @param source Source to store.
         * @return 0 on success, <0 on error.
         */
        int (*put)(struct dvbcfg_source_backend* backend,
                   struct dvbcfg_source* source);
};

/**
 * Convenience method to load all sources from a backend.
 *
 * @param backend Backend to use.
 * @param sources Where to put the pointer to the start of the loaded sources.
 * If NULL, a new list will be created, if it points to an already initialised
 * list, the loaded sources will be appended to it.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_source_load(struct dvbcfg_source_backend *backend,
                              struct dvbcfg_source **sources);

/**
 * Convenience method to store all sources to a backend.
 *
 * @param backend Backend to use.
 * @param sources Pointer to the list of sources to save.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_source_save(struct dvbcfg_source_backend *backend,
                              struct dvbcfg_source *sources);

/**
 * Add a new source.
 *
 * @param sources Pointer to list of sources to add to (source is added to this
 * list on success).
 * @param source_id The externalised source_id string of the source to add.
 * @param description Description of the source.
 * @return Pointer to the new dvbcfg_source structure, or NULL on error.
 */
extern struct dvbcfg_source* dvbcfg_source_new(struct dvbcfg_source** sources,
                                               char* source_id, char* description);

/**
 * Add a new source using a source_id structure.
 *
 * @param sources Pointer to list of sources to add to (source is added to this
 * list on success).
 * @param source_id The dvbcfg_source_id structure of the source to add.
 * @param description Description of the source.
 * @return Pointer to the new dvbcfg_source structure, or NULL on error.
 */
extern struct dvbcfg_source* dvbcfg_source_new2(struct dvbcfg_source** sources,
                                                struct dvbcfg_source_id* source_id,
                                                char* description);

/**
 * Find the entry for a particular source_id.
 *
 * @param sources Pointer to the list to search.
 * @param source_type Type of source.
 * @param source_network source_network to find.
 * @param source_region source_region to find (pass NULL for DVBS, or to simply
 * match by source_network).
 * @param source_locale source_locale to find (pass NULL for DVBS, or to simply
 * match by source_network+source_region).
 * @return A dvbcfg_source structure if found, or NULL if not.
 */
extern struct dvbcfg_source *dvbcfg_source_find(struct dvbcfg_source *sources,
                                                char source_type,
                                                char *source_network,
                                                char* source_region,
                                                char* source_locale);

/**
 * Find the entry for a particular source_id using a dvbcfg_source_id structure.
 *
 * @param sources Pointer to the list to search.
 * @param source_type Type of source.
 * @param source_network source_network to find.
 * @param source_region source_region to find (pass NULL for DVBS, or to simply
 * match by source_network).
 * @param source_locale source_locale to find (pass NULL for DVBS, or to simply
 * match by source_network+source_region).
 * @return A dvbcfg_source structure if found, or NULL if not.
 */
extern struct dvbcfg_source *dvbcfg_source_find2(struct dvbcfg_source *sources,
                                                 struct dvbcfg_source_id* source_id);

/**
 * Unlink a single source from a list, and free its memory.
 *
 * @param sources The list of sources.
 * @param tofree The source to free.
 */
extern void dvbcfg_source_free(struct dvbcfg_source **sources,
                               struct dvbcfg_source *tofree);

/**
 * Free memory for all sources in a list.
 *
 * @param sources Pointer to list of sources to free.
 */
extern void dvbcfg_source_free_all(struct dvbcfg_source *sources);

#endif
