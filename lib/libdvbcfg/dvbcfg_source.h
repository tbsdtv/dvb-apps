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
#define DVBCFG_SOURCE_H

/**
 * The sources file consists of multiple lines as follows:
 *
 * <source_id> <human readable description>
 *
 * The format of <source_id> depends on the DVB type.
 * DVBS: "S"<orbital position>
 * DVBT: "T"<country code and or network>-<physical location>
 * DVBC: "C"<country code and or network>-<physical location>
 *
 * Comments begin with '#' - any characters after this will be ignored
 * to the end of the line.
 *
 * Examples:
 * S5E     Sirius 2/3
 * S13E    Hotbird 1-(5)-6
 * Tau-Adelaide A DVB-T transmitter in Australia serving the Adelaide area.
 * Tuk-BlackHill A DVB-T transmitter in the UK serving the central belt of scotland.
 */

/**
 * In-memory representation of a single source.
 */
struct dvbcfg_source {
	char *source_id;
	char *description;

	struct dvbcfg_source *prev;	/* NULL=> this is the first entry */
	struct dvbcfg_source *next;	/* NULL=> this is the last entry */
};


/**
 * Load sources from a config file.
 *
 * @param config_file Config filename to load.
 * @param sources Where to put the pointer to the start of the loaded
 * sources. If NULL, a new list will be created, if it points to an already initialised list,
 * the loaded sources will be appended to it.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_source_load(char *config_file,
			      struct dvbcfg_source **sources);

/**
 * Save sources to a config file.
 *
 * @param config_file Config filename to save.
 * @param sources Pointer to the list of sources to save.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_source_save(char *config_file,
			      struct dvbcfg_source *sources);

/**
 * Find the entry for a particular source_id.
 *
 * @param sources Pointer to the list to search.
 * @param source_id source_id to find.
 * @return A dvbcfg_source structure if found, or NULL if not.
 */
extern struct dvbcfg_source *dvbcfg_source_find(struct dvbcfg_source
						*sources, char *source_id);

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

#endif				// DVBCFG_SOURCE_H
