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

#include <stdint.h>

/**
 * The types of frontend we support.
 */
typedef enum dvbfe_type {
	DVBFE_TYPE_DVBS,
	DVBFE_TYPE_DVBC,
	DVBFE_TYPE_DVBT,
	DVBFE_TYPE_ATSC,
} dvbfe_type_t;

typedef enum dvbfe_polarization {
	DVBFE_POLARIZATION_H,
	DVBFE_POLARIZATION_V,
	DVBFE_POLARIZATION_L,
	DVBFE_POLARIZATION_R,
} dvbfe_polarization_t;

typedef enum dvbfe_spectral_inversion {
	DVBFE_INVERSION_OFF,
	DVBFE_INVERSION_ON,
	DVBFE_INVERSION_AUTO
} dvbfe_spectral_inversion_t;

typedef enum dvbfe_code_rate {
	DVBFE_FEC_NONE,
	DVBFE_FEC_1_2,
	DVBFE_FEC_2_3,
	DVBFE_FEC_3_4,
	DVBFE_FEC_4_5,
	DVBFE_FEC_5_6,
	DVBFE_FEC_6_7,
	DVBFE_FEC_7_8,
	DVBFE_FEC_8_9,
	DVBFE_FEC_AUTO
} dvbfe_code_rate_t;

typedef enum dvbfe_modulation {
	DVBFE_QPSK,
	DVBFE_QAM_16,
	DVBFE_QAM_32,
	DVBFE_QAM_64,
	DVBFE_QAM_128,
	DVBFE_QAM_256,
	DVBFE_QAM_AUTO,
	DVBFE_VSB_8,
	DVBFE_VSB_16
} dvbfe_modulation_t;

typedef enum dvbfe_transmit_mode {
	DVBFE_TRANSMISSION_MODE_2K,
	DVBFE_TRANSMISSION_MODE_8K,
	DVBFE_TRANSMISSION_MODE_AUTO
} dvbfe_transmit_mode_t;

typedef enum dvbfe_bandwidth {
	DVBFE_BANDWIDTH_8_MHZ,
	DVBFE_BANDWIDTH_7_MHZ,
	DVBFE_BANDWIDTH_6_MHZ,
	DVBFE_BANDWIDTH_AUTO
} dvbfe_bandwidth_t;

typedef enum dvbfe_guard_interval {
	DVBFE_GUARD_INTERVAL_1_32,
	DVBFE_GUARD_INTERVAL_1_16,
	DVBFE_GUARD_INTERVAL_1_8,
	DVBFE_GUARD_INTERVAL_1_4,
	DVBFE_GUARD_INTERVAL_AUTO
} dvbfe_guard_interval_t;

typedef enum dvbfe_hierarchy {
	DVBFE_HIERARCHY_NONE,
	DVBFE_HIERARCHY_1,
	DVBFE_HIERARCHY_2,
	DVBFE_HIERARCHY_4,
	DVBFE_HIERARCHY_AUTO
} dvbfe_hierarchy_t;

/**
 * Structure used to store and communicate frontend parameters.
 */
struct dvbfe_parameters {
	uint32_t frequency;
	dvbfe_spectral_inversion_t inversion;
	union {
		struct {
			uint32_t			symbol_rate;
			dvbfe_code_rate_t		fec_inner;
			dvbfe_polarization_t		polarization;
		} dvbs;

		struct {
			uint32_t			symbol_rate;
			dvbfe_code_rate_t		fec_inner;
			dvbfe_modulation_t		modulation;
		} dvbc;

		struct {
			dvbfe_bandwidth_t		bandwidth;
			dvbfe_code_rate_t		code_rate_HP;
			dvbfe_code_rate_t		code_rate_LP;
			dvbfe_modulation_t		constellation;
			dvbfe_transmit_mode_t		transmission_mode;
			dvbfe_guard_interval_t		guard_interval;
			dvbfe_hierarchy_t		hierarchy_information;
		} dvbt;

		struct {
			dvbfe_modulation_t		modulation;
		} atsc;
	} u;
};

/**
 * Mask of values used in the dvbfe_get_status() call.
 */
typedef enum dvbfe_status_mask {
	DVBFE_STATUS_FE                 = 0x01,
	DVBFE_STATUS_BER                = 0x02,
	DVBFE_STATUS_SIGNAL_STRENGTH    = 0x04,
	DVBFE_STATUS_SNR                = 0x08,
	DVBFE_STATUS_UNCORRECTED_BLOCKS = 0x10,
} dvbfe_status_mask_t;

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
 * Retrieve the type of frontend.
 *
 * @param fd FD opened with libdvbfe_open().
 * @return One of the DVBFE_TYPE_* values on success, or < 0 on failure
 */
extern int dvbfe_get_type(int fd);

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
extern int dvbfe_set_frontend(int fd, struct dvbfe_parameters *params);

/**
 * Read the current frontend tuning parameters from the hardware.
 *
 * @param fd FD opened with libdvbfe_open().
 * @param params Where to put the parameters.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbfe_get_frontend(int fd, struct dvbfe_parameters *params);

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
