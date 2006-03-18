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
#include "en50221_app_lowspeed.h"
#include "asn_1.h"

// tags supported by this resource
#define TAG_COMMS_COMMAND       0x9f8c00
#define TAG_CONNECTION_DESCRIPTOR   0x9f8c01
#define TAG_COMMS_REPLY         0x9f8c02
#define TAG_COMMS_SEND_LAST     0x9f8c03
#define TAG_COMMS_SEND_MORE     0x9f8c04
#define TAG_COMMS_RECV_LAST     0x9f8c05
#define TAG_COMMS_RECV_MORE     0x9f8c06

struct en50221_app_lowspeed_private {
        en50221_session_layer *sl;

        en50221_app_lowspeed_command_callback command_callback;
        void *command_callback_arg;

        en50221_app_lowspeed_send_callback send_callback;
        void *send_callback_arg;
};

static int en50221_app_lowspeed_parse_connect_on_channel(struct en50221_app_lowspeed_command *command,
        uint8_t *data,
        int data_length);
static void en50221_app_lowspeed_parse_command(struct en50221_app_lowspeed_private *private,
                                               uint8_t slot_id, uint16_t session_number,
                                               uint8_t *data, uint32_t data_length);
static void en50221_app_lowspeed_parse_send(struct en50221_app_lowspeed_private *private,
                                            uint8_t slot_id, uint16_t session_number, int last_more,
                                            uint8_t *data, uint32_t data_length);



en50221_app_lowspeed en50221_app_lowspeed_create(en50221_session_layer sl)
{
    struct en50221_app_lowspeed_private *private = NULL;

    // create structure and set it up
    private = malloc(sizeof(struct en50221_app_lowspeed_private));
    if (private == NULL) {
        return NULL;
    }
    private->sl = sl;
    private->command_callback = NULL;
    private->send_callback = NULL;

    // done
    return private;
}

void en50221_app_lowspeed_destroy(en50221_app_lowspeed lowspeed)
{
    struct en50221_app_lowspeed_private *private = (struct en50221_app_lowspeed_private *) lowspeed;

    free(private);
}

void en50221_app_lowspeed_register_command_callback(en50221_app_lowspeed lowspeed,
                                                    en50221_app_lowspeed_command_callback callback, void *arg)
{
    struct en50221_app_lowspeed_private *private = (struct en50221_app_lowspeed_private *) lowspeed;

    private->command_callback = callback;
    private->command_callback_arg = arg;
}

void en50221_app_lowspeed_register_send_callback(en50221_app_lowspeed lowspeed,
                                                 en50221_app_lowspeed_send_callback callback, void *arg)
{
    struct en50221_app_lowspeed_private *private = (struct en50221_app_lowspeed_private *) lowspeed;

    private->send_callback = callback;
    private->send_callback_arg = arg;
}

int en50221_app_lowspeed_send_comms_reply(en50221_app_lowspeed lowspeed,
                                          uint16_t session_number,
                                          uint8_t comms_reply_id,
                                          uint8_t return_value)
{
    struct en50221_app_lowspeed_private *private = (struct en50221_app_lowspeed_private *) lowspeed;
    uint8_t data[6];

    data[0] = (TAG_COMMS_REPLY >> 16) & 0xFF;
    data[1] = (TAG_COMMS_REPLY >> 8) & 0xFF;
    data[2] = TAG_COMMS_REPLY & 0xFF;
    data[3] = 2;
    data[4] = comms_reply_id;
    data[5] = return_value;
    return en50221_sl_send_data(private->sl, session_number, data, 6);
}

int en50221_app_lowspeed_send_comms_data(en50221_app_lowspeed lowspeed,
                                         uint16_t session_number,
                                         uint8_t phase_id,
                                         uint32_t tx_data_length,
                                         uint8_t *tx_data)
{
    struct en50221_app_lowspeed_private *private = (struct en50221_app_lowspeed_private *) lowspeed;
    uint8_t buf[10];

    // the spec defines this limit
    if (tx_data_length > 254) {
        return -1;
    }

    // set up the tag
    buf[0] = (TAG_COMMS_RECV_LAST >> 16) & 0xFF;
    buf[1] = (TAG_COMMS_RECV_LAST >> 8) & 0xFF;
    buf[2] = TAG_COMMS_RECV_LAST & 0xFF;

    // encode the length field
    int length_field_len;
    if ((length_field_len = asn_1_encode(tx_data_length+1, buf+3, 3)) < 0) {
        return -1;
    }

    // the phase_id
    buf[3+length_field_len] = phase_id;

    // build the iovecs
    struct iovec iov[2];
    iov[0].iov_base = buf;
    iov[0].iov_len = 3+length_field_len+1;
    iov[1].iov_base = tx_data;
    iov[1].iov_len = tx_data_length;

    // create the data and send it
    return en50221_sl_send_datav(private->sl, session_number, iov, 2);
}

int en50221_app_lowspeed_message(en50221_app_lowspeed lowspeed,
                                 uint8_t slot_id,
                                 uint16_t session_number,
                                 uint32_t resource_id,
                                 uint8_t *data, uint32_t data_length)
{
    struct en50221_app_lowspeed_private *private = (struct en50221_app_lowspeed_private *) lowspeed;
    (void)resource_id;

    // get the tag
    if (data_length < 3) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return -1;
    }
    uint32_t tag = (data[0] << 16) | (data[1] << 8) | data[2];

    switch(tag)
    {
        case TAG_COMMS_COMMAND:
            en50221_app_lowspeed_parse_command(private, slot_id, session_number, data+3, data_length-3);
            break;
        case TAG_COMMS_SEND_LAST:
            en50221_app_lowspeed_parse_send(private, slot_id, session_number, 1, data+3, data_length-3);
            break;
        case TAG_COMMS_SEND_MORE:
            en50221_app_lowspeed_parse_send(private, slot_id, session_number, 0, data+3, data_length-3);
            break;
        default:
            print(LOG_LEVEL, ERROR, 1, "Received unexpected tag %x\n", tag);
            return -1;
    }

    return 0;
}



static int en50221_app_lowspeed_parse_connect_on_channel(struct en50221_app_lowspeed_command *command,
                                                         uint8_t *data,
                                                         int data_length)
{
    if (data_length < 3) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return -1;
    }

    // check the tag
    uint32_t tag = (data[0] << 16) | (data[1] << 8) | data[2];
    if (tag != TAG_CONNECTION_DESCRIPTOR) {
        print(LOG_LEVEL, ERROR, 1, "Received bad CONNECT_ON_CHANNEL\n");
        return -1;
    }
    data+=3;
    data_length-=3;

    // parse the descriptor-length-field
    uint16_t asn_data_length;
    int length_field_len;
    if ((length_field_len = asn_1_decode(&asn_data_length, data, data_length)) < 0) {
        print(LOG_LEVEL, ERROR, 1, "ASN.1 decode error\n");
        return -1;
    }
    data+=length_field_len;
    data_length-=length_field_len;

    // check length field
    if (asn_data_length > data_length) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return -1;
    }
    if (asn_data_length < 1) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return -1;
    }

    // get the descriptor type
    command->u.connect_on_channel.descriptor_type = data[0];
    data++;
    data_length--;
    asn_data_length--;

    // deal with the descriptor itself
    switch(command->u.connect_on_channel.descriptor_type) {
    case CONNECTION_DESCRIPTOR_TYPE_TELEPHONE:
    {
        // get the raw descriptor and validate length
        struct descriptor *d = (struct descriptor*) data;
        if (asn_data_length < 2) {
            print(LOG_LEVEL, ERROR, 1, "Received short data\n");
            return -1;
        }
        if (asn_data_length != (2 + d->len)) {
            print(LOG_LEVEL, ERROR, 1, "Received short data\n");
            return -1;
        }
        if (d->tag != dtag_dvb_telephone) {
            print(LOG_LEVEL, ERROR, 1, "Received invalid telephone descriptor\n");
            return -1;
        }

        // parse the telephone descriptor
        command->u.connect_on_channel.descriptor.telephone = dvb_telephone_descriptor_codec(d);
        if (command->u.connect_on_channel.descriptor.telephone == NULL) {
            print(LOG_LEVEL, ERROR, 1, "Received invalid telephone descriptor\n");
            return -1;
        }
        data += 2 + d->len;
        data_length -= 2 + d->len;
        break;
    }
    case CONNECTION_DESCRIPTOR_TYPE_CABLE:
        if (asn_data_length != 1) {
            print(LOG_LEVEL, ERROR, 1, "Received short data\n");
            return -1;
        }
        command->u.connect_on_channel.descriptor.cable_channel_id = data[0];
        data++;
        data_length--;
        break;
    default:
        print(LOG_LEVEL, ERROR, 1, "Received unknown connection descriptor %02x\n",
                command->u.connect_on_channel.descriptor_type);
        return -1;
    }

    // parse the last bit
    if (data_length != 2) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return -1;
    }
    command->u.connect_on_channel.retry_count = data[0];
    command->u.connect_on_channel.timeout = data[1];

    // ok
    return 0;
}

static void en50221_app_lowspeed_parse_command(struct en50221_app_lowspeed_private *private,
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
    if (asn_data_length < 1) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    if (asn_data_length > (data_length-length_field_len)) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    data+=length_field_len;

    // get command id
    uint8_t command_id = data[0];
    data++;
    asn_data_length--;

    // parse the command
    struct en50221_app_lowspeed_command command;
    switch(command_id) {
        case COMMS_COMMAND_ID_CONNECT_ON_CHANNEL:
            if (en50221_app_lowspeed_parse_connect_on_channel(&command, data, asn_data_length)) {
                return;
            }
            break;
        case COMMS_COMMAND_ID_SET_PARAMS:
            if (asn_data_length != 2) {
                print(LOG_LEVEL, ERROR, 1, "Received short data\n");
                return;
            }
            command.u.set_params.buffer_size = data[0];
            command.u.set_params.timeout = data[1];
            break;
        case COMMS_COMMAND_ID_GET_NEXT_BUFFER:
            if (asn_data_length != 1) {
                print(LOG_LEVEL, ERROR, 1, "Received short data\n");
                return;
            }
            command.u.get_next_buffer.phase_id = data[0];
            break;

        case COMMS_COMMAND_ID_DISCONNECT_ON_CHANNEL:
        case COMMS_COMMAND_ID_ENQUIRE_STATUS:
            break;

        default:
            print(LOG_LEVEL, ERROR, 1, "Received unexpected command_id %02x\n", command_id);
            return;
    }

    // tell the app
    if (private->command_callback)
        private->command_callback(private->command_callback_arg, slot_id, session_number,
                                  command_id, &command);
}

static void en50221_app_lowspeed_parse_send(struct en50221_app_lowspeed_private *private,
                                            uint8_t slot_id, uint16_t session_number, int last_more,
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
    if (asn_data_length < 1) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    if (asn_data_length > (data_length-length_field_len)) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    uint8_t phase_id = data[1];

    // tell the app
    if (private->send_callback)
        private->send_callback(private->send_callback_arg, slot_id, session_number, phase_id, last_more,
                               data+2, asn_data_length-1);
}
