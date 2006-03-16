/*
    en50221 encoder An implementation for libdvb
    an implementation for the en50221 session layer

    Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)
    Copyright (C) 2005 Julian Scheel (julian at jusst dot de)
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


#ifndef __EN50221_TRANSPORT_H__
#define __EN50221_TRANSPORT_H__


#include <stdlib.h>
#include <stdint.h>
#include <sys/uio.h>

/**
 * Opaque type representing a transport layer.
 */
typedef void *en50221_transport_layer;

/**
 * Type definition for callback function.
 */
typedef void (*en50221_tl_callback)(void *private, uint8_t *data, uint32_t data_length,
					 uint8_t slot_id, uint8_t connection_id);


/**
 * Construct a new instance of the transport layer.
 *
 * @param max_slots Maximum number of slots to support.
 * @param max_connections_per_slot Maximum connections per slot.
 * @return The en50221_transport_layer instance, or NULL on error.
 */
extern en50221_transport_layer en50221_tl_create(uint8_t max_slots, uint8_t max_connections_per_slot);

/**
 * Destroy an instance of the transport layer.
 *
 * @param tl The en50221_transport_layer instance.
 */
extern void en50221_tl_destroy(en50221_transport_layer tl);

/**
 * Register a new slot with the library.
 *
 * @param tl The en50221_transport_layer instance.
 * @param ca_hndl FD for talking to the slot.
 * @param response_timeout Maximum timeout in ms to a response we send before signalling a timeout.
 * @param poll_delay Interval between polls in ms.
 * @return slot_id on sucess, or -1 on error.
 */
extern int en50221_tl_register_slot(en50221_transport_layer tl, int ca_hndl,
                                    uint32_t response_timeout, uint32_t poll_delay);

/**
 * Destroy a registered slot - e.g. if a CAM is removed, or an error occurs. Does
 * not actually reset the CAM.
 *
 * @param tl The en50221_transport_layer instance.
 * @param slot_id Slot to destroy.
 */
extern void en50221_tl_destroy_slot(en50221_transport_layer tl, uint8_t slot_id);

/**
 * Performs one iteration of the transport layer poll -
 * checking for incoming data furthermore it will handle
 * the timeouts of certain commands like T_DELETE_T_C it
 * should be called by the application regularly, generally
 * faster than the poll delay.
 *
 * @param tl The en50221_transport_layer instance.
 * @return 0 on succes, or -1 if there was an error of some sort.
 */
extern int en50221_tl_poll(en50221_transport_layer tl);

/**
 * Register the callback for data reception.
 *
 * @param tl The en50221_transport_layer instance.
 * @param callback The callback.
 * @param private Private data passed as arg0 of the callback.
 */
extern void en50221_tl_register_callback(en50221_transport_layer tl,
						 en50221_tl_callback callback, void *private);

/**
 * Gets the ID of the slot an error occurred on.
 *
 * @param tl The en50221_transport_layer instance.
 * @return The offending slot id.
 */
extern int en50221_tl_get_error_slot(en50221_transport_layer tl);

/**
 * Gets the error code of the error.
 *
 * @param tl The en50221_transport_layer instance.
 * @return One of the EN50221ERR_* values.
 */
extern int en50221_tl_get_error(en50221_transport_layer tl);


/**
 * This function is used to take a data-block, pack into
 * into a TPDU (DATA_LAST) and send it to the device
 *
 * @param tl The en50221_transport_layer instance.
 * @param slot_id ID of the slot.
 * @param connection_id Connection id.
 * @param vector iov to send.
 * @param io_count Number of elements in vector.
 * @return 0 on success, or -1 on error.
 */
extern int en50221_tl_send_data(en50221_transport_layer tl,
  			                    uint8_t slot_id, uint8_t connection_id,
                                struct iovec *vector, int iov_count);

/**
 * Allocates a new transport connection.
 *
 * @param tl The en50221_transport_layer instance.
 * @param slot_id ID of the slot.
 * @param connection_id Connection id to send the request _on_.
 * @return The allocated connection id on success, or -1 on error.
 */
extern int en50221_tl_new_tc(en50221_transport_layer tl,
		      	     uint8_t slot_id, uint8_t connection_id);

/**
 * Deallocates a transport connection.
 *
 * @param tl The en50221_transport_layer instance.
 * @param slot_id ID of the slot.
 * @param connection_id Connection id to send the request _on_.
 * @return 0 on success, or -1 on error.
 */
extern int en50221_tl_del_tc(en50221_transport_layer tl,
 			     uint8_t slot_id, uint8_t connection_id);

#endif
