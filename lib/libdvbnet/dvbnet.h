/*
 * libdvbnet - a DVB network support library
 *
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
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

#ifndef LIBDVBNET_H
#define LIBDVBNET_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <linux/dvb/net.h>
#include <stdint.h>

/**
 * The maximum allowed number of dvb network devices per adapter netdevice.
 */
#define DVBNET_MAX_INTERFACES 10

/**
 * Open a DVB net interface.
 *
 * @param adapter DVB adapter ID.
 * @param netdeviceid Network control interface of that adapter to open.
 * @return A unix file descriptor on success, or -1 on failure.
 */
extern int dvbnet_open(int adapter, int netdeviceid);

/**
 * Create a new DVBNET interface.
 *
 * @param fd FD opened with libdvbnet_open().
 * @param pid PID of the stream containing the network data.
 * @param encapsulation Encapsulation type of the stream (one of DVB_NET_FEEDTYPE_*).
 * @return 0 on success, nonzero on failure.
 */
extern int dvbnet_add_interface(int fd, uint16_t pid, int encapsulation);

/**
 * Get details of a DVBNET interface.
 *
 * @param fd FD opened with libdvbnet_open().
 * @param ifnum Index of interface to retrieve.
 * @param info Place to put the information.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbnet_get_interface(int fd, int ifnum, struct dvb_net_if *info);

/**
 * Remove a DVBNET interface.
 *
 * @param fd FD opened with libdvbnet_open().
 * @param ifnum Index of interface to remove.
 * @return 0 on success, nonzero on failure.
 */
extern int dvbnet_remove_interface(int fd, int ifnum);

#ifdef __cplusplus
}
#endif

#endif // LIBDVBNET_H
