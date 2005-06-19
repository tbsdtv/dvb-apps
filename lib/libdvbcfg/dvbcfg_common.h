/**
 * DVBCFG common definitions.
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

#ifndef DVBCFG_COMMON_H
#define DVBCFG_COMMON_H

/**
 * Possible polarization values.
 */
#define DVBCFG_POLARIZATION_H 0
#define DVBCFG_POLARIZATION_V 1
#define DVBCFG_POLARIZATION_L 2
#define DVBCFG_POLARIZATION_R 3



/**
 * A Unique Multiplex ID uniquely identifies a multiplex across the global space of all multiplexes
 * in all DVB transmission types.
 *
 * The externalised string version of this value is as follows:
 * <source_id>:<original_network_id>:<transport_stream_id>:<multiplex_differentiator>
 *
 * <source_id> corresponds to one of the entries in the dvbcfg_sources file.
 * <original_network_id> Is the ID of the original broadcaster of the multiplex.
 * <transport_stream_id> Is the ID of the multiplex as allocated by the broadcaster.
 * <multiplex_differentiator> Inevitably there are clashes with the above values. This final value permits these
 * clashes to be resolved. The exact definition depends on the DVB type:
 *   DVBS: ((frequency / (symbolrate/1000)) << 2) | polarisation
 *   DVBT: (frequency / bandwidth)
 *   DVBC: (frequency / symbolrate)
 *   ATSC: (frequency / 6000000)
 */
struct umid
{
        char* source_id;
        uint32_t original_network_id;
        uint32_t transport_stream_id;
        uint32_t multiplex_differentiator;
};


/**
 * A Unique Service ID uniquely identifies a service within its multiplex.
 *
 * The externalised string version of this value is as follows:
 * <program_number>:<service_differentiator>
 *
 * <program_number> The program number of the service as allocated by the broadcaster (this is also known as the service_id).
 * <service_differentiator> In case there are clashes with the above, this final value permits these clashes to be resolved.
 *   The value will be the index of the service in the PAT table transmitted in its multiplex.
 *   Note: this should be set to 0 there are no clashes.
 */
struct usid
{
        uint32_t program_number;
        uint32_t service_differentiator;
};


/**
 * A Global Service ID uniquely identifies a service across the global space of all multiplexes
 * in all DVB transmission types. It is _almost_ simply the concatenation of UMID and USID, but not quite.
 *
 * FIXME: info about DVB-T horribleness
 */
struct gsid
{
        char* source_id;
        uint32_t original_network_id;
        uint32_t transport_stream_id;
        uint32_t multiplex_differentiator;

        struct usid usid;
};

/**
 * Convert a UMID structure into a string.
 *
 * @param umid The UMID structure.
 * @return Pointer to a malloc()ed buffer containing the string, or NULL on failure.
 */
extern char* umid_to_string(struct umid* umid);

/**
 * Parse a string into a UMID.
 *
 * @param string The string to parse.
 * @param umid Pointer to a UMID structure to fill out.
 *
 * @return 0 on success, nonzero on failure.
 */
extern int umid_from_string(char* string, struct umid* umid);

/**
 * Convert a USID structure into a string.
 *
 * @param umid The USID structure.
 * @return Pointer to a malloc()ed buffer containing the string, or NULL on failure.
 */
extern char* usid_to_string(struct umid* usid);

/**
 * Parse a string into a USID.
 *
 * @param string The string to parse.
 * @param umid Pointer to a USID structure to fill out.
 *
 * @return 0 on success, nonzero on failure.
 */
extern int usid_from_string(char* string, struct umid* usid);

/**
 * Convert a GSID structure into a string.
 *
 * @param umid The GSID structure.
 * @return Pointer to a malloc()ed buffer containing the string, or NULL on failure.
 */
extern char* gsid_to_string(struct umid* gsid);

/**
 * Parse a string into a GSID.
 *
 * @param string The string to parse.
 * @param umid Pointer to a GSID structure to fill out.
 *
 * @return 0 on success, nonzero on failure.
 */
extern int gsid_from_string(char* string, struct umid* gsid);

#endif                          // DVBCFG_COMMON_H
