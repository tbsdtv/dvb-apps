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

#ifndef DVBCFG_DISEQC_BACKEND_FILE_H
#define DVBCFG_DISEQC_BACKEND_FILE_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <dvbcfg/dvbcfg_diseqc.h>

/**
 * The dvbcfg_diseqc file backend consists of a set of lines as follows:
 *
 * <source_id> <slof> <polarization> <lof> <diseqc command>
 *
 * <source_id> Should correspond to an entry in the dvbcfg_sources file. In
 * this file, the special source_id "*" is used to allow a set of default diseqc
 * entries to be specified.
 * <slof> Is the switching frequency for this entry (the maximum frequency this
 * entry allows). It should be in MHz.
 * <polarization> Is the polarization for this entry - one of 'H','V','L', or 'R'.
 * <lof> The frequency (in MHz) to subtract from the channel frequency if
 * this entry matches.
 * <diseqc command> The diseqc command to execute if this entry matches. The
 * syntax is described in dvbapi/dvbfe.h
 */


/**
 * Create an instance of the file backend. This stores the diseqcs in a file
 * on disk.
 *
 * @param filename Pathname to the diseqcs file. Pass NULL to use the default
 * config file name/location.
 * @param sources Pointer to list of pre-loaded source instances.
 * @param create_sources If 1, any missing sources will automatically be added
 * to the supplied sources list. If 0, the missing source will be ignored.
 * @param backend Will be updated to point to the backend API instance.
 * @return 0 on success, nonzero on error.
 */
int dvbcfg_diseqc_backend_file_create(const char* filename,
                                      struct dvbcfg_source** sources,
                                      int create_sources,
                                      struct dvbcfg_diseqc_backend** backend);

/**
 * Destroy a file backend. Does not destroy any auto-created source instances.
 *
 * @param backend Pointer to the backend API instance to destroy.
 */
extern void dvbcfg_diseqc_backend_file_destroy(struct dvbcfg_diseqc_backend* backend);

#ifdef __cplusplus
}
#endif

#endif
