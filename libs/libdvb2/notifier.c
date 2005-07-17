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

#include <dvb/internal.h>
#include <dvb/notifier.h>
#include <dvb/array.h>

#include <stdlib.h>
#include <errno.h>
#include <string.h>

#include <stdio.h>

/**
 * @dvb		- the dvb context
 * @callbacks	- callbacks for adding/removing poll fds
 *
 * This function must be called right after creating the
 * dvb context, else some poll fds may be missed.
 */
int dvb_set_poll_callbacks(struct dvb * dvb,
			   const struct dvb_poll_callbacks * callbacks)
{
	dvb->poll_callbacks = *callbacks;
	return 0;
}

/**
 * @dvb		- dvb context
 * @pfds	- array of struct pollfd
 * @nfds	- size of pfds array
 *
 * Call this function when your poll function returns > 0,
 * the library will then handle the revents.
 */
int dvb_revents(struct dvb * dvb, const struct pollfd * pfds, int nfds)
{
	int i, n;

	for (i = 0; i < nfds; ++i) {
		if (pfds[i].revents == 0)
			continue;

		for (n = 0; n < dvb->nb_notifiers; ++n) {
			struct dvb_notifier * notifier = dvb->notifiers.ptrs[i];

			if (notifier == NULL)
				continue;
			if (dvb->pollfds[n].fd != pfds[i].fd)
				continue;

			dvb->pollfds[n].revents = pfds[i].revents;
			notifier->callback(notifier, &dvb->pollfds[n]);
		}
	}

	return 0;
}

/**
 * @dvb		- dvb context
 * @fd		- filedescriptor to handle
 * @revents	- events on fd
 *
 * Simple version of the dvb_revents function, call this function
 * if you are not able to construct an struct pollfd array.
 *
 * The library will handle the revents specified.
 */
int dvb_revent(struct dvb * dvb, int fd, short revents)
{
	struct dvb_notifier * notifier;
	int i;

	if (revents == 0)
		return 0;

	for (i = 0; i <= dvb->nb_notifiers; ++i) {
		if (dvb->notifiers.ptrs[i] == NULL ||
		    dvb->pollfds[i].fd != fd)
			continue;

		dvb->pollfds[i].revents = revents;
		notifier->callback(notifier, &dvb->pollfds[i]);
	}

	return 0;
}

static inline int __realloc_notifier_arrays(struct dvb * dvb)
{
	if (ptr_array_realloc(&dvb->notifiers, dvb->nb_notifiers + 8))
		return -ENOMEM;

	dvb->pollfds = realloc(dvb->pollfds, (dvb->nb_notifiers + 8) *
			       sizeof(struct pollfd));

	if (dvb->pollfds == NULL)
		return -ENOMEM;

	memset(dvb->pollfds + dvb->nb_notifiers, 0,
	       8 * sizeof(struct pollfd));

	dvb->nb_notifiers += 8;

	return 0;
}

/**
 * @dvb		- context
 * @pfd		- fd and events to handle
 * @notifier	- notifier struct with opaque variable and callback
 */
int dvb_add_notifier(struct dvb * dvb, const struct pollfd * pfd,
		     struct dvb_notifier * notifier)
{
	int i;

	for (i = 0; i <= dvb->nb_notifiers; ++i) {
		if (i == dvb->nb_notifiers) {
			if (__realloc_notifier_arrays(dvb))
				return -ENOMEM;
		}

		if (dvb->notifiers.ptrs[i] != NULL)
			continue;

		dvb->notifiers.ptrs[i] = notifier;
		dvb->pollfds[i] = *pfd;

		if (dvb->poll_callbacks.add) {
			dvb->poll_callbacks.add(pfd->fd, pfd->events);
		}

		break;
	}

	return 0;
}

/**
 * @dvb		- context
 * @notifier	- notifier struct with opaque variable and callback
 *
 * Remove the notifier registered with dvb_add_notifier, if no matching
 * notifier is found -EINVAL is returned.
 */
int dvb_remove_notifier(struct dvb * dvb, struct dvb_notifier * notifier)
{
	int i;

	for (i = 0; i < dvb->nb_notifiers; ++i) {
		if (dvb->notifiers.ptrs[i] != notifier)
			continue;

		if (dvb->poll_callbacks.remove)
			dvb->poll_callbacks.remove(dvb->pollfds[i].fd);

		dvb->notifiers.ptrs[i] = NULL;
		memset(&dvb->pollfds[i], 0, sizeof(struct pollfd));
		return 0;
	}

	return -EINVAL;
}

/**
 * @dvb		- dvb context
 * @timeout	- timeout in milliseconds
 *
 * Polls all filedescriptors in use in the library.
 *
 * Returns:
 *	- positive number if any polls where handled.
 *	- zero if the poll timedout
 *	- negative errno value of the failed poll system call
 */
int dvb_poll(struct dvb * dvb, int timeout)
{
	int ret, i;

	if (dvb->nb_notifiers <= 0)
		return 0;

	ret = poll(dvb->pollfds, dvb->nb_notifiers, timeout);

	if (ret == 0)
		return 0;

	if (ret < 0)
		return -errno;

	for (i = 0; i < dvb->nb_notifiers; ++i) {
		struct dvb_notifier * notifier = dvb->notifiers.ptrs[i];

		if (dvb->pollfds[i].revents == 0)
			continue;

		if (notifier == NULL)
			continue;

		notifier->callback(notifier, &dvb->pollfds[i]);
	}

	return ret;
}

