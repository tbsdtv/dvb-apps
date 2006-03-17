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
#include "en50221_session.h"
#include "en50221_app_utils.h"
#include "en50221_app_datetime.h"
#include "asn_1.h"

// tags supported by this resource
#define TAG_DATE_TIME_ENQUIRY       0x9f8440
#define TAG_DATE_TIME           0x9f8441

struct en50221_app_datetime_private {
        en50221_session_layer *sl;

        en50221_app_datetime_enquiry_callback callback;
        void *callback_arg;
};

static void en50221_app_datetime_resource_callback(void *arg,
                                             uint8_t slot_id,
                                             uint16_t session_number,
                                             uint32_t resource_id,
                                             uint8_t *data, uint32_t data_length);



en50221_app_datetime en50221_app_datetime_create(en50221_session_layer sl, en50221_app_rm rm)
{
    struct en50221_app_datetime_private *private = NULL;

    // create structure and set it up
    private = malloc(sizeof(struct en50221_app_datetime_private));
    if (private == NULL) {
        return NULL;
    }
    private->sl = sl;
    private->callback = NULL;

    // register with the RM
    if (en50221_app_rm_register(rm, MKRID(36,1,1), en50221_app_datetime_resource_callback, private)) {
        free(private);
        return NULL;
    }

    // done
    return private;
}

void en50221_app_datetime_destroy(en50221_app_datetime datetime)
{
    struct en50221_app_datetime_private *private = (struct en50221_app_datetime_private *) datetime;

    free(private);
}

void en50221_app_datetime_register_enquiry_callback(en50221_app_datetime datetime,
                                            en50221_app_datetime_enquiry_callback callback, void *arg)
{
    struct en50221_app_datetime_private *private = (struct en50221_app_datetime_private *) datetime;

    private->callback = callback;
    private->callback_arg = arg;
}

int en50221_app_datetime_send(en50221_app_datetime datetime,
                              uint16_t session_number,
                              uint8_t utc_time[5],
                              int time_offset)
{
    struct en50221_app_datetime_private *private = (struct en50221_app_datetime_private *) datetime;
    uint8_t data[11];
    int data_length;

    data[0] = (TAG_DATE_TIME >> 16) & 0xFF;
    data[1] = (TAG_DATE_TIME >> 8) & 0xFF;
    data[2] = TAG_DATE_TIME & 0xFF;
    if (time_offset != -1) {
        data[3] = 7;
        memcpy(data+4, utc_time, 5);
        data[9] = time_offset >> 8;
        data[10] = time_offset;
    } else {
        data[3] = 5;
        memcpy(data+4, utc_time, 5);
    }
    return en50221_sl_send_data(private->sl, session_number, data, data_length);
}

static void en50221_app_datetime_parse_enquiry(struct en50221_app_datetime_private *private,
                                               uint8_t slot_id, uint16_t session_number,
                                               uint8_t *data, uint32_t data_length)
{
    // validate data
    if (data_length != 2) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    if (data[0] != 1) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    uint8_t response_interval = data[1];

    // tell the app
    if (private->callback)
        private->callback(private->callback_arg, slot_id, session_number, response_interval);
}

static void en50221_app_datetime_resource_callback(void *arg,
                                            uint8_t slot_id,
                                            uint16_t session_number,
                                            uint32_t resource_id,
                                            uint8_t *data, uint32_t data_length)
{
    struct en50221_app_datetime_private *private = (struct en50221_app_datetime_private *) arg;

    // get the tag
    if (data_length < 3) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    uint32_t tag = (data[0] << 16) | (data[1] << 8) | data[2];

    switch(tag)
    {
        case TAG_DATE_TIME_ENQUIRY:
            en50221_app_datetime_parse_enquiry(private, slot_id, session_number, data+3, data_length-3);
            break;
        default:
            print(LOG_LEVEL, ERROR, 1, "Received unexpected tag %x\n", tag);
            break;
    }
}
