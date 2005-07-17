/**
 * Really simple linked list library.
 *
 * Copyright (c) 2005 by Andrew de Quincey <adq_dvb@lidskialf.net>
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

#ifndef LIST_H
#define LIST_H

#include <stddef.h>

struct list_entry
{
        struct list_entry* next;
};

#define LIST_STATIC_INIT { next: NULL, }

#define LIST_ENTRY_INIT(__ENTRY) \
	(__ENTRY)->next = NULL

#define list_get_entry(__ENTRY, __STRUCT, __ELEMENT) \
	(__STRUCT*) (((char*) __ENTRY) - offsetof(__STRUCT, __ELEMENT))

static inline void list_append_entry(struct list_entry* list, struct list_entry* toappend)
{
	while(list->next)
		list = list->next;
	list->next = toappend;
	toappend->next = NULL;
}

static inline void list_remove_entry(struct list_entry* list, struct list_entry* toremove)
{
	while(list) {
		if (list->next == toremove) {
			list->next = toremove->next;
			return;
		}
		list = list->next;
	}
}

#endif
