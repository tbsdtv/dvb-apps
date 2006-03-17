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
#include "en50221_app_teletext.h"
#include "asn_1.h"

// tags supported by this resource
#define TAG_TELETEXT_EBU      0x9f9000

struct en50221_app_teletext_private {
        en50221_session_layer *sl;

        en50221_app_teletext_callback callback;
        void *callback_arg;
};

static void en50221_app_teletext_resource_callback(void *arg,
                                                   uint8_t slot_id,
                                                   uint16_t session_number,
                                                   uint32_t resource_id,
                                                   uint8_t *data, uint32_t data_length);



en50221_app_teletext en50221_app_teletext_create(en50221_session_layer sl, en50221_app_rm rm)
{
    struct en50221_app_teletext_private *private = NULL;

    // create structure and set it up
    private = malloc(sizeof(struct en50221_app_teletext_private));
    if (private == NULL) {
        return NULL;
    }
    private->sl = sl;
    private->callback = NULL;

    // register with the RM
    if (en50221_app_rm_register(rm, MKRID(128,1,1), en50221_app_teletext_resource_callback, private)) {
        free(private);
        return NULL;
    }

    // done
    return private;
}

void en50221_app_teletext_destroy(en50221_app_teletext teletext)
{
    struct en50221_app_teletext_private *private = (struct en50221_app_teletext_private *) teletext;

    free(private);
}

void en50221_app_teletext_register_callback(en50221_app_teletext teletext,
                                            en50221_app_teletext_callback callback, void *arg)
{
    struct en50221_app_teletext_private *private = (struct en50221_app_teletext_private *) teletext;

    private->callback = callback;
    private->callback_arg = arg;
}


static void en50221_app_teletext_parse_ebu(struct en50221_app_teletext_private *private,
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
    if (asn_data_length > data_length) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    uint8_t *teletext_data = data + length_field_len;

    // tell the app
    if (private->callback)
        private->callback(private->callback_arg, slot_id, session_number,
                          teletext_data, asn_data_length);
}

static void en50221_app_teletext_resource_callback(void *arg,
                                                   uint8_t slot_id,
                                                   uint16_t session_number,
                                                   uint32_t resource_id,
                                                   uint8_t *data, uint32_t data_length)
{
    struct en50221_app_teletext_private *private = (struct en50221_app_teletext_private *) arg;
    (void) resource_id;

    // get the tag
    if (data_length < 3) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return;
    }
    uint32_t tag = (data[0] << 16) | (data[1] << 8) | data[2];

    switch(tag)
    {
        case TAG_TELETEXT_EBU:
            en50221_app_teletext_parse_ebu(private, slot_id, session_number, data+3, data_length-3);
            break;
        default:
            print(LOG_LEVEL, ERROR, 1, "Received unexpected tag %x\n", tag);
            break;
    }
}
