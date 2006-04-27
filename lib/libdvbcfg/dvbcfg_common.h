/**
 * dvbcfg common definitions.
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

#ifndef DVBCFG_COMMON_H
#define DVBCFG_COMMON_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <libdvbapi/dvbfe.h>

/**
 * A delivery represents how to tune to a specific channel. The format varies
 * depending on the source_type of that channel, as follows:
 *
 * DVBS: <frequency> <inversion> <polarization> <symbol_rate> <fec_inner>
 * DVBC: <frequency> <inversion> <symbol_rate> <fec_inner> <modulation>
 * DVBT: <frequency> <inversion> <bandwidth> <code_rate_HP> <code_rate_LP> <constellation> <tranmission_mode> <guard_interval> <hierarchy_information>
 * ATSC: <frequency> <inversion> <modulation>
 *
 * All numerical values in the delivery are in the units used in the
 * "struct dvbfe_parameters". For other parameters, there are two
 * possiblities: either the numerical value as defined in the enumerations in
 * dvbfe.h, or the exact string corresponding to that numerical value as
 * defined in dvbfe.h. The same format for all such values must be used
 * within a single entry.
 */
struct dvbcfg_delivery {
        union {
		struct dvbfe_parameters dvb;
        } u;
};

/**
 * Parse a string of externalised delivery parameters as described in
 * dvbcfg_multiplex_backend.h.
 *
 * @param delivery_str The string to parse.
 * @param source_type The type of source this delivery is for.
 * @param delivery The structure where the parsed parameters should be written.
 * @return 0 on success, non-zero on error.
 */
extern int dvbcfg_delivery_from_string(char * delivery_str,
                                       dvbfe_type_t source_type,
                                       struct dvbcfg_delivery *delivery);

/**
 * Generate an externalised string version of delivery parameters.
 *
 * @param source_type The type of source this delivery is for.
 * @param long_delivery If 1 the long version will be used, if 0 the short version.
 * @param delivery The delivery parameters structure.
 * @param dest Where to put the string.
 * @param destsz Size of dest in bytes.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbcfg_delivery_to_string(dvbfe_type_t source_type,
                                     int long_delivery,
                                     struct dvbcfg_delivery *delivery,
                                     char* dest,
                                     int destsz);

#ifdef __cplusplus
}
#endif

#endif
