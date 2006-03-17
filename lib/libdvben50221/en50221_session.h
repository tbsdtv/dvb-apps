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
#include <en50221_transport.h>

#define S_CALLBACK_REASON_CONNECTING     0x00  // Session connecting to resource - not established yet!
#define S_CALLBACK_REASON_CONNECTED      0x01  // Session connection established succesfully
#define S_CALLBACK_REASON_DATA           0x02  // Data received for resource
#define S_CALLBACK_REASON_CLOSE          0x03  // Session closed


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
 * @param slot_id The slot_id the request originated in.
 * @param session_number Session it arrived on.
 * @param resource_id Resource ID concerned.
 * @param data The data.
 * @param data_length Length of data in bytes.
 * @return 0 on success, or -1 on failure.
 */
typedef int (*en50221_sl_resource_callback)(void *arg,
                                            int reason,
                                            uint8_t slot_id,
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
 * @return 0 on success,
 * -1 if the resource was not found,
 * -2 if it exists, but had a lower version, or
 * -3 if it exists, but was unavailable.
 */
typedef int (*en50221_sl_lookup_callback)(void *arg, uint32_t resource_id,
                                          en50221_sl_resource_callback *callback_out, void **arg_out);

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
 * Gets the last error.
 *
 * @param tl The en50221_session_layer instance.
 * @return One of the EN50221ERR_* values.
 */
extern int en50221_sl_get_error(en50221_session_layer tl);

/**
 * Gets the maximum number of supported sessions.
 *
 * @param tl The en50221_session_layer instance.
 * @return The number.
 */
extern int en50221_sl_get_max_sessions(en50221_session_layer tl);

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
 * Create a new session to a module in a slot.
 *
 * @param sl The en50221_session_layer instance.
 * @param slot The slot to connect to.
 * @param resource_id The resource_id to connect to.
 * @param callback The callback for received data.
 * @param arg Argument to pass to the callback.
 * @return The new session_number, or -1 on error.
 */
extern int en50221_sl_create_session(en50221_session_layer sl, int slot_id, uint8_t connection_id,
                                     uint32_t resource_id,
                                     en50221_sl_resource_callback callback, void* arg);

/**
 * Destroy a session.
 *
 * @param sl The en50221_session_layer instance.
 * @param session_number The session to destroy.
 * @return 0 on success, or -1 on error.
 */
extern int en50221_sl_destroy_session(en50221_session_layer sl, int session_number);

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
 * @param slot_id Set to -1 to send to any slot. Other values will send to only that slot.
 * @param resource_id Resource id concerned.
 * @param data Data to send.
 * @param data_length Length of data in bytes.
 * @return 0 on success, or -1 on error.
 */
extern int en50221_sl_broadcast_data(en50221_transport_layer tl, int slot_id, uint32_t resource_id,
                                     uint8_t *data, uint16_t data_length);

#endif
