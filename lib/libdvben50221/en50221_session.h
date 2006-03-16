/*
    en50221 encoder An implementation for libdvb
    an implementation for the en50221 session layer

    Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)
    Copyright (C) 2005 Julian Scheel (julian@jusst.de)
    Copyright (C) 2006 Andrew de Quincey (adq_dvb@lidskialf.net)

    This library is free software; you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation; either version 2.1 of
    the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/


#ifndef __EN50221_SESSION_H__
#define __EN50221_SESSION_H__

#include <stdlib.h>
#include <stdint.h>
#include "en50221_transport.h"

// these are the possible session statuses
#define S_STATUS_OPEN                    0x00  // session is opened
#define S_STATUS_CLOSE_NO_RES            0xF0  // could not open session, no proper resource available
#define S_STATUS_CLOSE_RES_UNAVAILABLE   0xF1  // could not open session, resource unavailable
#define S_STATUS_CLOSE_RES_LOW_VERSION   0xF2  // could not open session, resource version too low
#define S_STATUS_CLOSE_RES_BUSY          0xF3  // could not open sessionm resource is busy

#define S_CALLBACK_REASON_CONNECT        0x00  // Session to resource created
#define S_CALLBACK_REASON_DATA           0x01  // Data received for resource
#define S_CALLBACK_REASON_CLOSE          0x02  // Session closed


/**
 * Opaque type representing a session layer.
 */
typedef void *en50221_session_layer;

/**
 * Type definition for resource callback function - called by session layer when data
 * arrives for a particular resource.
 *
 * @param arg Private argument.
 * @param reason One of the S_CALLBACK_REASON_* values.
 * @param session_number Session it arrived on.
 * @param resource_id Resource ID concerned.
 * @param data The data.
 * @param data_length Length of data in bytes.
 */
typedef void (*en50221_sl_resource_callback)(void *arg,
                                             int reason,
                                             uint16_t session_number,
                                             uint32_t resource_id,
                                             uint8_t *data, uint32_t data_length);

/**
 * Type definition for resource lookup callback function - used by the session layer to look up requested resources.
 *
 * @param arg Private argument.
 * @param resource_id Resource id to look up.
 * @param arg_out Output parameter for arg to pass to resource callback.
 * @param callback_out Output parameter for pointer to resource callback function.
 * @return One of the S_STATUS_* values above.
 */
typedef int (*en50221_sl_lookup_callback)(void *arg, uint32_t resource_id,
                                           void**arg_out, en50221_sl_resource_callback *callback_out);

/**
 * Construct a new instance of the session layer.
 *
 * @param tl The en50221_transport_layer instance to use.
 * @param max_sessions Maximum number of sessions supported.
 * @return The en50221_session_layer instance, or NULL on error.
 */
extern en50221_session_layer en50221_sl_create(en50221_transport_layer tl, uint32_t max_sessions);

/**
 * Destroy an instance of the session layer.
 *
 * @param tl The en50221_session_layer instance.
 */
extern void en50221_sl_destroy(en50221_session_layer sl);

/**
 * Register the callback for resource lookup.
 *
 * @param sl The en50221_session_layer instance.
 * @param callback The callback. Set to NULL to remove the callback completely.
 * @param arg Private data passed as arg0 of the callback.
 */
extern void en50221_sl_register_lookup_callback(en50221_session_layer sl,
                                                en50221_sl_lookup_callback callback, void *arg);

/**
 * this function is used to take a data-block, pack into
 * into a SPDU (SESSION_NUMBER) and send it to the transport layer
 *
 * @param tl The en50221_transport_layer instance to use.
 * @param session_number Session number concerned.
 * @param data Data to send.
 * @param data_length Length of data in bytes.
 * @return 0 on success, or -1 on error.
 */
extern int en50221_sl_send_data(en50221_transport_layer tl, uint8_t session_number, uint8_t *data, uint16_t data_length);

/**
 * this is used to send a message to all sessions, linked
 * to resource res
 *
 * @param tl The en50221_transport_layer instance to use.
 * @param resource_id Resource id concerned.
 * @param data Data to send.
 * @param data_length Length of data in bytes.
 * @return 0 on success, or -1 on error.
 */
extern int en50221_sl_broadcast_data(en50221_transport_layer tl, uint32_t resource_id, uint8_t *data, uint16_t data_length);

#endif
