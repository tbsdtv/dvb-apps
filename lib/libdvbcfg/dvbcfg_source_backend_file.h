/**
 * dvbcfg_source file backend.
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

#ifndef DVBCFG_SOURCE_BACKEND_FILE_H
#define DVBCFG_SOURCE_BACKEND_FILE_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <dvbcfg/dvbcfg_common.h>
#include <dvbcfg/dvbcfg_source.h>

/**
 * The file backend stores sources in a file on disk as follows:
 *
 * <source_id> <human readable description>
 *
 * Comments begin with '#' - any characters after this will be ignored to the
 * end of the line.
 *
 * Examples:
 * S-5E     Sirius 2/3
 * S-13E    Hotbird 1-(5)-6
 * T-au-au-Adelaide A DVB-T transmitter in Australia serving the Adelaide area.
 * T-uk-scottish-BlackHill A DVB-T transmitter in the UK serving the central belt of scotland.
 */

/**
 * Create an instance of the file backend. This stores the source in a file
 * on disk.
 *
 * @param filename Pathname to the sources file. Pass NULL to use the default
 * config file name/location.
 * @param backend Will be updated to point to the backend API instance.
 * @return 0 on success, nonzero on error.
 */
extern int dvbcfg_source_backend_file_create(const char* filename,
                                             struct dvbcfg_source_backend** backend);

/**
 * Destroy a file backend.
 *
 * @param backend Pointer to the backend API instance to destroy.
 */
extern void dvbcfg_source_backend_file_destroy(struct dvbcfg_source_backend* backend);

#ifdef __cplusplus
}
#endif

#endif
