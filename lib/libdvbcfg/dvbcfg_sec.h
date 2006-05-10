/**
 * dvbcfg_sec (i.e. linuxtv SEC format) configuration file support.
 *
 * Copyright (c) 2006 by Andrew de Quincey <adq_dvb@lidskialf.net>
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

#ifndef DVBCFG_SEC_H
#define DVBCFG_SEC_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdint.h>
#include <libdvbapi/dvbfe.h>

struct dvbcfg_sec
{
	/* these elements are use to match the entry */
        char sec_id[128];
	uint32_t slof; /* switching frequency */
	enum dvbfe_polarization polarization;

	/* these elements describe the SEC parameters */
	uint32_t lof; /* frequency to match */
	char command[256];

	/* these two are not used by this library - they're provided as a
	 * convenience for applications to use */
	struct dvbcfg_sec *next;
	struct dvbcfg_sec *prev;
};

/**
 * Callback function used in dvbcfg_sec_load().
 *
 * @param private Private information to caller.
 * @param channel The current channel details.
 * @return 0 to continue, 1 to stop loading.
 */
typedef int (*dvbcfg_sec_callback)(void *private, struct dvbcfg_sec *sec);

/**
 * Load an SEC file.
 *
 * @param f File to load from.
 * @param private Value to pass to 'private' in callback above.
 * @param cb Callback function called for each channel loaded from the file.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_sec_load(FILE *f, void *private,
			   dvbcfg_sec_callback cb);

/**
 * Convenience function to parse an SEC config file and find details for a particular setting.
 *
 * @param config_file Config filename to load.
 * @param sec_id ID of SEC channel.
 * @param frequency Desired frequency.
 * @param polarization Desired polarization.
 * @param channel Where to put the details if found.
 * @return 0 on success, nonzero on error.
 */
extern int dvbcfg_sec_find(const char *config_file,
			   const char *sec_id,
			   uint32_t frequency,
			   enum dvbfe_polarization polarization,
			   struct dvbcfg_sec *sec);

/**
 * Save SEC format config file.
 *
 * @param f File to save to.
 * @param secs Pointer to array of SECs to save.
 * @param count Number of entries in the above array.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_sec_save(FILE *f,
			   struct dvbcfg_sec *secs,
			   int count);

#ifdef __cplusplus
}
#endif

#endif
