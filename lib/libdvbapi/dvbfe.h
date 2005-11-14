/*
 * libdvbfe - a DVB frontend library
 *
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
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

#ifndef LIBDVBFE_H
#define LIBDVBFE_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/dvb/frontend.h>
#include <stdint.h>

/**
 * Mask of values used in the dvbfe_get_status() call.
 */
#define DVBFE_STATUS_FE                 0x01
#define DVBFE_STATUS_BER                0x02
#define DVBFE_STATUS_SIGNAL_STRENGTH    0x04
#define DVBFE_STATUS_SNR                0x08
#define DVBFE_STATUS_UNCORRECTED_BLOCKS 0x10

/**
 * Structure containing values used by the dvbfe_get_status() call.
 */
struct dvbfe_status {
	unsigned int signal     : 1;
	unsigned int carrier    : 1;
	unsigned int viterbi    : 1;
	unsigned int sync       : 1;
	unsigned int lock       : 1;
	uint32_t ber;
	uint16_t signal_strength;
	uint16_t snr;
	uint32_t ucblocks;
};

/**
 * Open a DVB frontend.
 *
 * @param adapter DVB adapter ID.
 * @param frontend Frontend ID of that adapter to open.
 * @param readonly If 1, frontend will be opened in readonly mode only.
 * @return A unix file descriptor on success, or -1 on failure.
 */
extern int dvbfe_open(int adapter, int frontend, int readonly);

/**
 * Retrieve information on the frontend.
 *
 * @param fd FD opened with libdvbfe_open().
 * @param info Place to put extracted information.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbfe_get_info(int fd, struct dvb_frontend_info *info);

/**
 * Retrieve status of frontend.
 *
 * @param fd FD opened with libdvbfe_open().
 * @param statusmask ORed bitmask of desired DVBFE_STATUS_* values.
 * @param result Where to put the retrieved results.
 * @return ORed bitmask of DVBFE_STATUS_* indicating which values were read successfully.
 */
extern int dvbfe_get_status(int fd, int statusmask, struct dvbfe_status *result);

/**
 * Set the frontend tuning parameters.
 *
 * @param fd FD opened with libdvbfe_open().
 * @param params Params to set.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbfe_set_frontend(int fd, struct dvb_frontend_parameters *params);

/**
 * Read the current frontend tuning parameters from the hardware.
 *
 * @param fd FD opened with libdvbfe_open().
 * @param params Where to put the parameters.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbfe_get_frontend(int fd, struct dvb_frontend_parameters *params);

/**
 * Execute a DISEQC command string (format as specified in libdvbcfg diseqc.conf).
 *
 * @param fd FD opened with libdvbfe_open().
 * @param command Command to execute.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbfe_diseqc_command(int fd, char *command);

/**
 * Read a DISEQC response from the frontend.
 *
 * @param fd FD opened with libdvbfe_open().
 * @param timeout Timeout for DISEQC response.
 * @param buf Buffer to store response in.
 * @param len Number of bytes in buffer.
 * @return >= 0 on success (number of received bytes), <0 on failure.
 */
extern int dvbfe_diseqc_read(int fd, int timeout, unsigned char *buf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif // LIBDVBFE_H
