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
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <time.h>
#include <dvbmisc.h>
#include <sys/uio.h>
#include <pthread.h>
#include "en50221_transport.h"
#include "en50221_session.h"
#include "en50221_errno.h"
#include "asn_1.h"


// these are the possible session statuses
#define S_STATUS_OPEN                    0x00  // session is opened
#define S_STATUS_CLOSE_NO_RES            0xF0  // could not open session, no proper resource available
#define S_STATUS_CLOSE_RES_UNAVAILABLE   0xF1  // could not open session, resource unavailable
#define S_STATUS_CLOSE_RES_LOW_VERSION   0xF2  // could not open session, resource version too low
#define S_STATUS_CLOSE_RES_BUSY          0xF3  // could not open session, resource is busy

#define ST_OPEN_SESSION_REQ     0x91    // h<--m
#define ST_OPEN_SESSION_RES     0x92    // h-->m
#define ST_CREATE_SESSION       0x93    // h-->m
#define ST_CREATE_SESSION_RES   0x94    // h<--m
#define ST_CLOSE_SESSION_REQ    0x95    // h<->m
#define ST_CLOSE_SESSION_RES    0x96    // h<->m
#define ST_SESSION_NUMBER       0x90    // h<->m

#define S_STATE_IDLE            0x01    // this session is not in use
#define S_STATE_ACTIVE          0x02    // this session is in use
#define S_STATE_IN_CREATION     0x04    // this session waits for a ST_CREATE_SESSION_RES to become active
#define S_STATE_IN_DELETION     0x08    // this session waits for ST_CLOSE_SESSION_RES to become idle again


// for each session we store its identifier, the resource-id
// it is linked to and the callback of the specific resource
struct en50221_session {
    uint8_t state;
    uint32_t resource_id;
    uint8_t slot_id;
    uint8_t connection_id;

    en50221_sl_resource_callback callback;
    void *callback_arg;

    pthread_mutex_t session_lock;
};

struct en50221_session_layer_private
{
    uint32_t max_sessions;
    en50221_transport_layer tl;

    en50221_sl_lookup_callback lookup;
    void *lookup_arg;

    en50221_sl_session_callback session;
    void *session_arg;

    pthread_mutex_t global_lock;
    pthread_mutex_t setcallback_lock;

    int error;

    struct en50221_session *sessions;
};

static void en50221_sl_transport_callback(void *arg, int reason, uint8_t *data, uint32_t data_length,
                                          uint8_t slot_id, uint8_t connection_id);
static int en50221_sl_alloc_new_session(struct en50221_session_layer_private *private,
                                        uint32_t resource_id,
                                        uint8_t slot_id,
                                        uint8_t connection_id,
                                        en50221_sl_resource_callback callback, void* arg);




en50221_session_layer en50221_sl_create(en50221_transport_layer tl,
                                        uint32_t max_sessions)
{
    struct en50221_session_layer_private *private = NULL;
    uint32_t i;

    // setup structure
    private = (struct en50221_session_layer_private*) malloc(sizeof(struct en50221_session_layer_private));
    if (private == NULL)
        goto error_exit;
    private->max_sessions = max_sessions;
    private->lookup = NULL;
    private->session = NULL;
    private->tl = tl;
    private->error = 0;

    // init the mutex
    pthread_mutex_init(&private->global_lock, NULL);
    pthread_mutex_init(&private->setcallback_lock, NULL);

    // create the slots
    private->sessions = malloc(sizeof(struct en50221_session) * max_sessions);
    if (private->sessions == NULL)
        goto error_exit;

    // set them up
    for(i=0; i< max_sessions; i++) {
        private->sessions[i].state = S_STATE_IDLE;
        private->sessions[i].callback = NULL;

        pthread_mutex_init(&private->sessions[i].session_lock, NULL);
    }

    // register ourselves with the transport layer
    en50221_tl_register_callback(tl, en50221_sl_transport_callback, private);

    return private;

error_exit:
    en50221_sl_destroy(private);
    return NULL;
}

void en50221_sl_destroy(en50221_session_layer sl)
{
    struct en50221_session_layer_private *private = (struct en50221_session_layer_private *) sl;
    uint32_t i;

    if (private) {
        if (private->sessions) {
            for(i=0; i< private->max_sessions; i++) {
                pthread_mutex_destroy(&private->sessions[i].session_lock);
            }
            free(private->sessions);
        }

        pthread_mutex_destroy(&private->setcallback_lock);
        pthread_mutex_destroy(&private->global_lock);

        free(private);
    }
}

int en50221_sl_get_error(en50221_session_layer tl)
{
    struct en50221_session_layer_private *private = (struct en50221_session_layer_private *) tl;
    return private->error;
}

void en50221_sl_register_lookup_callback(en50221_session_layer sl, en50221_sl_lookup_callback callback, void *arg)
{
    struct en50221_session_layer_private *private = (struct en50221_session_layer_private *) sl;

    pthread_mutex_lock(&private->setcallback_lock);
    private->lookup = callback;
    private->lookup_arg = arg;
    pthread_mutex_unlock(&private->setcallback_lock);
}

void en50221_sl_register_session_callback(en50221_session_layer sl,
                                          en50221_sl_session_callback callback, void *arg)
{
    struct en50221_session_layer_private *private = (struct en50221_session_layer_private *) sl;

    pthread_mutex_lock(&private->setcallback_lock);
    private->session = callback;
    private->session_arg = arg;
    pthread_mutex_unlock(&private->setcallback_lock);
}

int en50221_sl_create_session(en50221_session_layer sl, int slot_id, uint8_t connection_id, uint32_t resource_id,
                              en50221_sl_resource_callback callback, void* arg)
{
    struct en50221_session_layer_private *private = (struct en50221_session_layer_private *) sl;

    // lookup next free session_id:
    pthread_mutex_lock(&private->global_lock);
    int session_number = en50221_sl_alloc_new_session(private, resource_id, slot_id, connection_id, callback, arg);
    if (session_number == -1) {
        pthread_mutex_unlock(&private->global_lock);
        return -1;
    }
    pthread_mutex_unlock(&private->global_lock);

    // make up the header
    uint8_t hdr[8];
    hdr[0] = ST_CREATE_SESSION;
    hdr[1] = 6;
    hdr[2] = resource_id >> 24;
    hdr[3] = resource_id >> 16;
    hdr[4] = resource_id >> 8;
    hdr[5] = resource_id;
    hdr[6] = session_number >> 8;
    hdr[7] = session_number;

    // send this command
    if (en50221_tl_send_data(private->tl, slot_id, connection_id, hdr, 8)) {
        pthread_mutex_lock(&private->sessions[session_number].session_lock);
        if (private->sessions[session_number].state == S_STATE_IN_CREATION) {
            private->sessions[session_number].state = S_STATE_IDLE;
        }
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);

        private->error = en50221_tl_get_error(private->tl);
        return -1;
    }

    // ok.
    return session_number;
}

int en50221_sl_destroy_session(en50221_session_layer sl, uint16_t session_number)
{
    struct en50221_session_layer_private *private = (struct en50221_session_layer_private *) sl;

    if (session_number >= private->max_sessions) {
        private->error = EN50221ERR_BADSESSIONNUMBER;
        return -1;
    }

    pthread_mutex_lock(&private->sessions[session_number].session_lock);
    if (!(private->sessions[session_number].state & (S_STATE_ACTIVE|S_STATE_IN_DELETION))) {
        private->error = EN50221ERR_BADSESSIONNUMBER;
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return -1;
    }

    // set the state
    private->sessions[session_number].state = S_STATE_IN_DELETION;

    // get essential details
    uint8_t slot_id = private->sessions[session_number].slot_id;
    uint8_t connection_id = private->sessions[session_number].connection_id;
    pthread_mutex_unlock(&private->sessions[session_number].session_lock);

    //  sendit
    uint8_t hdr[4];
    hdr[0] = ST_CLOSE_SESSION_REQ;
    hdr[1] = 2;
    hdr[2] = session_number >> 8;
    hdr[3] = session_number;
    if (en50221_tl_send_data(private->tl, slot_id, connection_id, hdr, 4)) {
        pthread_mutex_lock(&private->sessions[session_number].session_lock);
        if (private->sessions[session_number].state == S_STATE_IN_DELETION) {
            private->sessions[session_number].state = S_STATE_IDLE;
        }
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);

        private->error = en50221_tl_get_error(private->tl);
        return -1;
    }

    return 0;
}

int en50221_sl_send_data(en50221_session_layer sl, uint16_t session_number, uint8_t *data, uint16_t data_length)
{
    struct en50221_session_layer_private *private = (struct en50221_session_layer_private *) sl;

    if (session_number >= private->max_sessions) {
        private->error = EN50221ERR_BADSESSIONNUMBER;
        return -1;
    }

    pthread_mutex_lock(&private->sessions[session_number].session_lock);
    if (private->sessions[session_number].state != S_STATE_ACTIVE) {
        private->error = EN50221ERR_BADSESSIONNUMBER;
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return -1;
    }

    // get essential details
    uint8_t slot_id = private->sessions[session_number].slot_id;
    uint8_t connection_id = private->sessions[session_number].connection_id;
    pthread_mutex_unlock(&private->sessions[session_number].session_lock);

    // sendit
    struct iovec iov[2];
    uint8_t hdr[4];
    hdr[0] = ST_SESSION_NUMBER;
    hdr[1] = 2;
    hdr[2] = session_number >> 8;
    hdr[3] = session_number;
    iov[0].iov_base = hdr;
    iov[0].iov_len = 4;
    iov[1].iov_base = data;
    iov[1].iov_len = data_length;
    if (en50221_tl_send_datav(private->tl, slot_id, connection_id, iov, 2)) {
        private->error = en50221_tl_get_error(private->tl);
        return -1;
    }

    return 0;
}

int en50221_sl_send_datav(en50221_session_layer sl, uint16_t session_number,
                          struct iovec *vector, int iov_count)
{
    struct en50221_session_layer_private *private = (struct en50221_session_layer_private *) sl;

    if (session_number >= private->max_sessions) {
        private->error = EN50221ERR_BADSESSIONNUMBER;
        return -1;
    }

    pthread_mutex_lock(&private->sessions[session_number].session_lock);
    if (private->sessions[session_number].state != S_STATE_ACTIVE) {
        private->error = EN50221ERR_BADSESSIONNUMBER;
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return -1;
    }
    if (iov_count > 9) {
        private->error = EN50221ERR_IOVLIMIT;
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return -1;
    }
    uint8_t slot_id = private->sessions[session_number].slot_id;
    uint8_t connection_id = private->sessions[session_number].connection_id;
    pthread_mutex_unlock(&private->sessions[session_number].session_lock);

    // make up the header
    struct iovec out_iov[10];
    uint8_t hdr[4];
    hdr[0] = ST_SESSION_NUMBER;
    hdr[1] = 2;
    hdr[2] = session_number >> 8;
    hdr[3] = session_number;
    out_iov[0].iov_base = hdr;
    out_iov[0].iov_len = 4;

    // make up the data
    memcpy(&out_iov[1], vector, iov_count * sizeof(struct iovec));

    // send this command
    if (en50221_tl_send_datav(private->tl, slot_id, connection_id, out_iov, iov_count+1)) {
        private->error = en50221_tl_get_error(private->tl);
        return -1;
    }
    return 0;
}

int en50221_sl_broadcast_data(en50221_session_layer sl, int slot_id, uint32_t resource_id,
                              uint8_t *data, uint16_t data_length)
{
    struct en50221_session_layer_private *private = (struct en50221_session_layer_private *) sl;
    uint32_t i;

    for(i = 0; i < private->max_sessions; i++)
    {
        pthread_mutex_lock(&private->sessions[i].session_lock);

        if (private->sessions[i].state != S_STATE_ACTIVE) {
            pthread_mutex_unlock(&private->sessions[i].session_lock);
            continue;
        }
        if ((slot_id != -1) && (slot_id != private->sessions[i].slot_id)) {
            pthread_mutex_unlock(&private->sessions[i].session_lock);
            continue;
        }

        if (private->sessions[i].resource_id == resource_id) {
            pthread_mutex_unlock(&private->sessions[i].session_lock);
            en50221_sl_send_data(sl, i, data, data_length);
        } else {
            pthread_mutex_unlock(&private->sessions[i].session_lock);
        }
    }

    return 0;
}



static void en50221_sl_handle_open_session_request(struct en50221_session_layer_private *private,
                                     uint8_t *data, uint32_t data_length, uint8_t slot_id, uint8_t connection_id)
{
    // check
    if (data_length < 5) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        return;
    }
    if (data[0] != 4) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        return;
    }

    // get the resource id
    uint32_t resource_id = (data[1] << 24) | (data[2] << 16) | (data[3] << 8) | data[4];

    // get lookup callback details
    pthread_mutex_lock(&private->setcallback_lock);
    en50221_sl_lookup_callback lcb = private->lookup;
    void *lcb_arg = private->lookup_arg;
    pthread_mutex_unlock(&private->setcallback_lock);

    // first of all, lookup this resource id
    int status = S_STATUS_CLOSE_NO_RES;
    en50221_sl_resource_callback resource_callback = NULL;
    void *resource_arg = NULL;
    if (lcb) {
        status = lcb(lcb_arg, slot_id, resource_id, &resource_callback, &resource_arg);
        switch(status) {
        case 0:
            status = S_STATUS_OPEN;
            break;

        case -1:
            status = S_STATUS_CLOSE_NO_RES;
            break;

        case -2:
            status = S_STATUS_CLOSE_RES_LOW_VERSION;
            break;

        case -3:
            status = S_STATUS_CLOSE_RES_UNAVAILABLE;
            break;
        }
    }

    // if we found it, get a new session for it
    int session_number = -1;
    if (status == S_STATUS_OPEN) {
        // lookup next free session_id:
        pthread_mutex_lock(&private->global_lock);
        session_number = en50221_sl_alloc_new_session(private, resource_id, slot_id, connection_id,
                                                      resource_callback, resource_arg);
        pthread_mutex_unlock(&private->global_lock);

        if (session_number == -1) {
            status = S_STATUS_CLOSE_NO_RES;
        } else {
            // inform upper layers/ check availability
            pthread_mutex_lock(&private->setcallback_lock);
            en50221_sl_session_callback cb = private->session;
            void *cb_arg = private->session_arg;
            pthread_mutex_unlock(&private->setcallback_lock);
            if (cb) {
                if (cb(cb_arg, S_SCALLBACK_REASON_CAMCONNECTING, slot_id, session_number, resource_id)) {
                    status = S_STATUS_CLOSE_RES_BUSY;
                }
            } else {
                status = S_STATUS_CLOSE_RES_UNAVAILABLE;
            }
        }
    }

    // send response
    uint8_t hdr[9];
    hdr[0] = ST_OPEN_SESSION_RES;
    hdr[1] = 7;
    hdr[2] = status;
    hdr[3] = resource_id >> 24;
    hdr[4] = resource_id >> 16;
    hdr[5] = resource_id >> 8;
    hdr[6] = resource_id;
    hdr[7] = session_number >> 8;
    hdr[8] = session_number;
    if (en50221_tl_send_data(private->tl, slot_id, connection_id, hdr, 9)) {
        print(LOG_LEVEL, ERROR, 1, "Transport layer error %i occurred\n", en50221_tl_get_error(private->tl));
        status = S_STATUS_CLOSE_NO_RES;
        // fallthrough
    }

    // inform upper layers what happened
    if (session_number != -1) {
        // setup session state apppropriately from upper layer response
        pthread_mutex_lock(&private->sessions[session_number].session_lock);
        if (status != S_STATUS_OPEN) {
            private->sessions[session_number].state = S_STATE_IDLE;
        } else {
            private->sessions[session_number].state = S_STATE_ACTIVE;
        }
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);

        // tell upper layers
        if (private->sessions[session_number].state == S_STATE_ACTIVE) {
            pthread_mutex_lock(&private->setcallback_lock);
            en50221_sl_session_callback cb = private->session;
            void *cb_arg = private->session_arg;
            pthread_mutex_unlock(&private->setcallback_lock);

            if (status == S_STATUS_OPEN) {
                if (cb)
                    cb(cb_arg, S_SCALLBACK_REASON_CAMCONNECTED, slot_id, session_number, resource_id);
            } else {
                private->sessions[session_number].state = S_STATE_IDLE;
                if (cb)
                    cb(cb_arg, S_SCALLBACK_REASON_CAMCONNECTFAIL, slot_id, session_number, resource_id);
            }
        }
    }
}

static void en50221_sl_handle_close_session_request(struct en50221_session_layer_private *private,
                                      uint8_t *data, uint32_t data_length, uint8_t slot_id, uint8_t connection_id)
{
    // check
    if (data_length < 3) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        return;
    }
    if (data[0] != 2) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        return;
    }

    // extract session number
    uint16_t session_number = (data[1] << 8) | data[2];

    // check session number is ok
    uint8_t code = 0x00;
    uint32_t resource_id = 0;
    if (session_number >= private->max_sessions) {
        code = 0xF0; // session close error
        print(LOG_LEVEL, ERROR, 1, "Received bad session id %i\n", slot_id);
    } else {
        pthread_mutex_lock(&private->sessions[session_number].session_lock);
        if (slot_id != private->sessions[session_number].slot_id) {
            print(LOG_LEVEL, ERROR, 1, "Received unexpected session on invalid slot %i\n", slot_id);
            code = 0xF0; // session close error
        }
        if (connection_id != private->sessions[session_number].connection_id) {
            print(LOG_LEVEL, ERROR, 1, "Received unexpected session on invalid slot %i\n", slot_id);
            code = 0xF0; // session close error
        }
        if (!(private->sessions[session_number].state & (S_STATE_ACTIVE|S_STATE_IN_DELETION))) {
            print(LOG_LEVEL, ERROR, 1, "Received unexpected session on invalid slot %i\n", slot_id);
            code = 0xF0; // session close error
        }

        if (code == 0x00) {
            private->sessions[session_number].state = S_STATE_IDLE;
            code = 0x00; // close ok
        }
        resource_id = private->sessions[session_number].resource_id;
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
    }

    // make up the response
    uint8_t hdr[5];
    hdr[0] = ST_CLOSE_SESSION_RES;
    hdr[1] = 3;
    hdr[2] = code;
    hdr[3] = session_number >> 8;
    hdr[4] = session_number;

    // sendit
    if (en50221_tl_send_data(private->tl, slot_id, connection_id, hdr, 5)) {
        print(LOG_LEVEL, ERROR, 1, "Transport layer reports error %i on slot %i\n",
              en50221_tl_get_error(private->tl), slot_id);
    }

    // callback to announce destruction to resource if it was ok
    if (code == 0x00) {
        pthread_mutex_lock(&private->setcallback_lock);
        en50221_sl_session_callback cb = private->session;
        void *cb_arg = private->session_arg;
        pthread_mutex_unlock(&private->setcallback_lock);

        if (cb)
            cb(cb_arg, S_SCALLBACK_REASON_CLOSE, slot_id, session_number, resource_id);
    }
}

static void en50221_sl_handle_create_session_response(struct en50221_session_layer_private *private,
    uint8_t *data, uint32_t data_length, uint8_t slot_id, uint8_t connection_id)
{
    // check
    if (data_length < 8) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        return;
    }
    if (data[0] != 7) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        return;
    }

    // extract session number
    uint16_t session_number = (data[5] << 8) | data[6];

    // check session number is ok
    if (session_number >= private->max_sessions) {
        print(LOG_LEVEL, ERROR, 1, "Received bad session id %i\n", slot_id);
        return;
    }

    pthread_mutex_lock(&private->sessions[session_number].session_lock);
    if (slot_id != private->sessions[session_number].slot_id) {
        print(LOG_LEVEL, ERROR, 1, "Received unexpected session on invalid slot %i\n", slot_id);
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return;
    }
    if (connection_id != private->sessions[session_number].connection_id) {
        print(LOG_LEVEL, ERROR, 1, "Received unexpected session on invalid slot %i\n", slot_id);
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return;
    }
    if (private->sessions[session_number].state != S_STATE_IN_CREATION) {
        print(LOG_LEVEL, ERROR, 1, "Received unexpected session on invalid slot %i\n", slot_id);
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return;
    }

    // extract status
    if (data[1] != S_STATUS_OPEN) {
        print(LOG_LEVEL, ERROR, 1, "Session creation failed 0x%02x\n", data[1]);
        private->sessions[session_number].state = S_STATE_IDLE;
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);

        // inform upper layers
        pthread_mutex_lock(&private->setcallback_lock);
        en50221_sl_session_callback cb = private->session;
        void *cb_arg = private->session_arg;
        pthread_mutex_unlock(&private->setcallback_lock);
        if (cb)
            cb(cb_arg, S_SCALLBACK_REASON_CONNECTFAIL, slot_id, session_number,
               private->sessions[session_number].resource_id);
        return;
    }

    // set it active
    private->sessions[session_number].state = S_STATE_ACTIVE;
    pthread_mutex_unlock(&private->sessions[session_number].session_lock);

    // inform upper layers
    pthread_mutex_lock(&private->setcallback_lock);
    en50221_sl_session_callback cb = private->session;
    void *cb_arg = private->session_arg;
    pthread_mutex_unlock(&private->setcallback_lock);
    if (cb)
        cb(cb_arg, S_SCALLBACK_REASON_CONNECTED, slot_id, session_number,
           private->sessions[session_number].resource_id);
}

static void en50221_sl_handle_close_session_response(struct en50221_session_layer_private *private,
        uint8_t *data, uint32_t data_length, uint8_t slot_id, uint8_t connection_id)
{
    // check
    if (data_length < 5) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        return;
    }
    if (data[0] != 4) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        return;
    }

    // extract session number
    uint16_t session_number = (data[2] << 8) | data[3];

    // check session number is ok
    if (session_number >= private->max_sessions) {
        print(LOG_LEVEL, ERROR, 1, "Received bad session id %i\n", slot_id);
        return;
    }

    pthread_mutex_lock(&private->sessions[session_number].session_lock);
    if (slot_id != private->sessions[session_number].slot_id) {
        print(LOG_LEVEL, ERROR, 1, "Received unexpected session on invalid slot %i\n", slot_id);
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return;
    }
    if (connection_id != private->sessions[session_number].connection_id) {
        print(LOG_LEVEL, ERROR, 1, "Received unexpected session on invalid slot %i\n", slot_id);
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return;
    }
    if (private->sessions[session_number].state != S_STATE_IN_DELETION) {
        print(LOG_LEVEL, ERROR, 1, "Received unexpected session on invalid slot %i\n", slot_id);
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return;
    }

    // extract status
    if (data[1] != 0x00) {
        print(LOG_LEVEL, ERROR, 1, "Session close failed 0x%02x\n", data[1]);
        // just fallthrough anyway
    }

    // completed
    private->sessions[session_number].state = S_STATE_IDLE;
    pthread_mutex_unlock(&private->sessions[session_number].session_lock);
}

static void en50221_sl_handle_session_package(struct en50221_session_layer_private *private,
                                              uint8_t *data, uint32_t data_length,
                                              uint8_t slot_id, uint8_t connection_id)
{
    // check
    if (data_length < 3) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %i\n", slot_id);
        return;
    }
    if (data[0] != 2) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %i\n", slot_id);
        return;
    }

    // get session number
    uint16_t session_number = (data[1] << 8) | data[2];

    // check it
    if (session_number >= private->max_sessions) {
        print(LOG_LEVEL, ERROR, 1, "Received data with bad session_number from module on slot %i\n", slot_id);
        return;
    }

    pthread_mutex_lock(&private->sessions[session_number].session_lock);
    if (slot_id != private->sessions[session_number].slot_id) {
        print(LOG_LEVEL, ERROR, 1, "Received unexpected session on invalid slot %i\n", slot_id);
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return;
    }
    if (connection_id != private->sessions[session_number].connection_id) {
        print(LOG_LEVEL, ERROR, 1, "Received unexpected session on invalid slot %i\n", slot_id);
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return;
    }
    if (private->sessions[session_number].state != S_STATE_ACTIVE) {
        print(LOG_LEVEL, ERROR, 1, "Received data with bad session_number from module on slot %i\n", slot_id);
        pthread_mutex_unlock(&private->sessions[session_number].session_lock);
        return;
    }

    en50221_sl_resource_callback cb = private->sessions[session_number].callback;
    void *cb_arg = private->sessions[session_number].callback_arg;
    uint32_t resource_id = private->sessions[session_number].resource_id;
    pthread_mutex_unlock(&private->sessions[session_number].session_lock);

    // there can be > 1 APDU following the package - all for the same session/resource_id tho.
    data += 3;
    data_length -= 3;
    while(data_length) {
        // check length field
        if (data_length < 3) {
            print(LOG_LEVEL, ERROR, 1, "Received invalid sized session package from slot %i\n", slot_id);
            return;
        }

        // parse the APDU's length field
        int length_field_len;
        uint16_t asn_data_length;
        if ((length_field_len = asn_1_decode(&asn_data_length, data+3, data_length-3)) < 0) {
            print(LOG_LEVEL, ERROR, 1, "Received invalid sized session package from slot %i\n", slot_id);
            return;
        }
        uint32_t apdu_length = 3 + length_field_len + asn_data_length;

        // check there is enough data
        if (apdu_length > data_length) {
            print(LOG_LEVEL, ERROR, 1, "Received invalid sized session package from slot %i\n", slot_id);
            return;
        }

        // pass the APDU up to the higher layers
        if (cb)
            cb(cb_arg, slot_id, session_number, resource_id, data, apdu_length);

        // next!
        data += apdu_length;
        data_length -= apdu_length;
    }

}

static void en50221_sl_transport_callback(void *arg, int reason, uint8_t *data, uint32_t data_length,
                                          uint8_t slot_id, uint8_t connection_id)
{
    struct en50221_session_layer_private *private = (struct en50221_session_layer_private *) arg;
    uint32_t i;

    // deal with the reason for this callback
    switch(reason) {
    case T_CALLBACK_REASON_DATA:
        // fallthrough into rest of this function
        break;

    case T_CALLBACK_REASON_CONNECTIONOPEN:
    {
        pthread_mutex_lock(&private->setcallback_lock);
        en50221_sl_session_callback cb = private->session;
        void *cb_arg = private->session_arg;
        pthread_mutex_unlock(&private->setcallback_lock);

        if (cb)
            cb(cb_arg, S_SCALLBACK_REASON_TC_CONNECT, slot_id, connection_id, 0);
        return;
    }

    case T_CALLBACK_REASON_CAMCONNECTIONOPEN:
    {
        pthread_mutex_lock(&private->setcallback_lock);
        en50221_sl_session_callback cb = private->session;
        void *cb_arg = private->session_arg;
        pthread_mutex_unlock(&private->setcallback_lock);

        if (cb)
            cb(cb_arg, S_SCALLBACK_REASON_TC_CAMCONNECT, slot_id, connection_id, 0);
        return;
    }

    case T_CALLBACK_REASON_CONNECTIONCLOSE:
    {
        pthread_mutex_lock(&private->setcallback_lock);
        en50221_sl_session_callback cb = private->session;
        void *cb_arg = private->session_arg;
        pthread_mutex_unlock(&private->setcallback_lock);

        for(i=0; i< private->max_sessions; i++) {
            pthread_mutex_lock(&private->sessions[i].session_lock);

            if (private->sessions[i].state == S_STATE_IDLE) {
                pthread_mutex_unlock(&private->sessions[i].session_lock);
                continue;
            }
            if (private->sessions[i].connection_id != connection_id) {
                pthread_mutex_unlock(&private->sessions[i].session_lock);
                continue;
            }

            private->sessions[i].state = S_STATE_IDLE;

            uint8_t slot_id = private->sessions[i].slot_id;
            uint32_t resource_id = private->sessions[i].resource_id;
            pthread_mutex_unlock(&private->sessions[i].session_lock);

            if (cb)
                cb(cb_arg, S_SCALLBACK_REASON_CLOSE, slot_id, i, resource_id);
        }
        return;
    }

    case T_CALLBACK_REASON_SLOTCLOSE:
    {
        pthread_mutex_lock(&private->setcallback_lock);
        en50221_sl_session_callback cb = private->session;
        void *cb_arg = private->session_arg;
        pthread_mutex_unlock(&private->setcallback_lock);

        for(i=0; i< private->max_sessions; i++) {
            pthread_mutex_lock(&private->sessions[i].session_lock);

            if (private->sessions[i].state == S_STATE_IDLE) {
                pthread_mutex_unlock(&private->sessions[i].session_lock);
                continue;
            }
            if (private->sessions[i].slot_id != slot_id) {
                pthread_mutex_unlock(&private->sessions[i].session_lock);
                continue;
            }
            private->sessions[i].state = S_STATE_IDLE;

            uint32_t resource_id = private->sessions[i].resource_id;
            pthread_mutex_unlock(&private->sessions[i].session_lock);

            if (cb)
                cb(cb_arg, S_SCALLBACK_REASON_CLOSE, slot_id, i, resource_id);

        }
        return;
    }
    }

    // sanity check data length
    if (data_length < 1) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %i\n", slot_id);
        return;
    }

    // deal with the data
    uint8_t spdu_tag = data[0];
    switch(spdu_tag)
    {
        case ST_OPEN_SESSION_REQ:
            en50221_sl_handle_open_session_request(private, data+1, data_length-1, slot_id, connection_id);
            break;

        case ST_CLOSE_SESSION_REQ:
            en50221_sl_handle_close_session_request(private, data+1, data_length-1, slot_id, connection_id);
            break;

        case ST_SESSION_NUMBER:
            en50221_sl_handle_session_package(private, data+1, data_length-1, slot_id, connection_id);
            break;

        case ST_CREATE_SESSION_RES:
            en50221_sl_handle_create_session_response(private, data+1, data_length-1, slot_id, connection_id);
            break;

        case ST_CLOSE_SESSION_RES:
            en50221_sl_handle_close_session_response(private, data+1, data_length-1, slot_id, connection_id);
            break;

        default:
            print(LOG_LEVEL, ERROR, 1, "Received unknown session tag %02x from module on slot %i", spdu_tag, slot_id);
            break;
    }
}

static int en50221_sl_alloc_new_session(struct en50221_session_layer_private *private,
                                        uint32_t resource_id,
                                        uint8_t slot_id,
                                        uint8_t connection_id,
                                        en50221_sl_resource_callback callback, void* arg)
{
    int session_number = -1;
    uint32_t i;
    for(i = 1; i < private->max_sessions; i++) {
        if (private->sessions[i].state == S_STATE_IDLE) {
            session_number = i;
            break;
        }
    }
    if (session_number == -1) {
        private->error = EN50221ERR_OUTOFSESSIONS;
        return -1;
    }

    // setup the session
    private->sessions[session_number].state = S_STATE_IN_CREATION;
    private->sessions[session_number].resource_id = resource_id;
    private->sessions[session_number].slot_id = slot_id;
    private->sessions[session_number].connection_id = connection_id;
    private->sessions[session_number].callback = callback;
    private->sessions[session_number].callback_arg = arg;

    // ok
    return session_number;
}
