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
#define DVBCFG_DISEQC_H

#include <stdint.h>

/**
 * The dvbcfg_diseqc file consists of multiple lines as follows:
 *
 * <source_id> <slof> <polarization> <lof> <diseqc command>
 *
 * <source_id> Should correspond to an entry in the dvbcfg_sources file. In this file, the special source_id
 * "*" is used to allow a set of default diseqc entries to be specified.
 * <slof> Is the switching frequency for this entry (the maximum frequency this entry allows). It should be in MHz.
 * <polarization> Is the polarization for this entry - one of 'H','V','L', or 'R'.
 * <lof> The frequency (in MHz) to subtract from the requested frequency if this entry matches.
 * <diseqc command> The diseqc command to execute if this entry matches.
 *
 * A diseqc command consists of a sequence of the following codes, separated by whitespace:
 * t        - turn 22kHz tone off.
 * T        - turn 22kHz tone on.
 * _        - set voltage to 0v (i.e. off).
 * v        - set voltage to 13v.
 * V        - set voltage to 18v.
 * +        - Enable high LNB voltage.
 * -        - Disable high LNB voltage.
 * A        - send DISEQC mini command A.
 * B        - send DISEQC mini command B.
 * Wii      - Delay for ii milliseconds.
 * [XX ...] - Send a diseqc master command. The command may be up to 6 bytes long, each byte must be in hex-ascii.
 *
 * Comments begin with '#' - any characters after this will be ignored
 * to the end of the line.
 *
 * Examples:
 * S19.2E  11700 V  9750  t v W15 [E0 10 38 F0] W15 A W15 t
 * S19.2E  99999 V 10600  t v W15 [E0 10 38 F1] W15 A W15 T
 * S19.2E  11700 H  9750  t V W15 [E0 10 38 F2] W15 A W15 t
 * S19.2E  99999 H 10600  t V W15 [E0 10 38 F3] W15 A W15 T
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

	struct dvbcfg_diseqc_entry *next;	/* NULL=> last entry */
	struct dvbcfg_diseqc_entry *prev;	/* NULL=> first entry */
};

/**
 * In-memory representation of diseqc information for a single source_id.
 */
struct dvbcfg_diseqc {
	char *source_id;
	struct dvbcfg_diseqc_entry *entries;

	struct dvbcfg_diseqc *next;	/* NULL=> last entry */
	struct dvbcfg_diseqc *prev;	/* NULL=> first entry */
};


/**
 * Load diseqcs from a config file.
 *
 * @param config_file Config filename to load.
 * @param diseqcs Where to put the pointer to the start of the loaded
 * diseqcs. If NULL, a new list will be created, if it points to an already initialised list,
 * the loaded diseqcs will be appended to it.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_diseqc_load(char *config_file,
			      struct dvbcfg_diseqc **diseqcs);

/**
 * Save diseqcs to a config file.
 *
 * @param config_file Config filename to save.
 * @param diseqcs Pointer to the list of diseqcs to save.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_diseqc_save(char *config_file,
			      struct dvbcfg_diseqc *diseqcs);

/**
 * Find the matching dvcfg_diseqc for a particular source_id.
 *
 * @param diseqcs Pointer to the list to search.
 * @param source_id source_id concerned.
 * @return A dvbcfg_diseqc structure if found, or NULL if not.
 */
extern struct dvbcfg_diseqc *dvbcfg_diseqc_find(struct dvbcfg_diseqc
						*diseqcs, char *source_id);

/**
 * Find the matching dvcfg_diseqc_entry within a source for a particular frequency/polarization.
 *
 * @param diseqcs Pointer to the dvbcfg_diseqc previously found with dvbcfg_diseqc_find().
 * @param frequency Frequency concerned.
 * @param polarization Polarization concerned.
 * @return A dvbcfg_diseqc_entry structure if found, or NULL if not.
 */
extern struct dvbcfg_diseqc_entry *dvbcfg_diseqc_find_entry(struct
							    dvbcfg_diseqc
							    *diseqc,
							    uint32_t
							    frequency,
							    int
							    polarization);

/**
 * Unlink a single diseqc from a list, and free its memory.
 *
 * @param diseqcs The list of diseqcs.
 * @param tofree The diseqc to free.
 */
extern void dvbcfg_diseqc_free(struct dvbcfg_diseqc **diseqcs,
			       struct dvbcfg_diseqc *tofree);

/**
 * Unlink a single dvbcfg_diseqc_entry from a dvbcfg_diseqc, and free its memory.
 *
 * @param diseqc The dvbcfg_diseqc.
 * @param tofree The dvbcfg_diseqc_entry to free.
 */
extern void dvbcfg_diseqc_free_entry(struct dvbcfg_diseqc *diseqc,
				     struct dvbcfg_diseqc_entry *tofree);

/**
 * Free memory for all diseqcs in a list.
 *
 * @param diseqcs Pointer to list of diseqcs to free.
 */
extern void dvbcfg_diseqc_free_all(struct dvbcfg_diseqc *diseqcs);

#endif				// DVBCFG_DISEQC_H
