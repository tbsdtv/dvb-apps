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
#include "en50221_app_rm.h"
#include "asn_1.h"

// tags supported by this resource
#define TAG_PROFILE_ENQUIRY             0x9f8010
#define TAG_PROFILE                     0x9f8011
#define TAG_PROFILE_CHANGE              0x9f8012

struct en50221_resource {
        struct en50221_app_public_resource_id id;

        en50221_sl_resource_callback callback;
        void *callback_arg;
};

struct en50221_session_info {
        int slot_id;
        uint32_t *resources;
        int resources_count;
};

struct en50221_app_rm_private {
        struct en50221_resource *resources;
        int resources_count;
        en50221_session_layer *sl;

        struct en50221_session_info *sessions;
        uint32_t max_sessions;
};


static int en50221_app_rm_lookup(void *arg, uint32_t resource_id,
                                 en50221_sl_resource_callback *callback_out, void**arg_out);
static int en50221_app_rm_resource_callback(void *arg,
                                            int reason,
                                            uint8_t slot_id,
                                            uint16_t session_number,
                                            uint32_t resource_id,
                                            uint8_t *data, uint32_t data_length);



en50221_app_rm en50221_app_rm_create(en50221_session_layer *sl)
{
    struct en50221_app_rm_private *private = NULL;

    // create structure and set it up
    private = malloc(sizeof(struct en50221_app_rm_private));
    if (private == NULL) {
        return NULL;
    }
    private->resources = NULL;
    private->resources_count = 0;
    private->sl = sl;
    private->max_sessions = en50221_sl_get_max_sessions(sl);

    // setup the session-specific information
    private->sessions = malloc(sizeof(struct en50221_session_info) * private->max_sessions);
    if (private->sessions == NULL) {
        free(private);
        return NULL;
    }
    memset(private->sessions, 0, sizeof(struct en50221_session_info) * private->max_sessions);

    // register with... ourself!
    if (en50221_app_rm_register((en50221_app_rm) private, MKRID(1,1,1), en50221_app_rm_resource_callback, private)) {
        free(private->resources);
        free(private);
        return NULL;
    }

    // register lookup callback with the session layer
    en50221_sl_register_lookup_callback(sl, en50221_app_rm_lookup, private);

    // done
    return private;
}

void en50221_app_rm_destroy(en50221_app_rm *rm)
{
    struct en50221_app_rm_private *private = (struct en50221_app_rm_private *) rm;

    // deregister callback
    en50221_sl_register_lookup_callback(private->sl, NULL, NULL);

    // free structure
    if (private->resources)
        free(private->resources);

    if (private->sessions) {
        uint32_t i;
        for(i=0; i< private->max_sessions; i++) {
            if (private->sessions[i].resources)
                free(private->sessions[i].resources);
        }
        free(private->sessions);
    }
    free(private);
}

int en50221_app_rm_register(en50221_app_rm *rm, uint32_t resource_id,
                            en50221_sl_resource_callback callback, void *arg)
{
    struct en50221_app_rm_private *private = (struct en50221_app_rm_private *) rm;
    struct en50221_app_public_resource_id new_id;

    // parse the resource id
    if (en50221_app_decode_public_resource_id(&new_id, resource_id) == NULL) {
        return -1;
    }

    // allocate new resources array
    struct en50221_resource *resources_new =
            realloc(private->resources, (private->resources_count + 1) * sizeof(struct en50221_resource));
    if (resources_new == NULL) {
        return -1;
    }
    private->resources = resources_new;

    // set everything up
    private->resources[private->resources_count].id.resource_class = new_id.resource_class;
    private->resources[private->resources_count].id.resource_type = new_id.resource_type;
    private->resources[private->resources_count].id.resource_version = new_id.resource_version;
    private->resources[private->resources_count].callback = callback;
    private->resources[private->resources_count].callback_arg = arg;
    private->resources_count++;

    // broadcast a profile change message
    uint8_t pc_data[3];
    pc_data[0] = (TAG_PROFILE_CHANGE >> 16) & 0xFF;
    pc_data[1] = (TAG_PROFILE_CHANGE >> 8) & 0xFF;
    pc_data[2] = TAG_PROFILE_CHANGE & 0xFF;
    en50221_sl_broadcast_data(private, -1, MKRID(1,1,1), pc_data, 3);

    // success
    return 0;
}

int en50221_app_rm_get_supported_resources(en50221_app_rm *rm, uint8_t slot_id, uint32_t **resources)
{
    struct en50221_app_rm_private *private = (struct en50221_app_rm_private *) rm;
    int i;

    for(i=0; i< private->resources_count; i++) {
        if (private->sessions[i].slot_id == slot_id) {
            *resources = private->sessions[i].resources;
            return private->sessions[i].resources_count;
        }
    }

    return -1;
}







static int en50221_app_rm_lookup(void *arg, uint32_t resource_id,
                                 en50221_sl_resource_callback *callback_out, void**arg_out)
{
    struct en50221_app_rm_private *private = (struct en50221_app_rm_private *) arg;
    struct en50221_app_public_resource_id search_id;
    int i;

    // decode the supplied id
    if (en50221_app_decode_public_resource_id(&search_id, resource_id) == NULL) {
        // it was a private ID which we do not support... for the moment anyway
        return -1;
    }

    // search for it
    for(i=0; i < private->resources_count; i++) {
        if ((private->resources[i].id.resource_class == search_id.resource_class) &&
            (private->resources[i].id.resource_type == search_id.resource_type)) {

            // resource version is less than expected
            if (private->resources[i].id.resource_version < search_id.resource_version) {
                return -2;
            }

            // we're suppose to wait for a profile change message before allowing anything because we're
            // meant to have a global list of resources. As we have a slot-specific list, we don't need to
            // bother doing this.

            // all was ok
            *callback_out = private->resources[i].callback;
            *arg_out = private->resources[i].callback_arg;
            return 0;
        }
    }

    // didn't find it
    return -1;
}

static void en50221_app_rm_handle_incoming_profile(struct en50221_app_rm_private *private,
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
    int resources_count = asn_data_length / 4;

    // check the session_number
    if (session_number >= private->max_sessions) {
        print(LOG_LEVEL, ERROR, 1, "Received bad session number\n");
        return;
    }

    // free up any existing resources
    if (private->sessions[session_number].resources)
        free(private->sessions[session_number].resources);
    private->sessions[session_number].resources_count = 0;

    // allocate new memory for 'em and copy them into it
    private->sessions[session_number].resources = malloc(resources_count*4);
    if (private->sessions[session_number].resources == NULL) {
        print(LOG_LEVEL, ERROR, 1, "Out of memory\n");
        return;
    }
    memcpy(private->sessions[session_number].resources, data+length_field_len, resources_count*4);
    private->sessions[session_number].resources_count = resources_count;
    private->sessions[session_number].slot_id = slot_id;

    // after we registered the resources the cam supports. Now we should send an
    // Profile Change on all sessions on this slot.
    uint8_t pc_data[3];
    pc_data[0] = (TAG_PROFILE_CHANGE >> 16) & 0xFF;
    pc_data[1] = (TAG_PROFILE_CHANGE >> 8) & 0xFF;
    pc_data[2] = TAG_PROFILE_CHANGE & 0xFF;
    en50221_sl_broadcast_data(private, slot_id, MKRID(1,1,1), pc_data, 3);
}

static void en50221_app_rm_send_profile_reply(struct en50221_app_rm_private *private, uint8_t session_number)
{
    // first of all, encode the length field
    uint8_t length_field[3];
    int length_field_len;
    if ((length_field_len = asn_1_encode((private->resources_count*4), length_field, 3)) < 0) {
        print(LOG_LEVEL, ERROR, 1, "ASN.1 encode error\n");
        return;
    }

    // now allocate memory for the reply
    uint16_t data_length = 3+length_field_len+(private->resources_count*4);
    uint8_t *data = malloc(data_length);
    if (data == NULL) {
        print(LOG_LEVEL, ERROR, 1, "Out of memory\n");
        return;
    }

    // create the data
    data[0] = (TAG_PROFILE >> 16) & 0xFF;
    data[1] = (TAG_PROFILE >> 8) & 0xFF;
    data[2] = TAG_PROFILE & 0xFF;
    memcpy(data+3, length_field, length_field_len);

    // now append the resources
    uint16_t offset = 3+length_field_len;
    int i;
    for(i = 0; i < private->resources_count; i++)
    {
        en50221_app_encode_public_resource_id(&private->resources[i].id, data+offset);
        offset+=4;
    }

    // sendit
    if (en50221_sl_send_data(private->sl, session_number, data, data_length)) {
        print(LOG_LEVEL, ERROR, 1, "Session layer reports error %i\n", en50221_sl_get_error(private->sl));
    }

    // cleanup
    free(data);
}

static void en50221_app_rm_enquiry(struct en50221_app_rm_private *private, uint8_t session_number)
{
    char data[3];

    data[0] = (TAG_PROFILE_ENQUIRY >> 16) & 0xFF;
    data[1] = (TAG_PROFILE_ENQUIRY >> 8) & 0xFF;
    data[2] = TAG_PROFILE_ENQUIRY & 0xFF;

    if (en50221_sl_send_data(private->sl, session_number, data, 3)) {
        print(LOG_LEVEL, ERROR, 1, "Session layer reports error %i\n", en50221_sl_get_error(private->sl));
    }
}

static int en50221_app_rm_resource_callback(void *arg,
                                            int reason,
                                            uint8_t slot_id,
                                            uint16_t session_number,
                                            uint32_t resource_id,
                                            uint8_t *data, uint32_t data_length)
{
    struct en50221_app_rm_private *private = (struct en50221_app_rm_private *) arg;

    // deal with the reason
    switch(reason) {
    case S_CALLBACK_REASON_CONNECTING:
        return 0;

    case S_CALLBACK_REASON_CONNECTED:
        en50221_app_rm_enquiry(private, session_number);
        return 0;

    case S_CALLBACK_REASON_DATA:
        // fallthrough into function
        break;

    case S_CALLBACK_REASON_CLOSE:
        if (session_number <= private->max_sessions) {
            if (private->sessions[session_number].resources) {
                free(private->sessions[session_number].resources);
            }
            private->sessions[session_number].slot_id = -1;
            private->sessions[session_number].resources = NULL;
            private->sessions[session_number].resources_count = 0;
        }

        return 0;
    }

    // get the tag
    if (data_length < 3) {
        print(LOG_LEVEL, ERROR, 1, "Received short data\n");
        return -1;
    }
    uint32_t tag = (data[0] << 16) | (data[1] << 8) | data[2];

    // dispatch it
    switch(tag)
    {
        case TAG_PROFILE_ENQUIRY:
            en50221_app_rm_send_profile_reply(private, session_number);
            break;
        case TAG_PROFILE:
            en50221_app_rm_handle_incoming_profile(private, slot_id, session_number, data+3, data_length-3);
            break;
        case TAG_PROFILE_CHANGE:
            en50221_app_rm_enquiry(private, session_number);
            break;
        default:
            print(LOG_LEVEL, ERROR, 1, "Received unexpected tag %x\n", tag);
            return -1;
    }

    return 0;
}
