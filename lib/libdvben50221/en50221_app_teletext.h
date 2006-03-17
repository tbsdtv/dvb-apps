/*
    en50221 encoder An implementation for libdvb
    an implementation for the en50221 transport layer

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

#ifndef __EN50221_APPLICATION_teletext_H__
#define __EN50221_APPLICATION_teletext_H__

#include <stdlib.h>
#include <stdint.h>
#include <en50221_session.h>
#include <en50221_app_rm.h>


/**
 * Type definition for request - called when we receive teletext from a CAM.
 *
 * @param arg Private argument.
 * @param slot_id Slot id concerned.
 * @param session_number Session number concerned.
 * @param teletext_data Data for the request.
 * @param teletext_data_lenghth Number of bytes.
 */
typedef void (*en50221_app_teletext_callback)(void *arg, uint8_t slot_id, uint16_t session_number,
                                              uint8_t *teletext_data,
                                              uint32_t teletext_data_length);

/**
 * Opaque type representing a teletext resource.
 */
typedef void *en50221_app_teletext;

/**
 * Create an instance of the teletext resource.
 *
 * @param sl Session layer to communicate with.
 * @param rm Resource Manager to register with
 * @return Instance, or NULL on failure.
 */
extern en50221_app_teletext en50221_app_teletext_create(en50221_session_layer sl, en50221_app_rm rm);

/**
 * Destroy an instance of the teletext resource.
 *
 * @param rm Instance to destroy.
 */
extern void en50221_app_teletext_destroy(en50221_app_teletext teletext);

/**
 * Register the callback for when we receive a request.
 *
 * @param teletext teletext resource instance.
 * @param callback The callback. Set to NULL to remove the callback completely.
 * @param arg Private data passed as arg0 of the callback.
 */
extern void en50221_app_teletext_register_callback(en50221_app_teletext teletext,
                                                   en50221_app_teletext_callback callback, void *arg);

#endif
