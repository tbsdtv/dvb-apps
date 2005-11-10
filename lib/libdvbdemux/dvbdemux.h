/*
 * libdvbdemux - a DVB demux library
 *
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#ifndef LIBDVBDEMUX_H
#define LIBDVBDEMUX_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

/**
 * Source of the data to be demuxed.
 */
#define DVBDEMUX_INPUT_FRONTEND 0
#define DVBDEMUX_INPUT_DVR 1

/**
 * Destination of the demuxed data.
 */
#define DVBDEMUX_OUTPUT_DECODER 0
#define DVBDEMUX_OUTPUT_DEMUX 1
#define DVBDEMUX_OUTPUT_DVR 2

/**
 * PES types.
 */
#define DVBDEMUX_PESTYPE_AUDIO 0
#define DVBDEMUX_PESTYPE_VIDEO 1
#define DVBDEMUX_PESTYPE_TELETEXT 2
#define DVBDEMUX_PESTYPE_SUBTITLE 3
#define DVBDEMUX_PESTYPE_PCR 4

/**
 * Open a demux device. Can be called multiple times. These let you setup a
 * single filter per FD. It can can also be read() from if you use a section
 * filter, or create a pes_filter or raw_filter with output DVBDEMUX_OUTPUT_DEMUX.
 *
 * @param adapter Index of the DVB adapter.
 * @param demuxdevice Index of the demux device on that adapter (usually 0).
 * @return A unix file descriptor on success, or -1 on failure.
 */
extern int dvbdemux_open_demux(int adapter, int demuxdevice);

/**
 * Open a DVR device. May be opened for writing once, or multiple times in readonly
 * mode. It is used to either write() transport stream data to be demuxed
 * (if input == DVBDEMUX_INPUT_DVR), or to read() a stream of demuxed data
 * (if output == DVBDEMUX_OUTPUT_DVR).
 *
 * Note, all demux filters with output set to DVBDEMUX_OUTPUT_DVR will be
 * multiplexed together and output their data on this device.
 *
 * @param adapter Index of the DVB adapter.
 * @param dvrdevice Index of the dvr device on that adapter (usually 0)
 * @param readonly If 1, frontend will be opened in readonly mode only.
 * @return A unix file descriptor on success, or -1 on failure.
 */
extern int dvbdemux_open_dvr(int adapter, int dvrdevice, int readonly);

/**
 * Set filter for capturing decoded SI table sections. This allows filtering on up
 * to the first 16 bytes of a section header. Conceptually, the driver computes
 * the following for each bit.
 *
 * (filter[X].bit[Y] & mask[X].bit[Y]) TESTOPERATION (header[X].bit[Y] & mask[X].bit[Y]).
 *
 * testtype can be either "==", or "!=" - the exact operation used is determined
 * as follows:
 *
 * if (testtype[X].bit[Y] == 0) then TESTOPERATION='==' else TESTOPERATION='!='.
 *
 * So it is possible to test that selected bits of a section header match the
 * filter AND that they DO NOT match. Any sections which do not match this
 * criteria for every bit will be discarded.
 *
 * The SI data is always read from the frontend, and is always returned by
 * read()ing the demux fd. FIXME: check this statement!
 *
 * @param fd FD as opened with dvbdemux_open_demux() above.
 * @param pid PID of the stream.
 * @param filter The filter values of the first 16 bytes of the desired sections.
 * @param mask Bitmask indicating which bits in values array should be tested
 * (if a bit is 1, it will be tested).
 * @param testtype Bitmask indicating the type of test to be applied to bit in
 * the values array. Note this is only applied to bits which have a '1' in the
 * mask array. If a testtype bit is 0, the corresponding bit in the section header
 * MUST match the value of the bit in filter. If a testtype bit is 1, it *MUST NOT*
 * match the value of the bit in filter.
 * @param start If 1, the filter will be started immediately. Otherwise you must
 * call dvbdemux_start() manually.
 * @param checkcrc If 1, the driver will check the CRC on the table sections.
 * Any bad sections will be dropped.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbdemux_set_section_filter(int fd, int pid,
                                       uint8_t filter[16], uint8_t mask[16], uint8_t testtype[16],
                                       int start, int checkcrc);

/**
 * Set filter for capturing PES data. This call can only used for cards
 * equipped with a hardware decoder.
 *
 * @param fd FD as opened with dvbdemux_open_demux() above.
 * @param pid PID of the stream.
 * @param input One of DVBDEMUX_INPUT_*.
 * @param output One of DVBDEMUX_OUTPUT_*.
 * @param pestype One of DVBDEMUX_PESTYPE_* - this tells the decoder the type
 * of data in this stream.
 * @param start If 1, the filter will be started immediately. Otherwise you must
 * call dvbdemux_start() manually.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbdemux_set_pes_filter(int fd, int pid,
                                   int input, int output,
                                   int pestype,
                                   int start);

/**
 * Create a pid filter - this will deliver raw transport stream packets for a
 * specified PID.
 *
 * Note: The wildcard PID can only be used on budget cards.
 *
 * @param fd FD as opened with dvbdemux_open_demux() above.
 * @param pid PID to retrieve, or use -1 as a wildcard for ALL PIDs.
 * @param input One of DVBDEMUX_INPUT_*.
 * @param output One of DVBDEMUX_OUTPUT_*.
 * @param start If 1, the filter will be started immediately. Otherwise you must
 * call dvbdemux_start() manually.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbdemux_set_pid_filter(int fd, int pid,
                                   int input, int output,
                                   int start);

/**
 * Start a demux going.
 *
 * @param fd FD as opened with dvbdemux_open_demux() above.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbdemux_start(int fd);

/**
 * Stop a demux.
 *
 * @param fd FD as opened with dvbdemux_open_demux() above.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbdemux_stop(int fd);

/**
 * Retrieve the current STC from the demux.
 *
 * @param fd FD as opened with dvbdemux_open_demux() above.
 * @param stc Where to put the retrieved STC value (in 90kHz clock).
 * @return 0 on success, nonzero on failure.
 */
extern int dvbdemux_get_stc(int fd, uint64_t *stc);

/**
 * Change the internal buffer size used by the demuxer. The default buffer size
 * is 8192 bytes.
 *
 * @param fd FD as opened with dvbdemux_open_demux() above.
 * @param bufsize New buffer size to use.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbdemux_set_buffer(int fd, int bufsize);

#ifdef __cplusplus
}
#endif

#endif // LIBDVBDEMUX_H
