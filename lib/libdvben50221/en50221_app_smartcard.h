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

#ifndef __EN50221_APPLICATION_smartcard_H__
#define __EN50221_APPLICATION_smartcard_H__

#include <stdlib.h>
#include <stdint.h>
#include <en50221_session.h>
#include <en50221_app_rm.h>

#define SMARTCARD_COMMAND_ID_CONNECT            0x01
#define SMARTCARD_COMMAND_ID_DISCONNECT         0x02
#define SMARTCARD_COMMAND_ID_POWERON_CARD       0x03
#define SMARTCARD_COMMAND_ID_POWEROFF_CARD      0x04
#define SMARTCARD_COMMAND_ID_RESET_CARD         0x05
#define SMARTCARD_COMMAND_ID_RESET_STATUS       0x06
#define SMARTCARD_COMMAND_ID_READ_ANSW_TO_RESET 0x07

#define SMARTCARD_REPLY_ID_CONNECTED            0x01
#define SMARTCARD_REPLY_ID_FREE                 0x02
#define SMARTCARD_REPLY_ID_BUSY                 0x03
#define SMARTCARD_REPLY_ID_ANSW_TO_RESET        0x04
#define SMARTCARD_REPLY_ID_NO_ANSW_TO_RESET     0x05

#define SMARTCARD_STATUS_CARD_INSERTED          0x01
#define SMARTCARD_STATUS_CARD_REMOVED           0x02
#define SMARTCARD_STATUS_CARD_IN_PLACE_POWEROFF 0x03
#define SMARTCARD_STATUS_CARD_IN_PLACE_POWERON  0x04
#define SMARTCARD_STATUS_CARD_NO_CARD           0x05
#define SMARTCARD_STATUS_CARD_UNRESPONSIVE_CARD 0x06
#define SMARTCARD_STATUS_CARD_REFUSED_CARD      0x07


/**
 * Type definition for command - called when we receive a command.
 *
 * @param arg Private argument.
 * @param slot_id Slot id concerned.
 * @param session_number Session number concerned.
 * @param command_id One of the SMARTCARD_COMMAND_ID_* values
 */
typedef void (*en50221_app_smartcard_command_callback)(void *arg, uint8_t slot_id, uint16_t session_number,
                                                       uint8_t command_id);

/**
 * Type definition for command - called when we receive a send command.
 *
 * @param arg Private argument.
 * @param slot_id Slot id concerned.
 * @param session_number Session number concerned.
 * @param CLA CLA value.
 * @param INS INS value.
 * @param P1 P1 value.
 * @param P2 P2 value.
 * @param in Data to send to the card
 * @param in_length Number of bytes to send.
 * @param out_length Number of bytes expected.
 */
typedef void (*en50221_app_smartcard_send_callback)(void *arg, uint8_t slot_id, uint16_t session_number,
                                                    uint8_t CLA, uint8_t INS, uint8_t P1, uint8_t P2,
                                                    uint8_t *in, uint32_t in_length,
                                                    uint32_t out_length);

/**
 * Opaque type representing a smartcard resource.
 */
typedef void *en50221_app_smartcard;

/**
 * Create an instance of the smartcard resource.
 *
 * @param sl Session layer to communicate with.
 * @param rm Resource Manager to register with.
 * @param device_number Number of the particular device.
 * @return Instance, or NULL on failure.
 */
extern en50221_app_smartcard en50221_app_smartcard_create(en50221_session_layer sl, en50221_app_rm rm,
                                                          uint8_t device_number);

/**
 * Destroy an instance of the smartcard resource.
 *
 * @param rm Instance to destroy.
 */
extern void en50221_app_smartcard_destroy(en50221_app_smartcard smartcard);

/**
 * Register the callback for when we receive a comms command.
 *
 * @param smartcard smartcard resource instance.
 * @param callback The callback. Set to NULL to remove the callback completely.
 * @param arg Private data passed as arg0 of the callback.
 */
extern void en50221_app_smartcard_register_command_callback(en50221_app_smartcard smartcard,
        en50221_app_smartcard_command_callback callback, void *arg);

/**
 * Register the callback for when we receive data to send.
 *
 * @param smartcard smartcard resource instance.
 * @param callback The callback. Set to NULL to remove the callback completely.
 * @param arg Private data passed as arg0 of the callback.
 */
extern void en50221_app_smartcard_register_send_callback(en50221_app_smartcard smartcard,
        en50221_app_smartcard_send_callback callback, void *arg);

/**
 * Send a command response to the CAM.
 *
 * @param smartcard smartcard resource instance.
 * @param session_number Session number to send it on.
 * @param reply_id One of the SMARTCARD_REPLY_ID_* values.
 * @param status One of the SMARTCARD_STATUS_* values.
 * @param data Data to send when it is a SMARTCARD_REPLY_ID_ANSW_TO_RESET.
 * @param data_length Length of data to send.
 * @return 0 on success, -1 on failure.
 */
extern int en50221_app_smartcard_command_reply(en50221_app_smartcard smartcard,
                                               uint16_t session_number,
                                               uint8_t reply_id,
                                               uint8_t status,
                                               uint8_t *data,
                                               uint32_t data_length);

/**
 * Send data received from a smartcart to the CAM.
 *
 * @param smartcard smartcard resource instance.
 * @param session_number Session number to send it on.
 * @param data Data to send when it is a SMARTCARD_REPLY_ID_ANSW_TO_RESET.
 * @param data_length Length of data to send.
 * @param SW1 SW1 value.
 * @param SW2 SW2 value.
 * @return 0 on success, -1 on failure.
 */
extern int en50221_app_smartcard_receive(en50221_app_smartcard smartcard,
                                         uint16_t session_number,
                                         uint8_t *data,
                                         uint32_t data_length,
                                         uint8_t SW1,
                                         uint8_t SW2);

#endif
