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
#include <ucsi/dvb/descriptor.h>
#include "en50221_session.h"
#include "en50221_app_utils.h"
#include "en50221_app_smartcard.h"
#include "asn_1.h"

// tags supported by this resource
#define TAG_SMARTCARD_COMMAND       0x9f8e00
#define TAG_SMARTCARD_REPLY         0x9f8e01
#define TAG_SMARTCARD_SEND          0x9f8e02
#define TAG_SMARTCARD_RCV           0x9f8e03

struct en50221_app_smartcard_private {
        en50221_session_layer *sl;

        en50221_app_smartcard_command_callback command_callback;
        void *command_callback_arg;

        en50221_app_smartcard_send_callback send_callback;
        void *send_callback_arg;
};

static void en50221_app_smartcard_resource_callback(void *arg,
                                                    uint8_t slot_id,
                                                    uint16_t session_number,
                                                    uint32_t resource_id,
                                                    uint8_t *data, uint32_t data_length);



en50221_app_smartcard en50221_app_smartcard_create(en50221_session_layer sl, en50221_app_rm rm,
                                                   uint8_t device_number)
{
    struct en50221_app_smartcard_private *private = NULL;

    // create structure and set it up
    private = malloc(sizeof(struct en50221_app_smartcard_private));
    if (private == NULL) {
        return NULL;
    }
    private->sl = sl;
    private->command_callback = NULL;
    private->send_callback = NULL;

    // register with the RM
    if (en50221_app_rm_register(rm,
                                MKRID(112, (device_number & 0x0f), 1),
                                en50221_app_smartcard_resource_callback, private)) {
        free(private);
        return NULL;
    }

    // done
    return private;
}

void en50221_app_smartcard_destroy(en50221_app_smartcard smartcard)
{
    struct en50221_app_smartcard_private *private = (struct en50221_app_smartcard_private *) smartcard;

    free(private);
}

void en50221_app_smartcard_register_command_callback(en50221_app_smartcard smartcard,
                                                    en50221_app_smartcard_command_callback callback, void *arg)
{
    struct en50221_app_smartcard_private *private = (struct en50221_app_smartcard_private *) smartcard;

    private->command_callback = callback;
    private->command_callback_arg = arg;
}

void en50221_app_smartcard_register_send_callback(en50221_app_smartcard smartcard,
                                                 en50221_app_smartcard_send_callback callback, void *arg)
{
    struct en50221_app_smartcard_private *private = (struct en50221_app_smartcard_private *) smartcard;

    private->send_callback = callback;
    private->send_callback_arg = arg;
}

int en50221_app_smartcard_command_reply(en50221_app_smartcard smartcard,
                                           uint16_t session_number,
                                           uint8_t reply_id,
                                           uint8_t status,
                                           uint8_t *data,
                                           uint32_t data_length)
{
    struct en50221_app_smartcard_private *private = (struct en50221_app_smartcard_private *) smartcard;
    uint8_t hdr[10];
    struct iovec iovec[2];
    int iov_count = 0;

    // the tag
    hdr[0] = (TAG_SMARTCARD_REPLY >> 16) & 0xFF;
    hdr[1] = (TAG_SMARTCARD_REPLY >> 8) & 0xFF;
    hdr[2] = TAG_SMARTCARD_REPLY & 0xFF;

    // the rest of the data
    if (reply_id == SMARTCARD_REPLY_ID_ANSW_TO_RESET) {
        // encode the length field
        int length_field_len;
        if ((length_field_len = asn_1_encode(data_length+2, data+3, 3)) < 0) {
            return -1;
        }

        // the rest of the header
        hdr[3+length_field_len] = reply_id;
        hdr[3+length_field_len+1] = status;
        iovec[0].iov_base = hdr;
        iovec[0].iov_len = 3+length_field_len+2;

        // the data
        iovec[1].iov_base = data;
        iovec[1].iov_len = data_length;
        iov_count = 2;
    } else {
        hdr[3] = 2;
        hdr[4] = reply_id;
        hdr[5] = status;
        iovec[0].iov_base = data;
        iovec[0].iov_len = 6;
        iov_count = 1;
    }

    return en50221_sl_send_datav(private->sl, session_number, iovec, iov_count);
}

int en50221_app_smartcard_receive(en50221_app_smartcard smartcard,
                                  uint16_t session_number,
                                  uint8_t *data,
                                  uint32_t data_length,
                                  uint8_t SW1,
                                  uint8_t SW2)
{
    struct en50221_app_smartcard_private *private = (struct en50221_app_smartcard_private *) smartcard;
    uint8_t buf[10];
    uint8_t trailer[10];

    // set up the tag
    buf[0] = (TAG_SMARTCARD_RCV >> 16) & 0xFF;
    buf[1] = (TAG_SMARTCARD_RCV >> 8) & 0xFF;
    buf[2] = TAG_SMARTCARD_RCV & 0xFF;

    // encode the length field
    int length_field_len;
    if ((length_field_len = asn_1_encode(data_length+2, buf+3, 3)) < 0) {
        return -1;
    }

    // set up the trailer
    trailer[0] = SW1;
    trailer[1] = SW2;

    // build the iovecs
    struct iovec iov[3];
    iov[0].iov_base = buf;
    iov[0].iov_len = 3+length_field_len;
    iov[1].iov_base = data;
    iov[1].iov_len = data_length;
    iov[2].iov_base = trailer;
    iov[2].iov_len = 2;

    // create the data and send it
    return en50221_sl_send_datav(private->sl, session_number, iov, 3);
}


static void en50221_app_smartcard_parse_command(struct en50221_app_smartcard_private *private,
                                               uint8_t slot_id, uint16_t session_number,
                                               uint8_t *data, uint32_t data_length)
{
    if (data_length != 2) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    if (data[0] != 1) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    uint8_t command_id = data[1];

    // tell the app
    if (private->command_callback)
        private->command_callback(private->command_callback_arg, slot_id, session_number,
                                  command_id);
}

static void en50221_app_smartcard_parse_send(struct en50221_app_smartcard_private *private,
                                            uint8_t slot_id, uint16_t session_number,
                                            uint8_t *data, uint32_t data_length)
{
    // first of all, decode the length field
    uint16_t asn_data_length;
    int length_field_len;
    if ((length_field_len = asn_1_decode(&asn_data_length, data, data_length)) < 0) {
        print(LOG_LEVEL, ERROR, 1, "ASN.1 decode error\n");
        return;
    }

    // check it
    if (asn_data_length < 8) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    if (asn_data_length > data_length) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    data += length_field_len;

    // parse
    uint8_t CLA = data[0];
    uint8_t INS = data[1];
    uint8_t P1 = data[2];
    uint8_t P2 = data[3];
    uint16_t length_in = (data[4]<<8)|data[5];
    uint8_t *data_in = data + 6;

    // validate the length
    if ((length_in + 8) != asn_data_length) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    uint16_t length_out = (data[6+length_in]<<8)|data[6+length_in+1];

    // tell the app
    if (private->send_callback)
        private->send_callback(private->send_callback_arg, slot_id, session_number,
                               CLA, INS, P1, P2, data_in, length_in, length_out);
}

static void en50221_app_smartcard_resource_callback(void *arg,
                                            uint8_t slot_id,
                                            uint16_t session_number,
                                            uint32_t resource_id,
                                            uint8_t *data, uint32_t data_length)
{
    struct en50221_app_smartcard_private *private = (struct en50221_app_smartcard_private *) arg;
    (void)resource_id;

    // get the tag
    if (data_length < 3) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    uint32_t tag = (data[0] << 16) | (data[1] << 8) | data[2];

    switch(tag)
    {
        case TAG_SMARTCARD_COMMAND:
            en50221_app_smartcard_parse_command(private, slot_id, session_number, data+3, data_length-3);
            break;
        case TAG_SMARTCARD_SEND:
            en50221_app_smartcard_parse_send(private, slot_id, session_number, data+3, data_length-3);
            break;
        default:
            print(LOG_LEVEL, ERROR, 1, "Received unexpected tag %x\n", tag);
            break;
    }
}
