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

#ifndef _DVB_PLUGIN_H_
#define _DVB_PLUGIN_H_

#include <dvb/internal.h>

enum dvb_plugin_type {
	dvb_ci_plugin_type	= 0x01,
	dvb_adapter_plugin_type	= 0x02,
};

struct dvb_plugin {
	struct dvb_plugin * next;

	char magic[10];
	const char * name;
	enum dvb_plugin_type type;
	void * priv;
};

struct dvb_frontend;
struct dvb_ci;
struct dvb_ci_apdu;

struct dvb_ci_plugin {
	int (*probe) (struct dvb_frontend *);

	int (*init) (struct dvb_ci *);
	int (*exit) (struct dvb_ci *);
};

#include <stdio.h>

#define __init __attribute__((constructor))
#define __fini __attribute__((destructor))

void register_plugin(struct dvb_plugin *);
void deregister_plugin(struct dvb_plugin *);

#define DECLARE_PLUGIN(_name, _type, _priv) \
	struct dvb_plugin dvb_plugin = { \
		magic: "libdvb2", \
		next: 0, \
		name: _name, \
		type: _type, \
		priv: _priv, \
	}; \
	__init void plugin_init(void) { register_plugin(&dvb_plugin); } \
	__fini void plugin_exit(void) { deregister_plugin(&dvb_plugin); }

int dvb_find_plugins(struct dvb * dvb, enum dvb_plugin_type type,
		     const struct dvb_plugin ** first);

#endif

