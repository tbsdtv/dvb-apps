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
#include <dvb/event.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>

struct dvb_event_data {
	struct list_entry event_hooks;
};

struct event_hook {
	struct list_entry list;

	struct dvb * dvb;

	dvb_event_hook_t func;
	void * opaque;

	struct list_entry events;
};

struct event_entry {
	struct list_entry list;
	struct dvb_event * event;
};

struct dvb_event * dvb_alloc_event(enum dvb_event_type type, size_t size)
{
	struct dvb_event * ret;

	size += sizeof(struct dvb_event);

	if ((ret = malloc(size)) == NULL)
		return NULL;

	memset(ret, 0, size);
	ret->ref = 0;
	ret->size = size;
	ret->type = type;

	return ret;
}

void dvb_free_event(struct dvb_event * event)
{
	if (--event->ref > 0)
		return;

	free(event);
}

void __free_entry(struct event_entry * entry)
{
	dvb_free_event(entry->event);
	free(entry);
}

enum dvb_event_type dvb_get_event_type(struct dvb_event *event)
{
	return event->type;
}

static inline int alloc_event_data(struct dvb * dvb)
{
	if (dvb->event == NULL) {
		dvb->event = malloc(sizeof(struct dvb_event_data));
		if (dvb->event == NULL)
			return -ENOMEM;

		memset(dvb->event, 0, sizeof(struct dvb_event_data));
		LIST_ENTRY_INIT(&dvb->event->event_hooks);
	}

	return 0;
}

int dvb_enqueue_event(struct dvb * dvb, struct dvb_event * event)
{
	struct dvb_event_data * data;
	struct list_entry * pos;

	if (dvb == NULL || event == NULL)
		return -EINVAL;

	if (alloc_event_data(dvb) < 0)
		return -ENOMEM;

	data = dvb->event;

	for (pos = data->event_hooks.next; pos != NULL; pos = pos->next) {
		struct event_entry * entry;
		struct event_hook * hook = list_get_entry(pos, struct event_hook, list);

		entry = malloc(sizeof(struct event_entry));
		if (entry == NULL)
			return -ENOMEM;

		++event->ref;
		entry->event = event;
		list_append_entry(&hook->events, &entry->list);
	}

	return 0;
}

static inline int __send_event(struct event_hook * hook)
{
	struct list_entry * pos;

	if (hook == NULL)
		return -EINVAL;

 	pos = hook->events.next;
 	if (pos != NULL) {
 		struct event_entry * entry = list_get_entry(pos, struct event_entry, list);

		hook->func(hook->dvb, entry->event, hook->opaque);
		list_remove_entry(&hook->events, pos);
		__free_entry(entry);
	}

	return 0;
}

int dvb_send_event(struct dvb * dvb, struct dvb_event * event)
{
	struct dvb_event_data * data;
	struct list_entry * pos;

	if (dvb == NULL || event == NULL)
		return -EINVAL;

	if (alloc_event_data(dvb) < 0)
		return -ENOMEM;

	data = dvb->event;

	/* XXX: lock thread */
	for (pos = data->event_hooks.next; pos != NULL; pos = pos->next) {
		struct event_hook * hook = list_get_entry(pos, struct event_hook, list);

		hook->func(hook->dvb, event, hook->opaque);
	}

	dvb_free_event(event);
	return 0;
}

int dvb_add_event_hook(struct dvb * dvb,
		       dvb_event_hook_t func, void * opaque)
{
	struct dvb_event_data * data;

	if (dvb == NULL || func == NULL)
		return -EINVAL;

	if (alloc_event_data(dvb) < 0)
		return -ENOMEM;

	data = dvb->event;

	struct event_hook * hook = malloc(sizeof(struct event_hook));
	if (hook == NULL)
		return -ENOMEM;

	LIST_ENTRY_INIT(&hook->events);

	hook->dvb = dvb;
	hook->func = func;
	hook->opaque = opaque;

	list_append_entry(&data->event_hooks, &hook->list);

	return 0;
}

int dvb_remove_event_hook(struct dvb * dvb,
			  dvb_event_hook_t func)
{
	struct dvb_event_data * data;
	struct list_entry * pos, * next;

	if (dvb == NULL || dvb->event == NULL || func == NULL)
		return -EINVAL;

	data = dvb->event;

	for (pos = data->event_hooks.next; pos != NULL; pos = next) {
		next = pos->next;
		struct event_hook * hook = list_get_entry(pos, struct event_hook, list);

		if (hook->func == func) {
			list_remove_entry(&data->event_hooks, pos);
			free(hook);
		}
	}

	return 0;
}

int dvb_send_one_event(struct dvb * dvb)
{
	struct dvb_event_data * data;
	struct list_entry * pos;

	if (dvb == NULL || dvb->event == NULL)
		return -EINVAL;

	data = dvb->event;

	for (pos = data->event_hooks.next; pos != NULL; pos = pos->next) {
		struct event_hook * hook = list_get_entry(pos, struct event_hook, list);

		if (__send_event(hook) == 0) {
			list_remove_entry(&data->event_hooks, pos);
			list_append_entry(&data->event_hooks, pos);
			break;
		}
	}

	return 0;
}

int __flush_events(struct event_hook * hook)
{
	struct list_entry * pos, * next;
	struct event_entry * entry;

	if (hook == NULL)
		return -EINVAL;

 	for (pos = hook->events.next; pos != NULL; pos = next) {
 		next = pos->next;
 		entry = list_get_entry(pos, struct event_entry, list);

		hook->func(hook->dvb, entry->event, hook->opaque);
		__free_entry(entry);
		list_remove_entry(&hook->events, pos);
	}

	return 0;
}

int __clear_events(struct event_hook * hook)
{
	struct list_entry * pos, * next;
	struct event_entry * entry;

	if (hook == NULL)
		return -EINVAL;

 	for (pos = hook->events.next; pos != NULL; pos = next) {
 		next = pos->next;
 		entry = list_get_entry(pos, struct event_entry, list);

		list_remove_entry(&hook->events, pos);
	}

	return 0;
}

int dvb_flush_events(struct dvb * dvb)
{
	struct dvb_event_data * data;
	struct list_entry * pos;

	if (dvb == NULL || dvb->event == NULL)
		return -EINVAL;

	data = dvb->event;

	for (pos = data->event_hooks.next; pos != NULL; pos = pos->next) {
		struct event_hook * hook = list_get_entry(pos, struct event_hook, list);

		__flush_events(hook);
	}

	return 0;
}

int dvb_clear_events(struct dvb * dvb)
{
	struct dvb_event_data * data;
	struct list_entry * pos;

	if (dvb == NULL || dvb->event == NULL)
		return -EINVAL;

	data = dvb->event;

	for (pos = data->event_hooks.next; pos != NULL; pos = pos->next) {
		struct event_hook * hook = list_get_entry(pos, struct event_hook, list);

		__clear_events(hook);
	}

	return 0;
}

