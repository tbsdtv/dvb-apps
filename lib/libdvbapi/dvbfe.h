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

typedef enum dvbfe_dvbt_const {
	DVBFE_DVBT_CONST_QPSK,
	DVBFE_DVBT_CONST_QAM_16,
	DVBFE_DVBT_CONST_QAM_32,
	DVBFE_DVBT_CONST_QAM_64,
	DVBFE_DVBT_CONST_QAM_128,
	DVBFE_DVBT_CONST_QAM_256,
	DVBFE_DVBT_CONST_AUTO
} dvbfe_dvbt_const_t;

typedef enum dvbfe_dvbc_mod {
	DVBFE_DVBC_MOD_QAM_16,
	DVBFE_DVBC_MOD_QAM_32,
	DVBFE_DVBC_MOD_QAM_64,
	DVBFE_DVBC_MOD_QAM_128,
	DVBFE_DVBC_MOD_QAM_256,
	DVBFE_DVBC_MOD_AUTO,
} dvbfe_dvbc_mod_t;

typedef enum dvbfe_atsc_mod {
	DVBFE_ATSC_MOD_QAM_64,
	DVBFE_ATSC_MOD_QAM_256,
	DVBFE_ATSC_MOD_VSB_8,
	DVBFE_ATSC_MOD_VSB_16,
	DVBFE_ATSC_MOD_AUTO
} dvbfe_atsc_mod_t;

typedef enum dvbfe_dvbt_transmit_mode {
	DVBFE_DVBT_TRANSMISSION_MODE_2K,
	DVBFE_DVBT_TRANSMISSION_MODE_8K,
	DVBFE_DVBT_TRANSMISSION_MODE_AUTO
} dvbfe_dvbt_transmit_mode_t;

typedef enum dvbfe_dvbt_bandwidth {
	DVBFE_DVBT_BANDWIDTH_8_MHZ,
	DVBFE_DVBT_BANDWIDTH_7_MHZ,
	DVBFE_DVBT_BANDWIDTH_6_MHZ,
	DVBFE_DVBT_BANDWIDTH_AUTO
} dvbfe_dvbt_bandwidth_t;

typedef enum dvbfe_dvbt_guard_interval {
	DVBFE_DVBT_GUARD_INTERVAL_1_32,
	DVBFE_DVBT_GUARD_INTERVAL_1_16,
	DVBFE_DVBT_GUARD_INTERVAL_1_8,
	DVBFE_DVBT_GUARD_INTERVAL_1_4,
	DVBFE_DVBT_GUARD_INTERVAL_AUTO
} dvbfe_dvbt_guard_interval_t;

typedef enum dvbfe_dvbt_hierarchy {
	DVBFE_DVBT_HIERARCHY_NONE,
	DVBFE_DVBT_HIERARCHY_1,
	DVBFE_DVBT_HIERARCHY_2,
	DVBFE_DVBT_HIERARCHY_4,
	DVBFE_DVBT_HIERARCHY_AUTO
} dvbfe_dvbt_hierarchy_t;

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
			dvbfe_dvbc_mod_t		modulation;
		} dvbc;

		struct {
			dvbfe_dvbt_bandwidth_t		bandwidth;
			dvbfe_code_rate_t		code_rate_HP;
			dvbfe_code_rate_t		code_rate_LP;
			dvbfe_dvbt_const_t		constellation;
			dvbfe_dvbt_transmit_mode_t	transmission_mode;
			dvbfe_dvbt_guard_interval_t	guard_interval;
			dvbfe_dvbt_hierarchy_t		hierarchy_information;
		} dvbt;

		struct {
			dvbfe_atsc_mod_t		modulation;
		} atsc;
	} u;
};

/**
 * Mask of values used in the dvbfe_get_info() call.
 */
typedef enum dvbfe_info_mask {
	DVBFE_INFO_LOCKSTATUS			= 0x01,
	DVBFE_INFO_FEPARAMS			= 0x02,
	DVBFE_INFO_BER				= 0x04,
	DVBFE_INFO_SIGNAL_STRENGTH		= 0x08,
	DVBFE_INFO_SNR				= 0x10,
	DVBFE_INFO_UNCORRECTED_BLOCKS		= 0x20,
} dvbfe_info_mask_t;

/**
 * Structure containing values used by the dvbfe_get_info() call.
 */
struct dvbfe_info {
	dvbfe_type_t type;			/* always retrieved */
	const char *name;			/* always retrieved */
	unsigned int signal     : 1;		/* } DVBFE_INFO_LOCKSTATUS */
	unsigned int carrier    : 1;		/* } */
	unsigned int viterbi    : 1;		/* } */
	unsigned int sync       : 1;		/* } */
	unsigned int lock       : 1;		/* } */
	struct dvbfe_parameters feparams;	/* DVBFE_INFO_FEPARAMS */
	uint32_t ber;				/* DVBFE_INFO_BER */
	uint16_t signal_strength;		/* DVBFE_INFO_SIGNAL_STRENGTH */
	uint16_t snr;				/* DVBFE_INFO_SNR */
	uint32_t ucblocks;			/* DVBFE_INFO_UNCORRECTED_BLOCKS */
};

/**
 * Frontend handle datatype.
 */
struct dvbfe_handle;

/**
 * Open a DVB frontend.
 *
 * @param adapter DVB adapter ID.
 * @param frontend Frontend ID of that adapter to open.
 * @param readonly If 1, frontend will be opened in readonly mode only.
 * @return A handle on success, or NULL on failure.
 */
extern struct dvbfe_handle *dvbfe_open(int adapter, int frontend, int readonly);

/**
 * Close a DVB frontend.
 *
 * @param fehandle Handle opened with dvbfe_open().
 */
extern void dvbfe_close(struct dvbfe_handle *handle);

/**
 * Set the frontend tuning parameters.
 *
 * @param fehandle Handle opened with dvbfe_open().
 * @param params Params to set.
 * @param timeout <0 => wait forever for lock. 0=>return immediately, >0=>
 * number of milliseconds to wait for a lock.
 * @return 0 on locked (or if timeout==0 and everything else worked), or
 * nonzero on failure (including no lock).
 */
extern int dvbfe_set(struct dvbfe_handle *fehandle, struct dvbfe_parameters *params, int timeout);

/**
 * Call this function regularly from a loop to maintain the frontend lock.
 *
 * @param fehandle Handle opened with dvbfe_open().
 */
extern void dvbfe_poll(struct dvbfe_handle *fehandle);

/**
 * Retrieve information about the frontend.
 *
 * @param fehandle Handle opened with dvbfe_open().
 * @param querymask ORed bitmask of desired DVBFE_INFO_* values.
 * @param result Where to put the retrieved results.
 * @return ORed bitmask of DVBFE_INFO_* indicating which values were read successfully.
 */
extern int dvbfe_get_info(struct dvbfe_handle *fehandle, dvbfe_info_mask_t querymask, struct dvbfe_info *result);

/**
 * Execute a DISEQC command string.
 *
 * A diseqc command consists of a sequence of the following codes, separated by
 * whitespace:
 * Simple commands:
 * t         - turn 22kHz tone off.
 * T         - turn 22kHz tone on.
 * _         - set voltage to 0v (i.e. off).
 * v         - set voltage to 13v.
 * V         - set voltage to 18v.
 * +         - Enable high LNB voltage.
 * -         - Disable high LNB voltage.
 * A         - send DISEQC mini command A.
 * B         - send DISEQC mini command B.
 * Wii       - Delay for ii milliseconds.
 *
 * Extended commands:
 * .dishnetworks(<value>) - Send a dish networks legacy command <value>
 * .D(<value> ...)   - Send a raw diseqc master command. The command may be up
 * 	to 6 bytes long.
 * .Dband(<addr> <lo|hi>) - Set frequency band hi or lo.
 * .Dpolarisation(<addr> <V|H|L|R>) - Set polarisation.
 * .Dsatellite_position(<addr> <A|B>) - Set "satellite position" input switch.
 * .Dswitch_option(<addr> <A|B>) - Set "switch option" input switch.
 * .Dport_group(<addr> <0|1> <value>) - Set port group 0 or 1 setting.
 * .Dgoto_preset(<addr> <index>) - Set a positioner to a preset index (integer)
 * .Dgoto_angle(<addr> <angle>) - Set a positioner to a given angle
 * (e.g. 49.6). The angle may range between -180 to 496. It may include a
 * fractional part.
 *
 * All integer values use standard notation - no prefix=>decimal, 0x=>hex etc.
 *
 * Set <addr> to 0x10 if you just have a simple DISEQC setup (e.g. one switch). See
 * the DISEQC specification at http://www.eutelsat.org/ for full information.
 *
 * Comments begin with '#' - any characters after this will be ignored
 * to the end of the line.
 *
 * Examples:
 * S-19.2E  11700000 V  9750000  t v W15 .Dport_group(0x10 0 0xf0) W15 A W15 t
 * S-19.2E  99999999 V 10600000  t v W15 .Dport_group(0x10 0 0xf1) W15 A W15 T
 * S-19.2E  11700000 H  9750000  t V W15 .Dport_group(0x10 0 0xf2) W15 A W15 t
 * S-19.2E  99999999 H 10600000  t V W15 .Dport_group(0x10 0 0xf3) W15 A W15 T
 *
 * @param fehandle Handle opened with dvbfe_open().
 * @param command Command to execute.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbfe_diseqc_command(struct dvbfe_handle *fehandle, char *command);

/**
 * Read a DISEQC response from the frontend.
 *
 * @param fehandle Handle opened with dvbfe_open().
 * @param timeout Timeout for DISEQC response.
 * @param buf Buffer to store response in.
 * @param len Number of bytes in buffer.
 * @return >= 0 on success (number of received bytes), <0 on failure.
 */
extern int dvbfe_diseqc_read(struct dvbfe_handle *fehandle, int timeout, unsigned char *buf, unsigned int len);

#ifdef __cplusplus
}
#endif

#endif // LIBDVBFE_H
