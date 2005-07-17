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

#include <dvb/dvb.h>
#include <dvb/internal.h>
#include <dvb/frontend.h>

#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>

static struct list_entry handles = LIST_STATIC_INIT;
static char dvb_error_str[1024] = "";

const char * dvb_strerror(int error)
{
	return dvb_error_str;
}

int dvb_error(int error, const char * fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
#if 1
	vfprintf(stderr, fmt, ap);
#endif
	vsnprintf(dvb_error_str, sizeof(dvb_error_str), fmt, ap);
	va_end(ap);

	return error;
}

#include <dvb/plugin.h>

int dvb_create_handle(struct dvb ** dvbptr)
{
	struct dvb * dvb = malloc(sizeof(struct dvb));

	if (dvb == NULL)
		return -ENOMEM;

	memset(dvb, 0, sizeof(struct dvb));

	LIST_ENTRY_INIT(&dvb->adapters);
	list_append_entry(&handles, &dvb->list);

	*dvbptr = dvb;
	return 0;
}

int dvb_close_handle(struct dvb * dvb)
{
	list_remove_entry(&handles, &dvb->list);
	free(dvb);
	/* XXX: close */
	return 0;
}

static inline void probe_adapter(struct dvb_adapter * adapter)
{
	int i;

	for (i = 0; i < 16; ++i)
		dvb_probe_frontend(adapter, i, NULL);
}

int dvb_open_adapter(struct dvb * dvb, const char * dev,
		     struct dvb_adapter ** ptr)
{
	struct dvb_adapter * adapter;
	int adapternum;

	if (dvb == NULL)
		return dvb_error(-EINVAL, "Invalid dvb handle.");

	if (dev == NULL)
		return dvb_error(-EINVAL, "Invalid device: %s.", dev);

	if (sscanf(dev, "dvb:%i", &adapternum) != 1) {
		char dummy;
		if (sscanf(dev, "DVB%c%i", &dummy, &adapternum) != 2)
			return dvb_error(-EINVAL, "Invalid device: %s.", dev);
	}

	adapter = malloc(sizeof(struct dvb_adapter));
	if (adapter == NULL)
		return -ENOMEM;

	memset(adapter, 0, sizeof(struct dvb_adapter));

	LIST_ENTRY_INIT(&adapter->frontends);
	adapter->dvb = dvb;
	adapter->num = adapternum;

	list_append_entry(&dvb->adapters, &adapter->list);
	*ptr = adapter;

	probe_adapter(adapter);
	return 0;
}

int dvb_close_adapter(struct dvb_adapter * adapter)
{
	dvb_close_frontends(adapter);

	free(adapter);
	return 0;
}

