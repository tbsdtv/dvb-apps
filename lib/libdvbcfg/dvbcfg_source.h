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
 * The sources file defines standardised unique IDs for all DVB transmitters (as there is no
 * other real standard). It consists of multiple lines as follows:
 *
 * <source_id> <human readable description>
 *
 * The first character of <source_id> gives the type of DVB source:
 *
 *   DVBS: "S"
 *   DVBT: "T"
 *   DVBC: "C"
 *   ATSC: "A"
 *
 * For DVBS, <source_id> is a unique identifier for a satellite cluster. It is defined to
 * be "S"<longitude><"E"|"W"> - i.e. the orbital position of the cluster. For convenience,
 * the <source_id> of a DVBS source is defined to be the same as the <source_network>
 * (see below). <source_locale> has no meaning for a DVBS source.
 *
 * All other DVB types have a slight complication. Unlike DVBS, these consist of multiple
 * locales spaced over a wide area (e.g. a country). Between each locale, _roughly_ the same
 * services/multiplexes are available, but can be on different frequencies. Also, some
 * networks provide localised programming, so the exact service lineup can and does
 * vary from locale to locale.
 *
 * Therefore, for DVBT, DVBC, and ATSC, the <source_id> is split into two components as follows:
 * <source_id> == <source_network>-<source_locale>
 *
 * <source_network> is the name of the DVB network. Currently we are simply using the country
 * code for this (e.g "Tuk"). However if necessary, this can easily be extended to allow multiple
 * networks, for example "Tuk:network1", "Tuk:network2". Note that <source_network> may not
 * contain '-' or whitespace characters.
 *
 * <source_locale> is an description of the physical location of the source. For example, in the
 * UK, the <source_locale> for a DVBT source is the name of the DVBT transmitter (e.g. BlackHill).
 * <source_locale> may contain any character except whitespace.
 *
 * Comments begin with '#' - any characters after this will be ignored to the end of the line.
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
        char *source_network;
        char *source_locale;            /* NULL for DVBS */
        char *description;

        struct dvbcfg_source *prev;     /* NULL=> this is the first entry */
        struct dvbcfg_source *next;     /* NULL=> this is the last entry */
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
 * @param source_network source_network to find.
 * @param source_locale source_local to find (pass NULL for DVBS, or to simply match by source_locale).
 * @return A dvbcfg_source structure if found, or NULL if not.
 */
extern struct dvbcfg_source *dvbcfg_source_find(struct dvbcfg_source *sources, char *source_network, char* source_locale);

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

#endif                          // DVBCFG_SOURCE_H
