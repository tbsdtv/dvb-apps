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

#include <dvb/thread.h>
#include <dvb/dvb.h>
#include <dvb/internal.h>
#include <dvb/frontend.h>
#include <dvb/demux.h>
#include <dvb/event.h>
#include <dvb/notifier.h>

#include <stdio.h>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>

void * thread(void *);

struct dvb_thread_data {
	pthread_mutex_t mutex;

	int running;
	pthread_t thread;
	struct timespec ts;

	struct list_entry taskqueue;
};

static inline struct dvb_thread_data * get_thread_data(struct dvb * dvb)
{
	struct dvb_thread_data * data;

	if (dvb->thread)
		return dvb->thread;

	data = malloc(sizeof(struct dvb_thread_data));

	if (data == NULL)
		return NULL;

	LIST_ENTRY_INIT(&data->taskqueue);
	data->running = 0;

	pthread_mutex_init(&data->mutex, NULL);

	dvb->thread = data;
	return data;
}

struct dvb_task {
	struct list_entry list;

	struct dvb_thread_data * data;
	struct timespec ts;

	dvb_task_func_t func;
	void * opaque;
};

int dvb_alloc_task(struct dvb * dvb, struct dvb_task ** ptr)
{
	struct dvb_thread_data * thread = get_thread_data(dvb);
	struct dvb_task * task;

	if (dvb == NULL)
		return -EINVAL;

	task = malloc(sizeof(struct dvb_task));
	if (task == NULL)
		return -ENOMEM;

	task->data = dvb->thread;
	task->func = NULL;
	task->opaque = NULL;

	list_append_entry(&thread->taskqueue, &task->list);

	*ptr = task;
	return 0;
}

int dvb_free_task(struct dvb_task * task)
{
	list_remove_entry(&(task->data->taskqueue), &task->list);
	free(task);
	return 0;
}

dvb_task_func_t dvb_get_task_func(struct dvb_task * handle)
{
	return handle ? handle->func : NULL;
}

void * dvb_get_task_opaque(struct dvb_task * handle)
{
	return handle ? handle->opaque : NULL;
}

int dvb_set_task_func(struct dvb_task * task, dvb_task_func_t func)
{
	if (task == NULL)
		return -EINVAL;
	task->func = func;
	return 0;
}

int dvb_set_task_opaque(struct dvb_task * task, void * opaque)
{
	if (task == NULL)
		return -EINVAL;
	task->opaque = opaque;
	return 0;
}

int dvb_schedule_task(struct dvb_task * task, int delay)
{
	if (task == NULL)
		return -ENOMEM;

	clock_gettime(CLOCK_MONOTONIC, &task->ts);

	task->ts.tv_sec += delay / 1000000;
	task->ts.tv_nsec += (delay % 1000000) * 1000;

	while (task->ts.tv_nsec > 1000000000) {
		task->ts.tv_nsec -= 1000000000;
		++task->ts.tv_sec;
	}

	return 0;
}

int dvb_enter_thread_loop(struct dvb * dvb)
{
	struct dvb_thread_data * data = get_thread_data(dvb);

	if (data == NULL)
		return -ENOMEM;

	if (data->running)
		return -EINVAL;

	data->running = 1;
	thread(dvb);
	return 0;
}

int dvb_start_thread(struct dvb * dvb)
{
	int ret;
	struct dvb_thread_data * data = get_thread_data(dvb);

	if (data == NULL)
		return -ENOMEM;

	if (data->running)
		return -EINVAL;

	if ((ret = pthread_create(&data->thread, NULL, thread, dvb)))
		return ret;

	return 0;
}

int dvb_stop_thread(struct dvb * dvb)
{
	int ret;
	struct dvb_thread_data * data = get_thread_data(dvb);

	if (data && data->running) {
		dvb_lock_thread(dvb);

		if ((ret = pthread_join(data->thread, NULL)))
			return ret;

		dvb_unlock_thread(dvb);

		pthread_mutex_destroy(&data->mutex);
		dvb->thread = NULL;
		free(data);
	}

	return 0;
}

void dvb_lock_thread(struct dvb * dvb)
{
	struct dvb_thread_data * data = dvb->thread;

	if (data == NULL)
		return;

	pthread_mutex_lock(&data->mutex);
}

int dvb_trylock_thread(struct dvb * dvb)
{
	struct dvb_thread_data * data = dvb->thread;

	if (data == NULL)
		return -EINVAL;

	if (pthread_mutex_trylock(&data->mutex))
		return -EBUSY;

	return 0;
}

void dvb_unlock_thread(struct dvb * dvb)
{
	struct dvb_thread_data * data = dvb->thread;

	if (data == NULL)
		return;

	pthread_mutex_unlock(&data->mutex);
}

static inline int do_scheduled_task(struct dvb * dvb)
{
	struct list_entry * pos;
	struct dvb_thread_data * data = dvb->thread;
	struct dvb_task * task;

	clock_gettime(CLOCK_MONOTONIC, &data->ts);

	for (pos = data->taskqueue.next; pos != NULL; pos = pos->next) {
		task = list_get_entry(pos, struct dvb_task, list);

		int diff = (data->ts.tv_sec - task->ts.tv_sec) * 1000000 +
			   (data->ts.tv_nsec - task->ts.tv_nsec) / 1000;

		if (diff < 0 || task->func == NULL)
			continue;

		task->func(task);

		// rotate task queue
		list_remove_entry(&data->taskqueue, pos);
		list_append_entry(&data->taskqueue, pos);
		return 0;
	}

	return -EAGAIN;
}

void * thread(void * opaque)
{
	struct dvb * dvb = opaque;

	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	for (;;) {

		if (dvb_trylock_thread(dvb)) {
			pthread_testcancel(); /* XXX */
			usleep(10000);
			continue;
		}

		dvb_send_one_event(dvb);
		dvb_unlock_thread(dvb);

		do_scheduled_task(dvb);

		dvb_poll(dvb, 10000);
		usleep(1000);
	}

	return NULL;
}

