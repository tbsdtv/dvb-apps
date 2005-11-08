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

#include <cfg/dvbcfg_seed.h>

/**
 * The file backend consists of multiple lines as follows:
 *
 * <delivery>
 *
 * <delivery> is an externalised delivery structure as described in
 * dvbcfg_common.h.
 *
 * The name of the seed file should correspond to the source_id it is a seed
 * for (e.g. S-5E.conf).
 *
 * Comments begin with '#' - any characters after this will be ignored
 * to the end of the line.
 *
 * Examples:
 * 12345 0 H 27500000 9
 */

/**
 * Create an instance of the file backend. This stores the seeds in a file
 * on disk.
 *
 * @param basedir Base pathname to seeds storage area (pass NULL to use the
 * system default basedir).
 * @param filename Pathname to the seeds file relative to basedir.
 * @param long_delivery If 1, the long format for delivery lines will be used
 * (human readable strings). If 0, the short format will be used (integers only).
 * @param source_type source_type of the deliveries in this seed file.
 * @param backend Will be updated to point to the backend API instance.
 * @return 0 on success, nonzero on error.
 */
int dvbcfg_seed_backend_file_create(const char* basedir,
                                    const char* filename,
                                    int long_delivery,
                                    enum dvbcfg_sourcetype source_type,
                                    struct dvbcfg_seed_backend** backend);

/**
 * Destroy a file backend. Does not destroy any auto-created source instances.
 *
 * @param backend Pointer to the backend API instance to destroy.
 */
extern void dvbcfg_seed_backend_file_destroy(struct dvbcfg_seed_backend* backend);

#endif
