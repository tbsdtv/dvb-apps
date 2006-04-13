/**
 * dvbcfg_multiplex configuration file support.
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

#ifndef DVBCFG_MULTIPLEX_BACKEND_FILE_H
#define DVBCFG_MULTIPLEX_BACKEND_FILE_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <libdvbcfg/dvbcfg_multiplex.h>
#include <libdvbcfg/dvbcfg_common.h>

/**
 * The file backend consists of multiple sections, each describing a
 * particular multiplex or a service belonging to a multiplex. Its format is as
 * follows:
 * [multiplexes]
 * version=0.1
 * date=<unixdate>
 *
 * [m]
 * gmid = <GMID of this multiplex>
 * d = <delivery specific parameters>
 *
 * [s]
 * usid= <USID of this service>
 * name= <name of this service>
 * sname= <short name of this service>
 * pname= <provider name of this service>
 * flags= <service specific flags>
 * ca= <list of ca systems supported by this multiplex>
 * zap= <list of pids and their types for accelerated channel locking>
 * pmt= <list of pids and their types used as a supplement/replacement to the standard PMT SI tables>
 *
 * There may only be one [dvbmultiplex] section, and it must be the first
 * section within the file.
 *
 * There can be multiple [m] and [s] sections in any one file. A [s] belongs
 * to the most recent preceding [m] section.
 *
 * All keys within the sections are mandatory, except for the following:
 * flags, sname, pname, ca, zap, pmt.
 *
 * GMID and USID are described in dvbcfg_common.h.
 *
 * The <delivery specific parameters> are described in dvbcfg_common.h.
 *
 * Currently only the flag "nopmt" is defined for the <service specific flags>.
 * If this is present, the PMT should be ignored completely, and the pmt entries
 * used instead.
 *
 * The <list of ca systems supported by this multiplex> is only used for
 * encrypted services to identify the type of CAM/subscription card the user
 * must have in order to successfully decrypt a service. This is just a list of
 * numbers seperated by whitespace.
 *
 * Both zap and pmt have the same format: <pid>:<type>. <type> may either be
 * one of the standard PMT stream types as defined in ISO13818-1, table 2-29,
 * or it may be one of the special values "_ac3", "_dts", "_tt", or "_pcr".
 *
 * zap is to provide accelerated channel zapping. It gives a pre-extracted list
 * of PIDs (usually just audio, video, and pcr) so that a channel can be zapped
 * to without having to wait for the PAT or PMT tables. They should be verified
 * against the PAT/PMT when it is received, in case they have changed in the
 * meantime. Use of these is optional.
 *
 * pmt gives a list of extra PID/type pairs for PIDS that are for some reason
 * missing from the PMT table for a channel. If the "nopmt" flag is also set,
 * these entries will be used instead of a PMT. If not, they will be used in
 * addition to the PMT (however the pmt entries should override any clashing
 * entries in the PMT).
 *
 * Comments begin with '#' - any characters after this will be ignored
 * to the end of the line.
 *
 * Example:
 * [dvbmultiplexes]
 * version=0.1
 * date=5798475834
 *
 * [m]
 * gmid=S-5E:0x0001:0x002:0x000
 * d=12345 0 H 27500000 9
 *
 * [s]
 * usid=0x0001:0x000
 * name=service 1
 * zap=0x55:_ac3 0x56:0x78 0x57:_pcr 0x59:0x01
 * pmt=0x100:_dts 0x101:_tt 0x102:0x76
 *
 * [s]
 * usid=0x0002:0x000
 * name=service 2
 * zap=0x55:_ac3 0x56:0x78 0x57:_pcr 0x59:0x01
 * pmt=0x100:_dts 0x101:_tt 0x102:0x76
 *
 * [m]
 * gmid=C-de-de-Berlin:0x0001:0x002:0x000
 * d = 12345 0 27500000 9 4
 */


/**
 * Create an instance of the file backend. This stores the multiplexes in a file
 * on disk.
 *
 * @param filename Pathname to the multiplexes file. Pass NULL to use the default
 * config file name/location.
 * @param sources Pointer to list of pre-loaded source instances.
 * @param create_sources If 1, any missing sources will automatically be added
 * to the supplied sources list. If 0, the missing source will be ignored.
 * @param long_delivery If 1, the long format for delivery lines will be used
 * (human readable strings). If 0, the short format will be used (integers only).
 * @param backend Will be updated to point to the backend API instance.
 * @return 0 on success, nonzero on error.
 */
extern int dvbcfg_multiplex_backend_file_create(const char* filename,
                                                struct dvbcfg_source** sources,
                                                int create_sources,
                                                int long_delivery,
                                                struct dvbcfg_multiplex_backend** backend);

/**
 * Destroy a file backend. Does not destroy any auto-created source instances.
 *
 * @param backend Pointer to the backend API instance to destroy.
 */
extern void dvbcfg_multiplex_backend_file_destroy(struct dvbcfg_multiplex_backend* backend);

#ifdef __cplusplus
}
#endif

#endif
