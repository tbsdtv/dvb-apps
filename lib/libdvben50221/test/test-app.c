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

#include <stdio.h>
#include <unistd.h>
#include <dvben50221/en50221_session.h>
#include <dvben50221/en50221_app_utils.h>
#include <dvben50221/en50221_app_ai.h>
#include <dvben50221/en50221_app_auth.h>
#include <dvben50221/en50221_app_ca.h>
#include <dvben50221/en50221_app_datetime.h>
#include <dvben50221/en50221_app_dvb.h>
#include <dvben50221/en50221_app_epg.h>
#include <dvben50221/en50221_app_lowspeed.h>
#include <dvben50221/en50221_app_mmi.h>
#include <dvben50221/en50221_app_rm.h>
#include <dvben50221/en50221_app_smartcard.h>
#include <dvben50221/en50221_app_teletext.h>
#include <dvbapi/dvbca.h>
#include <pthread.h>

void *stackthread_func(void* arg);
int test_lookup_callback(void *arg, uint8_t slot_id, uint32_t resource_id, en50221_sl_resource_callback *callback_out, void **arg_out);
int test_session_callback(void *arg, int reason, uint8_t slot_id, uint16_t session_number, uint32_t resource_id);

int test_datetime_enquiry_callback(void *arg, uint8_t slot_id, uint16_t session_number, uint8_t response_interval);

int test_rm_enq_callback(void *arg, uint8_t slot_id, uint16_t session_number);
int test_rm_reply_callback(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t resource_id_count, uint32_t *resource_ids);
int test_rm_changed_callback(void *arg, uint8_t slot_id, uint16_t session_number);

int test_ai_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                     uint8_t application_type, uint16_t application_manufacturer,
                     uint16_t manufacturer_code, uint8_t menu_string_length,
                     uint8_t *menu_string);

int test_ca_info_callback(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t ca_id_count, uint16_t *ca_ids);
int test_ca_pmt_reply_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                               struct en50221_app_pmt_reply *reply, uint32_t reply_size);


int shutdown_stackthread = 0;


struct resource {
    struct en50221_app_public_resource_id resid;
    en50221_sl_resource_callback callback;
    void *arg;
};
struct resource resources[20];
uint32_t resourceids[20];
int resources_count = 0;

en50221_app_rm rm_resource;
en50221_app_datetime datetime_resource;
en50221_app_ai ai_resource;
en50221_app_ca ca_resource;

/*
en50221_app_auth auth_resource;
en50221_app_dvb dvb_resource;
en50221_app_epg epg_resource;
en50221_app_lowspeed lowspeed_resource;
en50221_app_mmi mmi_resource;
en50221_app_smartcard smartcard_resource;
en50221_app_teletext teletext_resource;
*/


int main(int argc, char * argv[])
{
    int i;
    pthread_t stackthread;
    struct en50221_app_send_functions sendfuncs;

    // create transport layer
    en50221_transport_layer tl = en50221_tl_create(5, 32);
    if (tl == NULL) {
        fprintf(stderr, "Failed to create transport layer\n");
        exit(1);
    }

    // find CAMs
    int slot_count = 0;
    int cafd= -1;
    for(i=0; i<20; i++) {
        if ((cafd = dvbca_open(i, 0)) > 0) {
            if (dvbca_get_cam_state(cafd) == DVBCA_CAMSTATE_MISSING) {
                close(cafd);
                continue;
            }

            // reset it and wait
            dvbca_reset(cafd);
            printf("Found a CAM on adapter%i... waiting...\n", i);
            while(dvbca_get_cam_state(cafd) != DVBCA_CAMSTATE_READY) {
                usleep(1000);
            }

            // register it with the CA stack
            int slot_id = 0;
            if ((slot_id = en50221_tl_register_slot(tl, cafd, 1000, 100)) < 0) {
                fprintf(stderr, "Slot registration failed\n");
                exit(1);
            }
            printf("slotid: %i\n", slot_id);
            slot_count++;
        }
    }

    // create session layer
    en50221_session_layer sl = en50221_sl_create(tl, 256);
    if (sl == NULL) {
        fprintf(stderr, "Failed to create session layer\n");
        exit(1);
    }

    // create the sendfuncs
    sendfuncs.arg        = sl;
    sendfuncs.send_data  = en50221_sl_send_data;
    sendfuncs.send_datav = en50221_sl_send_datav;

    // create the resource manager resource
    rm_resource = en50221_app_rm_create(&sendfuncs);
    en50221_app_decode_public_resource_id(&resources[resources_count].resid, EN50221_APP_RM_RESOURCEID);
    resources[resources_count].callback = en50221_app_rm_message;
    resources[resources_count].arg = rm_resource;
    resourceids[resources_count] = EN50221_APP_RM_RESOURCEID;
    en50221_app_rm_register_enq_callback(rm_resource, test_rm_enq_callback, NULL);
    en50221_app_rm_register_reply_callback(rm_resource, test_rm_reply_callback, NULL);
    en50221_app_rm_register_changed_callback(rm_resource, test_rm_changed_callback, NULL);
    resources_count++;

    // create the datetime resource
    datetime_resource = en50221_app_datetime_create(&sendfuncs);
    en50221_app_decode_public_resource_id(&resources[resources_count].resid, EN50221_APP_DATETIME_RESOURCEID);
    resources[resources_count].callback = en50221_app_datetime_message;
    resources[resources_count].arg = datetime_resource;
    resourceids[resources_count] = EN50221_APP_DATETIME_RESOURCEID;
    en50221_app_datetime_register_enquiry_callback(datetime_resource, test_datetime_enquiry_callback, NULL);
    resources_count++;

    // create the application information resource
    ai_resource = en50221_app_ai_create(&sendfuncs);
    en50221_app_decode_public_resource_id(&resources[resources_count].resid, EN50221_APP_AI_RESOURCEID);
    resources[resources_count].callback = en50221_app_ai_message;
    resources[resources_count].arg = ai_resource;
    resourceids[resources_count] = EN50221_APP_AI_RESOURCEID;
    en50221_app_ai_register_callback(ai_resource, test_ai_callback, NULL);
    resources_count++;

    // create the CA resource
    ca_resource = en50221_app_ca_create(&sendfuncs);
    en50221_app_decode_public_resource_id(&resources[resources_count].resid, EN50221_APP_CA_RESOURCEID);
    resources[resources_count].callback = en50221_app_ca_message;
    resources[resources_count].arg = ca_resource;
    resourceids[resources_count] = EN50221_APP_CA_RESOURCEID;
    en50221_app_ca_register_info_callback(ca_resource, test_ca_info_callback, NULL);
    en50221_app_ca_register_pmt_reply_callback(ca_resource, test_ca_pmt_reply_callback, NULL);
    resources_count++;

    /*
    resourceids[resources_count+1] = EN50221_APP_AUTH_RESOURCEID;
    resourceids[resources_count+3] = EN50221_APP_DVB_RESOURCEID;
    resourceids[resources_count+4] = EN50221_APP_EPG_RESOURCEID(0);
    resourceids[resources_count+5] = EN50221_APP_MMI_RESOURCEID;
    resourceids[resources_count+6] = EN50221_APP_SMARTCARD_RESOURCEID(0);
    resourceids[resources_count+7] = EN50221_APP_TELETEXT_RESOURCEID;
    */



    // create the resources
//    auth_resource = en50221_app_auth_create(&sendfuncs);
//    ca_resource = en50221_app_ca_create(&sendfuncs);
//    dvb_resource = en50221_app_dvb_create(&sendfuncs);
//    epg_resource = en50221_app_epg_create(&sendfuncs);
//    lowspeed_resource = en50221_app_lowspeed_create(&sendfuncs);
//    mmi_resource = en50221_app_mmi_create(&sendfuncs);
//    smartcard_resource = en50221_app_smartcard_create(&sendfuncs);
//    teletext_resource = en50221_app_teletext_create(&sendfuncs);

    // start another thread running the stack
    pthread_create(&stackthread, NULL, stackthread_func, tl);

    // register callbacks
    en50221_sl_register_lookup_callback(sl, test_lookup_callback, sl);
    en50221_sl_register_session_callback(sl, test_session_callback, sl);

    // create a new connection
    for(i=0; i<slot_count; i++) {
        int tc = en50221_tl_new_tc(tl, i, 1);
        printf("tcid: %i\n", tc);
    }

    // wait
    printf("Press a key to exit\n");
    getchar();

    // destroy slots
    for(i=0; i<slot_count; i++) {
        en50221_tl_destroy_slot(tl, i);
    }
    shutdown_stackthread = 1;
    pthread_join(stackthread, NULL);

    // destroy session layer
    en50221_sl_destroy(sl);

    // destroy transport layer
    en50221_tl_destroy(tl);

    return 0;
}

int test_lookup_callback(void *arg, uint8_t slot_id, uint32_t resource_id, en50221_sl_resource_callback *callback_out, void **arg_out)
{
    struct en50221_app_public_resource_id resid;

    // decode the resource id
    if (en50221_app_decode_public_resource_id(&resid, resource_id)) {
        printf("Public resource lookup callback %i %i %i %i\n", slot_id,
               resid.resource_class, resid.resource_type, resid.resource_version);
    } else {
        printf("Private resource lookup callback %i %08x\n", slot_id, resource_id);
        return -1;
    }

    // try and find an instance of the resource
    int i;
    for(i=0; i<resources_count; i++) {
        if ((resid.resource_class == resources[i].resid.resource_class) &&
            (resid.resource_type == resources[i].resid.resource_type)) {
            *callback_out = resources[i].callback;
            *arg_out = resources[i].arg;
            return 0;
        }
    }

    return -1;
}

int test_session_callback(void *arg, int reason, uint8_t slot_id, uint16_t session_number, uint32_t resource_id)
{
    switch(reason) {
        case S_SCALLBACK_REASON_CAMCONNECTING:
            printf("CAM on slot %i connecting to resource %08x, session_number %i\n",
                   slot_id, resource_id, session_number);
            break;
        case S_SCALLBACK_REASON_CAMCONNECTED:
            printf("CAM on slot %i successfully connected to resource %08x, session_number %i\n",
                   slot_id, resource_id, session_number);

            if (resource_id == EN50221_APP_RM_RESOURCEID) {
                en50221_app_rm_enq(rm_resource, session_number);
            } else if (resource_id == EN50221_APP_AI_RESOURCEID) {
                en50221_app_ai_enquiry(ai_resource, session_number);
            } else if (resource_id == EN50221_APP_CA_RESOURCEID) {
                en50221_app_ca_info_enq(ca_resource, session_number);
            }

            break;
        case S_SCALLBACK_REASON_CAMCONNECTFAIL:
            printf("CAM on slot %i failed to connect to resource %08x\n", slot_id, resource_id);
            break;
        case S_SCALLBACK_REASON_CONNECTED:
            printf("Host connection to resource %08x on slot %i connected successfully, session_number %i\n",
                   resource_id, slot_id, session_number);
            break;
        case S_SCALLBACK_REASON_CONNECTFAIL:
            printf("Host connection to resource %08x on slot %i failed, session_number %i\n",
                   resource_id, slot_id, session_number);
            break;
        case S_SCALLBACK_REASON_CLOSE:
            printf("Connection on slot %i to resource %08x, session_number %i closed\n",
                   slot_id, resource_id, session_number);
            break;
        case S_SCALLBACK_REASON_TC_CONNECT:
            printf("Host originated transport connection %i on slot %i connected\n", session_number, slot_id);
            break;
        case S_SCALLBACK_REASON_TC_CAMCONNECT:
            printf("CAM originated transport connection %i on slot %i connected\n", session_number, slot_id);
            break;
    }
    return 0;
}



int test_rm_enq_callback(void *arg, uint8_t slot_id, uint16_t session_number)
{
    printf("RM: enq\n");

    if (en50221_app_rm_reply(rm_resource, session_number, resources_count, resourceids)) {
        printf("Failed to send reply to ENQ\n");
    }

    return 0;
}

int test_rm_reply_callback(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t resource_id_count, uint32_t *resource_ids)
{
    printf("RM: reply\n");
    int i;
    for(i=0; i< resource_id_count; i++) {
        printf("  %08x\n", resource_ids[i]);
    }


    if (en50221_app_rm_changed(rm_resource, session_number)) {
        printf("Failed to send REPLY\n");
    }

    return 0;
}

int test_rm_changed_callback(void *arg, uint8_t slot_id, uint16_t session_number)
{
    printf("RM: changed\n");

    if (en50221_app_rm_enq(rm_resource, session_number)) {
        printf("Failed to send ENQ\n");
    }

    return 0;
}



int test_datetime_enquiry_callback(void *arg, uint8_t slot_id, uint16_t session_number, uint8_t response_interval)
{
    printf("Datetime: enquiry\n");
    printf("  response_interval:%i\n", response_interval);

    if (en50221_app_datetime_send(datetime_resource, session_number, time(NULL), -1)) {
        printf("failed to send datetime\n");
    }

    // FIXME: need to do response interval stuff

    return 0;
}



int test_ai_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                     uint8_t application_type, uint16_t application_manufacturer,
                     uint16_t manufacturer_code, uint8_t menu_string_length,
                     uint8_t *menu_string)
{
    printf("AI: info\n");
    printf("  Application type: %02x\n", application_type);
    printf("  Application manufacturer: %04x\n", application_manufacturer);
    printf("  Manufacturer code: %04x\n", manufacturer_code);
    printf("  Menu string: %*s\n", menu_string_length, menu_string);

    return 0;
}



int test_ca_info_callback(void *arg, uint8_t slot_id, uint16_t session_number, uint32_t ca_id_count, uint16_t *ca_ids)
{
    printf("CA: info\n");
    int i;
    for(i=0; i< ca_id_count; i++) {
        printf("  %04x\n", ca_ids[i]);
    }

    return 0;
}

int test_ca_pmt_reply_callback(void *arg, uint8_t slot_id, uint16_t session_number,
                               struct en50221_app_pmt_reply *reply, uint32_t reply_size)
{
    printf("CA: PMT reply\n");

    return 0;
}






void *stackthread_func(void* arg) {
    en50221_transport_layer tl = arg;
    int lasterror = 0;

    while(!shutdown_stackthread) {
        int error;
        if ((error = en50221_tl_poll(tl)) != 0) {
            if (error != lasterror) {
                fprintf(stderr, "Error reported by stack slot:%i error:%i\n",
                        en50221_tl_get_error_slot(tl),
                        en50221_tl_get_error(tl));
            }
            lasterror = error;
        }
    }

    shutdown_stackthread = 0;
    return 0;
}
