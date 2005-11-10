/**
 * dvbcfg_diseqc configuration file support.
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

#ifndef DVBCFG_DISEQC_H
#define DVBCFG_DISEQC_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <dvbcfg/dvbcfg_source.h>

/**
 * dvbcfg_diseqc defines DISEQC command sequences to use for DVBS channels.
 * A channel is matched to a command sequence by matching its source_id,
 * polarization, and if its frequency is less than the SLOF for the command
 * (if more than one command is a candidate because the frequency is less than
 * several SLOFs, the one with the lowest SLOF is chosen).
 */

/**
 * In-memory representation of diseqc information for a particular
 * source_id/slof/polarization combination.
 */
struct dvbcfg_diseqc_entry {
        uint32_t slof;
        uint8_t polarization:2;

        uint32_t lof;
        char *command;

        struct dvbcfg_diseqc_entry *next;     /* NULL=> last entry */
};

/**
 * In-memory representation of diseqc information for a single source_id.
 */
struct dvbcfg_diseqc {
        struct dvbcfg_source* source;

        struct dvbcfg_diseqc_entry *entries;

        struct dvbcfg_diseqc *next;     /* NULL=> last entry */
};

/**
 * Diseqc backend API.
 */
struct dvbcfg_diseqc_backend {
        /**
         * Loads a single diseqc from the backend and add to the supplied list.
         *
         * @param backend Pointer to the backend structure concerned.
         * @param diseqcs Pointer to the list of diseqcs.
         * @return 0 on success, <0 on error, or 1 on end of file.
         */
        int (*get)(struct dvbcfg_diseqc_backend* backend,
                   struct dvbcfg_diseqc** diseqcs);

        /**
         * Stores a single diseqc to the backend.
         *
         * @param backend Pointer to the backend structure concerned.
         * @param diseqc Diseqc to store.
         * @return 0 on success, <0 on error.
         */
        int (*put)(struct dvbcfg_diseqc_backend* backend,
                   struct dvbcfg_diseqc* diseqc);
};

/**
 * Convenience method to load all diseqcs from a backend.
 *
 * @param config_file Config filename to load.
 * @param sources Where to put the pointer to the start of the loaded sources.
 * If NULL, a new list will be created, if it points to an already initialised
 * list, the loaded sources will be appended to it.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_diseqc_load(struct dvbcfg_diseqc_backend *backend,
                              struct dvbcfg_diseqc **diseqcs);

/**
 * Convenience method to store all sources to a backend.
 *
 * @param config_file Config filename to save.
 * @param sources Pointer to the list of sources to save.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_diseqc_save(struct dvbcfg_diseqc_backend *backend,
                              struct dvbcfg_diseqc *diseqcs);

/**
 * Create a new diseqc.
 *
 * @param diseqcs List of diseqcs to add to.
 * @param source The source of the diseqc.
 * @return The new dvbcfg_adapter structure, or NULL on error.
 */
extern struct dvbcfg_diseqc* dvbcfg_diseqc_new(struct dvbcfg_diseqc** diseqcs,
                                               struct dvbcfg_source* source);

/**
 * Add a new entry to a diseqc.
 *
 * @param diseqc Diseqc to add to.
 * @param slof Is the switching frequency for this entry (the maximum frequency
 * this entry allows). It should be in MHz.
 * @param polarization Is the polarization for this entry (one of DVBCFG_POLARIZATION_*).
 * @param lof The frequency (in MHz) to subtract from the channel frequency.
 * @param command The diseqc command to execute.
 * @return The new dvbcfg_diseqc_entry structure, or NULL on error.
 */
extern struct dvbcfg_diseqc_entry* dvbcfg_diseqc_add_entry(struct dvbcfg_diseqc* diseqc,
                                                           uint32_t slof,
                                                           uint8_t polarization,
                                                           uint32_t lof,
                                                           char *command);

/**
 * Remove an entry from a diseqc.
 *
 * @param diseqc Diseqc to remove from.
 * @param entry dvbcfg_diseqc_entry to remove.
 */
extern void dvbcfg_diseqc_remove_entry(struct dvbcfg_diseqc* diseqc,
                                       struct dvbcfg_diseqc_entry* entry);

/**
 * Find the matching dvcfg_diseqc for a particular source.
 *
 * @param diseqcs Pointer to the list to search.
 * @param source source concerned.
 * @return A dvbcfg_diseqc structure if found, or NULL if not.
 */
extern struct dvbcfg_diseqc *dvbcfg_diseqc_find(struct dvbcfg_diseqc* diseqcs,
                                                struct dvbcfg_source* source);

/**
 * Find the matching dvcfg_diseqc_entry within a source for a particular
 * frequency/polarization.
 *
 * @param diseqcs Pointer to the dvbcfg_diseqc previously found with
 * dvbcfg_diseqc_find().
 * @param frequency Frequency concerned.
 * @param polarization Polarization concerned.
 * @return A dvbcfg_diseqc_entry structure if found, or NULL if not.
 */
extern struct dvbcfg_diseqc_entry *dvbcfg_diseqc_find_entry(struct dvbcfg_diseqc* diseqc,
                                                            uint32_t frequency,
                                                            int polarization);

/**
 * Unlink a single diseqc from a list, and free its memory.
 *
 * @param diseqcs The list of diseqcs.
 * @param tofree The diseqc to free.
 */
extern void dvbcfg_diseqc_free(struct dvbcfg_diseqc **diseqcs,
                               struct dvbcfg_diseqc *tofree);

/**
 * Free memory for all diseqcs in a list.
 *
 * @param diseqcs Pointer to list of diseqcs to free.
 */
extern void dvbcfg_diseqc_free_all(struct dvbcfg_diseqc *diseqcs);

#ifdef __cplusplus
}
#endif

#endif
