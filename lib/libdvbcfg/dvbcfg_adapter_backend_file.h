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

#ifndef DVBCFG_ADAPTER_BACKEND_FILE_H
#define DVBCFG_ADAPTER_BACKEND_FILE_H 1

#include <cfg/dvbcfg_adapter.h>

/**
 * The file backend consists of multiple lines as follows:
 *
 * <adapter_id> <source_id> ...
 *
 * <adapter_id> identifies a DVB adapter in the system. The following adapter_ids are supported:
 *
 *   <adapter_number>.<frontend_number>
 *
 *   <adapter_number> is the number allocated to the DVB device by the OS (i.e. /dev/dvb/adapterX)
 *   <frontend_number> is the frontend ID on a particular DVB device (i.e. /dev/dvb/adapterX/frontendY)
 *
 * <source_id> corresponds to an entry in the dvbcfg_sources file. Multiple
 * source_ids can be specified for an adapter, indicating it can be automatically
 * switched between them in some manner (e.g. by using DISEQC for DVBS adapters).
 *
 * Comments begin with '#' - any characters after this will be ignored
 * to the end of the line.
 *
 * Examples:
 * 0.0 S-5E S-7E
 * 1.0
 * 2.0 T-uk-BlackHill
 */

/**
 * Create an instance of the file backend. This stores the adapters in a file
 * on disk.
 *
 * @param filename Pathname to the adapters file. Pass NULL to use the default
 * config file name/location.
 * @param sources Pointer to list of pre-loaded source instances.
 * @param create_sources If 1, any missing sources will automatically be added
 * to the supplied sources list. If 0, the missing source will be ignored.
 * @param backend Will be updated to point to the backend API instance.
 * @return 0 on success, nonzero on error.
 */
int dvbcfg_adapter_backend_file_create(const char* filename,
                                       struct dvbcfg_source** sources,
                                       int create_sources,
                                       struct dvbcfg_adapter_backend** backend);

/**
 * Destroy a file backend. Does not destroy any auto-created source instances.
 *
 * @param backend Pointer to the backend API instance to destroy.
 */
extern void dvbcfg_adapter_backend_file_destroy(struct dvbcfg_adapter_backend* backend);

#endif
