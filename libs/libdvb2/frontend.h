/*
 * libdvb2 - dvb helper and wrapper library
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
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

#ifndef _DVB_FRONTEND_H_
#define _DVB_FRONTEND_H_

#include <dvb/dvb.h>
#include <dvb/internal.h>
#include <dvb/thread.h>
#include <linux/dvb/frontend.h>
#include <stdint.h>


struct dvb_frontend {
	struct list_entry list;

	struct dvb_adapter * adapter;

	struct list_entry demux;
	struct list_entry ci;

	struct dvb_task * update_task;

	int num;
	int fd;
	char * name;
	unsigned int lock	: 1;

	unsigned int readonly	: 1;
};

#define DVB_FRONTEND_VALUE_STATUS             0x01
#define DVB_FRONTEND_VALUE_BER                0x02
#define DVB_FRONTEND_VALUE_SIGNAL_STRENGTH    0x04
#define DVB_FRONTEND_VALUE_SNR                0x08
#define DVB_FRONTEND_VALUE_UNCORRECTED_BLOCKS 0x10

struct dvb_frontend_values {
	unsigned int signal	: 1;
	unsigned int carrier	: 1;
	unsigned int viterbi	: 1;
	unsigned int sync	: 1;
	unsigned int lock	: 1;
	uint32_t ber;
	uint16_t signal_strength;
	uint16_t snr;
	uint32_t ucblocks;
};

/**
 * Probe a frontend of an adapter.
 *
 * @param adapter The adapter concerned.
 * @param device The device name of the frontend to probe.
 * @param frontend Where to put resulting frontend structure.
 * @return 0 on success, non-zero on failure.
 */
int dvb_probe_frontend(struct dvb_adapter * adapter, int num,
		       struct dvb_frontend ** frontend);

/**
 * Find a frontend on adapter by number.
 *
 * @param adapter Adapter to search.
 * @param num Frontend number to find.
 * @return NULL if the frontend was not found.
 */
struct dvb_frontend * dvb_find_frontend(struct dvb_adapter * adapter, int num);

/**
 * Close all frontends associated with an adapter.
 *
 * @param adapter The adapter concerned.
 * @return 0 on success, non-zero on failure.
 */
int dvb_close_frontends(struct dvb_adapter * adapter);

/**
 * Updates frontend information for all frontends associated with an adapter.
 *
 * @param adapter The adapter concerned.
 * @return 0 on success, non-zero on failure.
 */
int dvb_update_frontends(struct dvb_adapter * adapter);

/**
 * Read value(s) from the frontend.
 *
 * @param frontend The frontend concerned.
 * @param value_flags ORred mask of DVB_FRONTEND_VALUE_* above giving those to read.
 * @param dest Where to put the resulting values.
 *
 * @return ORed mask of resulting valid values in dest.
 */
int dvb_frontend_read_value(struct dvb_frontend * frontend, int value_flags,
			    struct dvb_frontend_values * dest);

/**
 * Execute a VDR-style LNB command.
 *
 * @param frontend The frontend concerned.
 * @param lnbcommand The command to execute.
 * @return 0 on success, error code on failure.
 */
int dvb_frontend_lnb_command(struct dvb_frontend * frontend, char * lnbcommand);

/**
 * Read a DISEQC response from the frontend.
 *
 * @param frontend The frontend concerned.
 * @param timeout Timeout for DISEQC response.
 * @param buf Buffer to store response in.
 * @param len Number of bytes in buffer.
 * @return >= 0 on success (number of received bytes), error code on failure.
 */
int dvb_frontend_read_diseqc_response(struct dvb_frontend * frontend, int timeout,
				      uint8_t * buf, unsigned int len);

/**
 * Read the frontend settings from the hardware.
 *
 * @param frontend The frontend concerned.
 * @param params Where to read the parameters to.
 * @return 0 on success, error code on failure.
 */
int dvb_frontend_get(struct dvb_frontend * frontend,
		     struct dvb_frontend_parameters * params);

/**
 * Sets the frontend parameters.
 *
 * @param frontend The frontend concerned.
 * @param params Frontend settings to use.
 * @return 0 on success, error code on failure.
 */
int dvb_frontend_set(struct dvb_frontend * frontend,
		     struct dvb_frontend_parameters * params);

#endif
