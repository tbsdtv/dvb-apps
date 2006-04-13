/**
 * dvbcfg_adapter configuration file support.
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

#ifndef DVBCFG_ADAPTER_H
#define DVBCFG_ADAPTER_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <libdvbcfg/dvbcfg_source.h>

/**
 * dvbcfg_adapter describes each DVB adapter in a system, and indicates what
 * source_ids are receivable by each adapter.
 */

/**
 * In-memory representation of a single adapter.
 */
struct dvbcfg_adapter {
        char *adapter_id;

        struct dvbcfg_source** sources;
        int sources_count;

        struct dvbcfg_adapter *next;    /* NULL=> this is the last entry */
};

/**
 * Adapter backend API.
 */
struct dvbcfg_adapter_backend {
        /**
         * Loads a single adapter from the backend and add to the supplied list.
         *
         * @param backend Pointer to the backend structure concerned.
         * @param adapters Pointer to the list of adapters.
         * @return 0 on success, <0 on error, or 1 on end of file.
         */
        int (*get)(struct dvbcfg_adapter_backend* backend,
                   struct dvbcfg_adapter** adapters);

        /**
         * Stores a single adapter to the backend.
         *
         * @param backend Pointer to the backend structure concerned.
         * @param adapter Adapter to store.
         * @return 0 on success, <0 on error.
         */
        int (*put)(struct dvbcfg_adapter_backend* backend,
                   struct dvbcfg_adapter* adapter);
};

/**
 * Convenience method to load all adapters from a backend.
 *
 * @param backend Backend to use.
 * @param adapters Where to put the pointer to the start of the loaded adapters.
 * If NULL, a new list will be created, if it points to an already initialised
 * list, the loaded adapters will be appended to it.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_adapter_load(struct dvbcfg_adapter_backend *backend,
                               struct dvbcfg_adapter **adapters);

/**
 * Convenience method to store all adapters to a backend.
 *
 * @param backend Backend to use.
 * @param adapters Pointer to the list of adapters to save.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_adapter_save(struct dvbcfg_adapter_backend *backend,
                               struct dvbcfg_adapter *adapters);

/**
 * Create a new adapter
 *
 * @param adapters List of adapters to add to.
 * @param adapter_id The ID of the adapter.
 * @return The new dvbcfg_adapter structure, or NULL on error.
 */
extern struct dvbcfg_adapter* dvbcfg_adapter_new(struct dvbcfg_adapter** adapters, char* adapter_id);

/**
 * Add a new source to an adapter.
 *
 * @param adapter Adapter to add to.
 * @param source dvbcfg_source to add.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbcfg_adapter_add_source(struct dvbcfg_adapter* adapter, struct dvbcfg_source* source);

/**
 * Remove a source from an adapter.
 *
 * @param adapter Adapter to remove from.
 * @param source dvbcfg_source to remove.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbcfg_adapter_remove_source(struct dvbcfg_adapter* adapter, struct dvbcfg_source* source);

/**
 * Find the entry for a particular adapter_id.
 *
 * @param adapters Pointer to the list to search.
 * @param adapter_id adapter_id to find.
 * @return A dvbcfg_adapter structure if found, or NULL if not.
 */
extern struct dvbcfg_adapter *dvbcfg_adapter_find(struct dvbcfg_adapter* adapters,
                                                  char* adapter_id);

/**
 * Find an adapter supporting a dvbcfg_source.
 *
 * @param adapters Pointer to the list to search.
 * @param source dvbcfg_source to find.
 * @return A dvbcfg_adapter structure if found, or NULL if not.
 */
extern struct dvbcfg_adapter *dvbcfg_adapter_find_source(struct dvbcfg_adapter* adapters,
                                                         struct dvbcfg_source* source);

/**
 * Does the supplied adapter support the supplied dvbcfg_source?
 *
 * @param adapter Adapter to check
 * @param source dvbcfg_source to check for.
 * @return 1 if it does, 0 if not.
 */
extern int dvbcfg_adapter_supports_source(struct dvbcfg_adapter* adapter,
                                          struct dvbcfg_source* source);

/**
 * Unlink a single adapter from a list, and free its memory.
 *
 * @param adapters The list of adapters.
 * @param tofree The adapter to free.
 */
extern void dvbcfg_adapter_free(struct dvbcfg_adapter **adapters,
                                struct dvbcfg_adapter *tofree);

/**
 * Free memory for all adapters in a list.
 *
 * @param adapters Pointer to list of adapters to free.
 */
extern void dvbcfg_adapter_free_all(struct dvbcfg_adapter *adapters);

#ifdef __cplusplus
}
#endif

#endif
