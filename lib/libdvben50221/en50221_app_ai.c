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

#include <string.h>
#include <dvbmisc.h>
#include "en50221_app_ai.h"
#include "asn_1.h"

// tags supported by this resource
#define TAG_APP_INFO_ENQUIRY        0x9f8020
#define TAG_APP_INFO            0x9f8021
#define TAG_ENTER_MENU          0x9f8022

struct en50221_app_ai_private {
        struct en50221_app_send_functions *funcs;

        en50221_app_ai_callback callback;
        void *callback_arg;
};

static void en50221_app_ai_parse_app_info(struct en50221_app_ai_private *private,
                                          uint8_t slot_id, uint16_t session_number,
                                          uint8_t *data, uint32_t data_length);


en50221_app_ai en50221_app_ai_create(struct en50221_app_send_functions *funcs)
{
    struct en50221_app_ai_private *private = NULL;

    // create structure and set it up
    private = malloc(sizeof(struct en50221_app_ai_private));
    if (private == NULL) {
        return NULL;
    }
    private->funcs = funcs;
    private->callback = NULL;

    // done
    return private;
}

void en50221_app_ai_destroy(en50221_app_ai ai)
{
    struct en50221_app_ai_private *private = (struct en50221_app_ai_private *) ai;

    free(private);
}

void en50221_app_ai_register_callback(en50221_app_ai ai, en50221_app_ai_callback callback, void *arg)
{
    struct en50221_app_ai_private *private = (struct en50221_app_ai_private *) ai;

    private->callback = callback;
    private->callback_arg = arg;
}

int en50221_app_ai_enquiry(en50221_app_ai ai, uint16_t session_number)
{
    struct en50221_app_ai_private *private = (struct en50221_app_ai_private *) ai;
    uint8_t data[3];

    data[0] = (TAG_APP_INFO_ENQUIRY >> 16) & 0xFF;
    data[1] = (TAG_APP_INFO_ENQUIRY >> 8) & 0xFF;
    data[2] = TAG_APP_INFO_ENQUIRY & 0xFF;

    return private->funcs->send_data(private->funcs->arg, session_number, data, 3);
}

int en50221_app_ai_entermenu(en50221_app_ai ai, uint16_t session_number)
{
    struct en50221_app_ai_private *private = (struct en50221_app_ai_private *) ai;
    uint8_t data[3];

    data[0] = (TAG_ENTER_MENU >> 16) & 0xFF;
    data[1] = (TAG_ENTER_MENU >> 8) & 0xFF;
    data[2] = TAG_ENTER_MENU & 0xFF;

    return private->funcs->send_data(private->funcs->arg, session_number, data, 3);
}

int en50221_app_ai_message(en50221_app_ai ai,
                           uint8_t slot_id,
                           uint16_t session_number,
                           uint32_t resource_id,
                           uint8_t *data, uint32_t data_length)
{
    struct en50221_app_ai_private *private = (struct en50221_app_ai_private *) ai;
    (void) resource_id;

    // get the tag
    if (data_length < 3) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return -1;
    }
    uint32_t tag = (data[0] << 16) | (data[1] << 8) | data[2];

    switch(tag)
    {
        case TAG_APP_INFO:
            en50221_app_ai_parse_app_info(private, slot_id, session_number, data+3, data_length-3);
            break;
        default:
            print(LOG_LEVEL, ERROR, 1, "Received unexpected tag %x\n", tag);
            return -1;
    }

    return 0;
}







static void en50221_app_ai_parse_app_info(struct en50221_app_ai_private *private,
                                          uint8_t slot_id, uint16_t session_number,
                                          uint8_t *data, uint32_t data_length)
{
    // parse the length field
    int length_field_len;
    uint16_t asn_data_length;
    if ((length_field_len = asn_1_decode(&asn_data_length, data, data_length)) < 0) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        return;
    }

    // check it
    if (asn_data_length < 6) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    if (asn_data_length > (data_length - length_field_len)) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    uint8_t *app_info = data + length_field_len;

    // parse the fields
    uint8_t application_type = app_info[0];
    uint16_t application_manufacturer = (app_info[1] << 8) | app_info[2];
    uint16_t manufacturer_code = (app_info[3] << 8) | app_info[4];
    uint8_t menu_string_length = app_info[5];
    uint8_t *menu_string = app_info + 6;

    // check the menu_string_length
    if (menu_string_length > (asn_data_length-6)) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }

    // tell the app
    if (private->callback)
        private->callback(private->callback_arg, slot_id, session_number,
                          application_type, application_manufacturer,
                          manufacturer_code, menu_string_length, menu_string);
}
