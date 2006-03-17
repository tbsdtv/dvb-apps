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

#ifndef __EN50221_APPLICATION_RM_H__
#define __EN50221_APPLICATION_RM_H__

#include <stdlib.h>
#include <stdint.h>
#include "en50221_session.h"


/**
 * Type definition for resources callback function - called when we receive
 * a list of resource ids supported by a CAM.
 *
 * @param arg Private argument.
 * @param slot_id Slot id concerned.
 * @param resources_count Number of resources.
 * @param resources Pointer to resourceids themselves.
 */
typedef void (*en50221_app_rm_resources_callback)(void *arg, uint8_t slot_id,
                                                  uint32_t resources_count, uint32_t *resources);

/**
 * Opaque type representing a resource manager.
 */
typedef void *en50221_app_rm;

/**
 * Create an instance of the resource manager.
 *
 * @param sl Session layer to communicate with.
 * @return Instance, or NULL on failure.
 */
extern en50221_app_rm en50221_app_rm_create(en50221_session_layer sl);

/**
 * Destroy an instance of the resource manager.
 *
 * @param rm Instance to destroy.
 */
extern void en50221_app_rm_destroy(en50221_app_rm rm);

/**
 * Register a host-side resource provider with the resource manager.
 *
 * @param rm Resource manager instance.
 * @param resource_id Resource identifier.
 * @param callback Callback called when this resource receives an event.
 * @param arg Private argument passed during calls to the callback.
 * @return 0 on success, or -1 on failure.
 */
extern int en50221_app_rm_register(en50221_app_rm rm, uint32_t resource_id,
                            en50221_sl_resource_callback callback, void *arg);

/**
 * Register the callback for when we receive a list of resources supported by a cam.
 *
 * @param rm Resource manager instance.
 * @param callback The callback. Set to NULL to remove the callback completely.
 * @param arg Private data passed as arg0 of the callback.
 */
extern void en50221_rm_register_resources_callback(en50221_app_rm rm,
                                                   en50221_app_rm_resources_callback callback, void *arg);

#endif
