/*
 * libdvb2 - dvb helper and wrapper library
 *
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

#ifndef _DVB_PROGRAM_H_
#define _DVB_PROGRAM_H_

#include <dvb/dvb.h>
#include <dvb/list.h>
#include <ci/ca.h>

#include <si/mpeg/pmt_section.h>

struct dvb_program {
	struct list_entry list;

	/* duplicated in pmt */
	int program_number;
	int pmtpid;

	dvb_filter_entry_t * pmt_filter_entry;
	struct dvb_ca_program * ca_program;

	struct mpeg_pmt_section * pmt; /* table */
	struct list_entry streams;
};

#endif

