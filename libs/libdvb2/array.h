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

#ifndef _DVB_ARRAY_H_
#define _DVB_ARRAY_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct ptr_array {
	int size;
	void ** ptrs;
};

#define DECLARE_PTR_ARRAY(name) \
	struct ptr_array name = { 0, NULL }

#define INIT_PTR_ARRAY(array) \
	(array)->size = 0; \
	(array)->ptrs = NULL;

static inline int ptr_array_empty(struct ptr_array * array)
{
	return array->size == 0;
}

static inline int ptr_array_clear(struct ptr_array * array)
{
	free(array->ptrs);
	INIT_PTR_ARRAY(array);
	return 0;
}

static inline int ptr_array_add(struct ptr_array * array, void * ptr)
{
	void ** ptrs = array->ptrs;
	int i, count = array->size / sizeof(void*);

	if (array->size == 0) {
		array->ptrs = malloc(8 * sizeof(void *));
		if (array->ptrs == NULL)
			return -ENOMEM;
		array->size = 8 * sizeof(void *);
		memset(array->ptrs, 0, array->size);
	}

	for (i = 0; i <= count; ++i) {
		if (i >= count) {
			array->ptrs = realloc(array->ptrs, array->size * 2);
			if (array->ptrs == NULL)
				return -ENOMEM;

			memset((uint8_t*)array->ptrs + array->size, 0, array->size);
			ptrs = (void**)array->ptrs;
			array->size *= 2;
		}

		if (ptrs[i] != NULL)
			continue;

		ptrs[i] = ptr;
		break;
	}

	return 0;
}

static inline int ptr_array_remove(struct ptr_array * array, void * ptr)
{
	return 0;
}

static inline int ptr_array_realloc(struct ptr_array * array, int count)
{
	int newsize = count * sizeof(void *);

	array->ptrs = realloc(array->ptrs, newsize);
	if (array->ptrs == NULL)
		return -ENOMEM;

	if (array->size < newsize) {
		memset((uint8_t*)array->ptrs + array->size, 0, newsize - array->size);
	}

	array->size = newsize;
	return 0;
}

#define ptr_array_for_each(item, array) \
	for ((item) = *((array)->ptrs); \
	     (uint8_t*)(item) < ((uint8_t*)(array)->ptrs + (array)->size); \
	     (item) = (void*)((uint8_t*)(item) + sizeof(void *)))

#endif

