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

#ifndef DVBCFG_SEED_BACKEND_FILE_H
#define DVBCFG_SEED_BACKEND_FILE_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <libdvbcfg/dvbcfg_seed.h>

/**
 * The file backend consists of multiple lines as follows:
 *
 * <source_id> <delivery>
 *
 * <source_id> Should correspond to an entry in the dvbcfg_sources file.
 * <delivery> is an externalised delivery structure as described in
 * dvbcfg_common.h.
 *
 * Comments begin with '#' - any characters after this will be ignored
 * to the end of the line.
 *
 * Examples:
 * S-28.8E 12345 0 H 27500000 9
 */

/**
 * Create an instance of the file backend. This stores the seeds in a file
 * on disk.
 *
 * @param filename Pathname to the seeds file. Pass NULL to use the default
 * config file name/location.
 * @param long_delivery If 1, the long format for delivery lines will be used
 * (human readable strings). If 0, the short format will be used (integers only).
 * @param sources Pointer to list of pre-loaded source instances.
 * @param create_sources If 1, any missing sources will automatically be added
 * to the supplied sources list. If 0, the missing source will be ignored.
 * @param backend Will be updated to point to the backend API instance.
 * @return 0 on success, nonzero on error.
 */
int dvbcfg_seed_backend_file_create(const char* filename,
                                    int long_delivery,
				    struct dvbcfg_source** sources,
				    int create_sources,
                                    struct dvbcfg_seed_backend** backend);

/**
 * Destroy a file backend. Does not destroy any auto-created source instances.
 *
 * @param backend Pointer to the backend API instance to destroy.
 */
extern void dvbcfg_seed_backend_file_destroy(struct dvbcfg_seed_backend* backend);

#ifdef __cplusplus
}
#endif

#endif
