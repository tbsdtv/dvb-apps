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

#ifndef _DVB_THREAD_H_
#define _DVB_THREAD_H_

#include <dvb/dvb.h>
#include <dvb/internal.h>

struct dvb_task;

typedef void (* dvb_task_func_t) (struct dvb_task * handle);

int dvb_alloc_task(struct dvb * dvb, struct dvb_task ** handle);
int dvb_free_task(struct dvb_task * handle);

dvb_task_func_t dvb_get_task_func(struct dvb_task * handle);
void * dvb_get_task_opaque(struct dvb_task * handle);

int dvb_set_task_func(struct dvb_task * handle, dvb_task_func_t func);
int dvb_set_task_opaque(struct dvb_task * handle, void * opaque);

int dvb_schedule_task(struct dvb_task * handle, int delay);

#endif

