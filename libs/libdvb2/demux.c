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

#include <dvb/demux.h>
#include <dvb/dvb.h>
#include <dvb/event.h>
#include <dvb/frontend.h>
#include <dvb/internal.h>
#include <dvb/device.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <linux/dvb/dmx.h>

#include <stdio.h>

static int demux_simple_enqueue_event(struct dvb_demux * demux,
				      enum dvb_demux_event_type type);

static int demux_simple_send_event(struct dvb_demux * demux,
				   enum dvb_demux_event_type type);

static void dvb_demux_notifier(struct dvb_notifier *,
			       const struct pollfd *);

static int next_section_entry(struct dvb_filter * filter);

struct dvb_demux {
	struct list_entry list;

	struct dvb_frontend * frontend;

	struct list_entry filters;
};

struct dvb_filter {
	struct list_entry list;

	enum dvb_filter_type type;

	struct dvb_demux * demux;

	int fd;
	uint8_t buf[4096]; // XXX - temporary
	struct dvb_notifier notifier;

	unsigned int paused	:1;

	int nb_entries;
	struct list_entry entries;
	struct dvb_filter_entry * cur_entry;
};

struct dvb_filter_entry {
	struct list_entry list;
	struct dvb_filter * filter;
};

struct dvb_pid_filter_entry {
	struct dvb_filter_entry head;
	struct dmx_pes_filter_params params;
};

struct dvb_section_filter_entry {
	struct dvb_filter_entry head;
	struct dmx_sct_filter_params params;
};

int dvb_open_demux(struct dvb_frontend * frontend, struct dvb_demux ** handle)
{
	struct dvb_demux * demux;

	if (frontend == NULL)
		return -EINVAL;

	demux = malloc(sizeof(struct dvb_demux));
	if (demux == NULL)
		return -ENOMEM;

	memset(demux, 0, sizeof(struct dvb_demux));

	LIST_ENTRY_INIT(&demux->filters);
	demux->frontend = frontend;

	list_append_entry(&frontend->demux, &demux->list);

	if (handle != NULL)
		*handle = demux;

	demux_simple_enqueue_event(demux, dvb_demux_open_event);
	return 0;
}

int dvb_close_demux(struct dvb_demux * demux)
{
	if (demux == NULL)
		return -EINVAL;

	demux_simple_send_event(demux, dvb_demux_close_event);
	/* XXX - free filters */
	free(demux);
	return 0;
}

int dvb_get_demux_event_data(struct dvb_event * event,
		             struct dvb_demux_event * data)
{
	if (event == NULL || data == NULL)
		return -EINVAL;

	if (dvb_get_event_type(event)      != dvb_demux_event)
	/* XXX && dvb_get_event_data_size(event) != sizeof(struct dvb_demux_event)) */
		return -EINVAL;

	memcpy(data, dvb_get_event_data(event), dvb_get_event_data_size(event));
	return 0;
}

int dvb_open_filter(struct dvb_demux * demux, enum dvb_filter_type type,
		    struct dvb_filter ** ptr)
{
	struct dvb_filter * filter;
	int fd;

	if (demux == NULL)
		return -EINVAL;

	if ((fd = dvb_open_device(demux->frontend->adapter, DVB_DEMUX, 0)) < 0)
		return dvb_error(errno, "dvb_open_device: %s\n", strerror(errno)); // XXX

	filter = malloc(sizeof(struct dvb_filter));
	if (filter == NULL)
		return -ENOMEM;

	memset(filter, 0, sizeof(struct dvb_filter));

	filter->demux = demux;
	filter->type = type;
	filter->fd = fd;

	filter->notifier.opaque = filter;
	filter->notifier.callback = dvb_demux_notifier;

	const struct pollfd pfd = {
		fd:	filter->fd,
		events: POLLIN,
		revents: 0,
	};

	dvb_add_notifier(demux->frontend->adapter->dvb, &pfd, &filter->notifier);

	list_append_entry(&demux->filters, &filter->list);

	if (ptr)
		*ptr = filter;

	return 0;
}

int dvb_close_filter(struct dvb_filter * filter)
{
	if (filter == NULL)
		return -EINVAL;

	list_remove_entry(&(filter->demux->filters), &filter->list);
	free(filter);

	return 0;
}

int dvb_pause_filter(struct dvb_filter * filter)
{
	int paused;

	if (filter == NULL)
		return -EINVAL;

	paused = filter->paused;
	filter->paused = 1;

	if (paused != filter->paused) {
		/* XXX: notify */
	}

	return 0;
}

int dvb_resume_filter(struct dvb_filter * filter)
{
	int paused;

	if (filter == NULL)
		return -EINVAL;

	paused = filter->paused;
	filter->paused = 0;

	if (paused != filter->paused) {
		/* XXX: notify */
	}

	return 0;
}

int dvb_pid_filter_add(dvb_filter_t * filter, int pid,
		       dvb_filter_entry_t ** ptr)
{
	struct dvb_pid_filter_entry * entry;
	int ret;

	if (filter->type != dvb_ts_filter &&
	    filter->type != dvb_pes_filter)
		return -EINVAL;

	/* This could be used if the dvb drivers change */
	if (filter->nb_entries > 0)
		return -EBUSY;

	entry = malloc(sizeof(struct dvb_pid_filter_entry));
	if (entry == NULL)
		return -ENOMEM;

	memset(entry, 0, sizeof(struct dvb_pid_filter_entry));
	entry->head.filter = filter;

	entry->params.pid = pid;
	entry->params.input = DMX_IN_FRONTEND;
	entry->params.output = DMX_OUT_TS_TAP;
	entry->params.pes_type = DMX_PES_OTHER;
	entry->params.flags = DMX_IMMEDIATE_START;

	if ((ret = ioctl(filter->fd, DMX_SET_PES_FILTER, &entry->params)) < 0)
		return dvb_error(errno, "DMX_SET_FILTER: %s", strerror(errno));

	++filter->nb_entries;
	list_append_entry(&filter->entries, &entry->head.list);

	/* device */

	if (ptr)
		*ptr = &entry->head;

	return 0;
}

int dvb_section_filter_add(dvb_filter_t * filter, int pid, uint8_t table_id,
			   dvb_filter_entry_t ** ptr)
{
	struct dvb_section_filter_entry * entry;

	if (filter->type != dvb_section_filter)
		return -EINVAL;

	entry = malloc(sizeof(struct dvb_section_filter_entry));
	if (entry == NULL)
		return -ENOMEM;

	memset(entry, 0, sizeof(struct dvb_section_filter_entry));
	entry->head.filter = filter;

	entry->params.pid = pid;
	/* XXX - entry->params.table_id = table_id; */
	entry->params.timeout = 1000;
	entry->params.flags = DMX_CHECK_CRC | DMX_IMMEDIATE_START;

	++filter->nb_entries;
	list_append_entry(&filter->entries, &entry->head.list);

	if (filter->nb_entries)
		next_section_entry(filter);

	if (ptr)
		*ptr = &entry->head;

	return 0;
}

int dvb_filter_remove(dvb_filter_entry_t * entry)
{
	struct dvb_filter * filter = entry->filter;

	list_remove_entry(&filter->entries, &entry->list);
	free(entry);

	--filter->nb_entries;
	/* XXX : close device */
	return 0;
}


/*****************************************************************************/

static inline int section_event(struct dvb_filter * filter,
				uint8_t * buf, int len)
{
	struct dvb_event * event;
	struct dvb_demux_event * event_data;

	event = dvb_alloc_event(dvb_demux_event,
			        sizeof(struct dvb_demux_event));

	if (event == NULL)
		return -ENOMEM;

	event_data = dvb_get_event_data(event);
	event_data->demux = filter->demux;
	event_data->filter = filter;
	event_data->type = dvb_demux_data_event;
	/* XXX: event_data->pid = pid; */
	event_data->len = len;
	event_data->data = buf;

	dvb_send_event(filter->demux->frontend->adapter->dvb, event);
	return 0;
}

static int next_section_entry(struct dvb_filter * filter)
{
	struct list_entry * pos;
	struct dvb_section_filter_entry * entry;
	int ret;

	if (filter->type != dvb_section_filter)
		return -EINVAL;

	if (filter->nb_entries <= 0)
		return -EINVAL;

	for (pos = filter->entries.next; pos; pos = pos->next)
	{
		entry = list_get_entry(pos, struct dvb_section_filter_entry, head.list);
		if (&entry->head == filter->cur_entry)
			break;
	}

	if (pos && pos->next)
		pos = pos->next;
	else
		pos = filter->entries.next;

	entry = list_get_entry(pos, struct dvb_section_filter_entry, head.list);

#if 0
	fprintf(stderr, "nb_entries %i filter_entry %p pid %i\n",
		filter->nb_entries, entry, entry->params.pid);
#endif

	/* XXX - need to start a timeout here, generic timers? */

	if ((ret = ioctl(filter->fd, DMX_SET_FILTER, &entry->params)) < 0) {
		fprintf(stderr, "DMX_SET_FILTER: %s\n", strerror(errno));
		return ret;
	}

	filter->cur_entry = &entry->head;
	return 0;
}

static int handle_section(struct dvb_filter * filter, uint8_t * buf, int ret)
{
	section_event(filter, filter->buf, ret);
	next_section_entry(filter);
	return 0;
}

static void dvb_demux_notifier(struct dvb_notifier * notifier,
			       const struct pollfd * pfd)
{
	struct dvb_filter * filter = notifier->opaque;
	int ret;

	if (filter == NULL || filter->fd < 0)
		return;

	if (pfd->revents & POLLIN || pfd->revents & POLLPRI) {

		ret = read(filter->fd, filter->buf, sizeof(filter->buf));

		if (ret <= 0)
			return;

		switch(filter->type) {
			case dvb_section_filter:
				handle_section(filter, filter->buf, ret);
				return;
			default:
				return;
		}
	}
}

static struct dvb_event * demux_create_event(struct dvb_demux * demux,
					     enum dvb_demux_event_type type)
{
	struct dvb_event * event;
	struct dvb_demux_event * ed;

	event = dvb_alloc_event(dvb_demux_event,
			        sizeof(struct dvb_demux_event));
	if (event == NULL)
		return NULL;

	ed = dvb_get_event_data(event);
	ed->demux = demux;
	ed->type = type;

	return event;
}

static int demux_simple_enqueue_event(struct dvb_demux * demux,
				      enum dvb_demux_event_type type)
{
	struct dvb_event * event = demux_create_event(demux, type);
	if (event == NULL)
		return -ENOMEM;
	dvb_enqueue_event(demux->frontend->adapter->dvb, event);
	return 0;
}

static int demux_simple_send_event(struct dvb_demux * demux,
				   enum dvb_demux_event_type type)
{
	struct dvb_event * event = demux_create_event(demux, type);
	if (event == NULL)
		return -ENOMEM;
	dvb_send_event(demux->frontend->adapter->dvb, event);
	return 0;
}

#if 0
int dvb_add_pid(struct dvb_filter * filter, int pid)
{
	struct list_entry * pos;
	struct dvb_demux * demux;
	struct dvb_device * device;
	int fd, ret;

	if (filter == NULL || filter->demux == NULL)
		return -EINVAL;

	demux = filter->demux;

	for (pos = demux->devices.next; pos != NULL; pos = pos->next) {
		device = list_get_entry(pos, struct dvb_device, list);

		if ((device->section != 1) && (device->pid == pid)) {
			++device->ref;
			return 0;
		}
	}

	if ((fd = dvb_open_device(demux->frontend->adapter, DVB_DEMUX, 0)) < 0)
		return dvb_error(errno, "dvb_open_device: %s\n", strerror(errno));

	struct dmx_pes_filter_params params;
	memset(&params, 0, sizeof(struct dmx_pes_filter_params));
	params.pid = pid;
	params.input = DMX_IN_FRONTEND;
	params.output = DMX_OUT_TS_TAP;
	params.pes_type = DMX_PES_OTHER;
	params.flags = DMX_IMMEDIATE_START;

	if ((ret = ioctl(fd, DMX_SET_PES_FILTER, &params)) < 0)
		return dvb_error(errno, "DMX_SET_FILTER: %s", strerror(errno));

	if ((device = malloc(sizeof(struct dvb_device))) == NULL)
		return -ENOMEM;

	memset(device, 0, sizeof(struct dvb_device));
	device->demux = demux;
	device->section = 0;
	device->pid = pid;
	device->fd = fd;

	device->notifier.opaque = device;
	device->notifier.callback = dvb_demux_notifier;

	const struct pollfd pfd = {
		fd:	device->fd,
		events: POLLIN,
		revents: 0,
	};

	dvb_add_notifier(filter->demux->frontend->adapter->dvb, &pfd, &device->notifier);

	list_append_entry(&demux->devices, &device->list);

	return 0;
}

int dvb_add_section_pid(struct dvb_filter * filter, int pid)
{
	struct list_entry * pos;
	struct dvb_demux * demux;
	struct dvb_device * device;
	int fd, ret;

	if (filter == NULL || filter->demux == NULL)
		return -EINVAL;

	demux = filter->demux;

	for (pos = demux->devices.next; pos != NULL; pos = pos->next) {
		device = list_get_entry(pos, struct dvb_device, list);

		if ((device->section != 0) && (device->pid == pid)) {
			++device->ref;
			return 0;
		}
	}

	if ((fd = dvb_open_device(demux->frontend->adapter, DVB_DEMUX, 0)) < 0) {
		fprintf(stderr, "dvb_open_device: %s\n", strerror(errno));
		return -errno;
	}

	struct dmx_sct_filter_params params;
	memset(&params, 0, sizeof(struct dmx_sct_filter_params));
	params.pid = pid;
	params.flags = DMX_CHECK_CRC | DMX_IMMEDIATE_START;

	if ((ret = ioctl(fd, DMX_SET_FILTER, &params)) < 0) {
		fprintf(stderr, "DMX_SET_FILTER: %s\n", strerror(errno));
		return -errno;
	}

	if ((device = malloc(sizeof(struct dvb_device))) == NULL)
		return -ENOMEM;

	memset(device, 0, sizeof(struct dvb_device));

	device->demux = demux;
	device->section = 1;
	device->pid = pid;
	device->fd = fd;

	device->notifier.opaque = device;
	device->notifier.callback = dvb_demux_notifier;

	const struct pollfd pfd = {
		fd:	device->fd,
		events: POLLIN,
		revents: 0,
	};

	dvb_add_notifier(filter->demux->frontend->adapter->dvb, &pfd, &device->notifier);

	list_append_entry(&demux->devices, &device->list);

	return 0;
}

static int dvb_demux_read_device(struct dvb_device * device)
{
	int ret;

	if (device->fd < 0)
		return -EINVAL;

	ret = read(device->fd, device->buf, sizeof(device->buf));
	if (ret > 0)
	{
		if (device->section) {
			section_event(device->demux, device->pid, device->buf, ret);
		} else {
			fprintf(stderr, "0x%.4x: ret %i\n", device->pid, ret);
		}
	}

	return 0;
}

int dvb_read_filter(struct dvb_filter * filter)
{
	struct dvb_demux * demux;
	struct dvb_device * device;
	struct list_entry * pos;

	if (filter == NULL || filter->demux == NULL)
		return -EINVAL;

	demux = filter->demux;

	pos = demux->devices.next;
	if (pos) {
		device = list_get_entry(pos, struct dvb_device, list);

		dvb_demux_read_device(device);
		list_remove_entry(&demux->devices, pos);
		list_append_entry(&demux->devices, pos);
		return 0;
	}

	return -EAGAIN;
}
#endif


#if 0 //unused
static void read_demux(void * opaque)
{
	struct dvb_demux * demux = opaque;
	struct list_entry * pos;
	struct dvb_filter * filter;

	for (pos = demux->filters.next; pos != NULL; pos = pos->next) {
		filter = list_get_entry(pos, struct dvb_filter, list);

		if (!dvb_read_filter(filter)) {
			list_remove_entry(&demux->filters, pos);
			list_append_entry(&demux->filters, pos);
			break;
		}
	}
}

static void dvb_read_dvr(void * opaque)
{
	/* XXX: need a ring here */
}
#endif

