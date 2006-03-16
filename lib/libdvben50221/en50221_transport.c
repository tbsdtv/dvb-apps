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
#include <linux/delay.h>
#include <sys/poll.h>
#include <time.h>
#include <dvbmisc.h>
#include <dvbapi/dvbca.h>
#include "en50221_errno.h"
#include "en50221_transport.h"
#include "asn_1.h"

// these are the Transport Tags, like
// described in EN50221, Annex A.4.1.13 (pg70)
#define T_SB                0x80 // sb                           primitive   h<--m
#define T_RCV               0x81 // receive                      primitive   h-->m
#define T_CREATE_T_C        0x82 // create transport connection  primitive   h-->m
#define T_C_T_C_REPLY       0x83 // ctc reply                    primitive   h<--m
#define T_DELETE_T_C        0x84 // delete tc                    primitive   h<->m
#define T_D_T_C_REPLY       0x85 // dtc reply                    primitive   h<->m
#define T_REQUEST_T_C       0x86 // request transport connection primitive   h<--m
#define T_NEW_T_C           0x87 // new tc / reply to t_request  primitive   h-->m
#define T_T_C_ERROR         0x77 // error creating tc            primitive   h-->m
#define T_DATA_LAST         0xA0 // convey data from higher      constructed h<->m
                                 // layers
#define T_DATA_MORE         0xA1 // convey data from higher      constructed h<->m
                                 // layers

// these are the states a TC can be in
// the values are not specified in the EN50221
// so we use similiar values like descibed for
// the Session Layer states
#define T_STATE_IDLE            0x00    // this transport connection is not in use
#define T_STATE_ACTIVE          0xF0    // this transport connection is in use
#define T_STATE_IN_CREATION     0xF1    // this transport waits for a T_C_T_C_REPLY to become active
#define T_STATE_IN_DELETION     0xF2    // this transport waits for T_D_T_C_REPLY to become idle again

struct en50221_connection {
    uint32_t state;                  // the current state: idle/in_delete/in_create/active
    uint32_t tx_time;                // time last request was sent from host->module, or 0 if ok
    uint32_t last_poll_time;         // time of last poll transmission
    uint8_t *chain_buffer;           // used to save parts of chained packets
    uint32_t buffer_length;
};

struct en50221_slot {
    int ca_hndl;
    struct pollfd ca_poll;
    struct en50221_connection *connections;

    uint32_t response_timeout;
    uint32_t poll_delay;
};

struct en50221_transport_layer_private
{
    uint8_t max_slots;
    uint8_t max_connections_per_slot;
    struct en50221_slot *slots;

    int error;
    int error_slot;

    en50221_session_callback callback;
    void *callback_private;
};

struct en50221_tpdu {
    uint8_t tpdu_tag;
    uint8_t length_field[3]; // use asn_1_encode/decode to process this field
    int length_field_len; // this stores the length of the length field
    uint8_t connection_id;
    uint8_t *data;
    uint16_t data_length; // *EXCLUDING* the connection_id field!
};

static int en50221_tl_write_tpdu(struct en50221_transport_layer_private *private, uint8_t slot_id,
                                 uint8_t connection_id, struct en50221_tpdu *data);
static int en50221_tl_proc_data_tc(struct en50221_transport_layer_private *tl, uint8_t slot_id,
                                   uint8_t *data, uint32_t data_length);
static int en50221_tl_poll_tc(struct en50221_transport_layer_private *tl, uint8_t slot_id, uint8_t connection_id);
static int en50221_tl_alloc_new_tc(struct en50221_transport_layer_private *tl, uint8_t slot_id);



en50221_transport_layer en50221_tl_create(uint8_t max_slots, uint8_t max_connections_per_slot)
{
    struct en50221_transport_layer_private *private = NULL;
    int i;
    int j;

    // setup structure
    private = (struct en50221_transport_layer_private*) malloc(sizeof(struct en50221_transport_layer_private));
    if (private == NULL)
        goto error_exit;
    private->max_slots = max_slots;
    private->max_connections_per_slot = max_connections_per_slot;
    private->slots = NULL;
    private->callback = NULL;
    private->callback_private = NULL;
    private->error_slot = 0;
    private->error = 0;

    // create the slots
    private->slots = malloc(sizeof(struct en50221_slot) * max_slots);
    if (private->slots == NULL)
        goto error_exit;

    // set them up
    for(i=0; i< max_slots; i++) {
        private->slots[i].ca_hndl = -1;

        // create the connections for this slot
        private->slots[i].connections = malloc(sizeof(struct en50221_connection) * max_connections_per_slot);
        if (private->slots[i].connections == NULL)
            goto error_exit;

        // set them up
        for(j = 0; j < max_connections_per_slot; j++) {
            private->slots[i].connections[j].state = T_STATE_IDLE;
            private->slots[i].connections[j].tx_time = 0;
            private->slots[i].connections[j].last_poll_time = 0;
            private->slots[i].connections[j].chain_buffer = NULL;
            private->slots[i].connections[j].buffer_length = 0;
        }
    }
    return private;

error_exit:
    en50221_tl_destroy(private);
    return NULL;
}

// Destroy an instance of the transport layer
void en50221_tl_destroy(en50221_transport_layer tl)
{
    struct en50221_transport_layer_private *private = (struct en50221_transport_layer_private *) tl;
    int i, j;

    if (private) {
        if (private->slots) {
            for(i=0; i< private->max_slots; i++) {
                if (private->slots[i].connections) {
                    for(j=0; j<private->max_connections_per_slot; j++) {
                        if (private->slots[i].connections[j].chain_buffer) {
                            free(private->slots[i].connections[j].chain_buffer);
                        }
                    }
                    free(private->slots[i].connections);
                }
            }
            free(private->slots);
        }
        free(private);
    }
}

// this can be called from the user-space app to
// register new slots that we should work with
int en50221_tl_register_slot(en50221_transport_layer tl, int ca_hndl,
                             uint32_t response_timeout, uint32_t poll_delay)
{
    struct en50221_transport_layer_private *private = (struct en50221_transport_layer_private *) tl;

    // we browse through the array of slots
    // to look for the first unused one
    int i;
    int16_t slot_id = -1;
    for(i=0; i < private->max_slots; i++) {
        if (private->slots[i].ca_hndl == -1) {
            slot_id = i;
            break;
        }
    }
    if (slot_id == -1) {
        private->error = EN50221ERR_OUTOFSLOTS;
        return -1;
    }

    // set up the slot struct
    private->slots[slot_id].ca_hndl = ca_hndl;
    private->slots[slot_id].response_timeout = response_timeout;
    private->slots[slot_id].poll_delay = poll_delay;
    private->slots[slot_id].ca_poll.fd = ca_hndl;
    private->slots[slot_id].ca_poll.events=POLLIN|POLLPRI|POLLERR;

    return slot_id;
}

void en50221_tl_destroy_slot(en50221_transport_layer tl, uint8_t slot_id)
{
    struct en50221_transport_layer_private *private = (struct en50221_transport_layer_private *) tl;
    int i;

    if (slot_id >= private->max_slots)
        return;

    private->slots[slot_id].ca_hndl = -1;
    for(i=0; i<private->max_connections_per_slot; i++) {
        private->slots[slot_id].connections[i].state = T_STATE_IDLE;
        private->slots[slot_id].connections[i].tx_time = 0;
        private->slots[slot_id].connections[i].last_poll_time = 0;
        if (private->slots[slot_id].connections[i].chain_buffer) {
            free(private->slots[slot_id].connections[i].chain_buffer);
        }
        private->slots[slot_id].connections[i].chain_buffer = NULL;
        private->slots[slot_id].connections[i].buffer_length = 0;
    }
}

int en50221_tl_poll(en50221_transport_layer tl)
{
    struct en50221_transport_layer_private *private = (struct en50221_transport_layer_private *) tl;
    uint8_t data[4096];
    int slot_id;

    for(slot_id = 0; slot_id < private->max_slots; slot_id++)
    {
        // check if this slot is used
        if (private->slots[slot_id].ca_hndl == -1)
            continue;

        int ca_hndl = private->slots[slot_id].ca_hndl;
        if (poll(&private->slots[slot_id].ca_poll, 1, 10))
        {
            if (private->slots[slot_id].ca_poll.revents & (POLLPRI | POLLIN)) {
                uint8_t connection_id;

                int readcnt = dvbca_link_read(ca_hndl, &connection_id, data, sizeof(data));
                if (readcnt < 0) {
                    private->error_slot = slot_id;
                    private->error = EN50221ERR_CAREAD;
                    return -1;
                }

                if (readcnt > 0) {
                    if (en50221_tl_proc_data_tc(private, slot_id, data, readcnt)) {
                        return -1;
                   }
                }
            } else { /* POLLERR */
                private->error_slot = slot_id;
                private->error = EN50221ERR_CAREAD;
                return -1;
            }
        }
        else
        {
            int j;
            for(j=1; j < private->max_connections_per_slot; j++) {
                if (private->slots[slot_id].connections[j].state != T_STATE_ACTIVE)
                    continue;

                if ((private->slots[slot_id].connections[j].tx_time == 0) &&
                     ((private->slots[slot_id].connections[j].last_poll_time + private->slots[slot_id].poll_delay) > time_ms())) {
                    if (en50221_tl_poll_tc(private, slot_id, j)) {
                        return -1;
                    }
                    private->slots[slot_id].connections[j].last_poll_time = time_ms();
                }
            }
        }

        // check for timeouts in connections
        int j;
        for(j=1; j < private->max_connections_per_slot; j++) {
            if (private->slots[slot_id].connections[j].tx_time > (time_ms() + private->slots[slot_id].response_timeout)) {
                private->error_slot = slot_id;
                private->error = EN50221ERR_TIMEOUT;
                return -1;
            }
        }
    }

    return 0;
}

void en50221_tl_register_session_callback(en50221_transport_layer tl, en50221_session_callback callback, void *private_data)
{
    struct en50221_transport_layer_private *private = (struct en50221_transport_layer_private *) tl;

    private->callback = callback;
    private->callback_private = private_data;
}

int en50221_tl_get_error_slot(en50221_transport_layer tl)
{
    struct en50221_transport_layer_private *private = (struct en50221_transport_layer_private *) tl;
    return private->error_slot;
}

int en50221_tl_get_error(en50221_transport_layer tl)
{
    struct en50221_transport_layer_private *private = (struct en50221_transport_layer_private *) tl;
    return private->error;
}






int en50221_tl_send_data(en50221_transport_layer tl, uint8_t slot_id, uint8_t connection_id, uint8_t *data,
                         uint16_t data_length)
{
    struct en50221_transport_layer_private *private = (struct en50221_transport_layer_private *) tl;

    if (slot_id >= private->max_slots) {
        private->error = EN50221ERR_BADSLOTID;
        return -1;
    }
    if (connection_id >= private->max_connections_per_slot) {
        private->error_slot = slot_id;
        private->error = EN50221ERR_BADCONNECTIONID;
        return -1;
    }

    struct en50221_tpdu command;
    command.tpdu_tag = T_DATA_LAST;
    if ((command.length_field_len = asn_1_encode(data_length + 1, command.length_field, 3)) < 0) {
        private->error_slot = slot_id;
        private->error = EN50221ERR_ASNENCODE;
        return -1;
    }
    command.connection_id = connection_id;
    command.data = data;
    command.data_length = data_length;

    // sendit
    int res = en50221_tl_write_tpdu(private, slot_id, connection_id, &command);
    if (!res)
        private->slots[slot_id].connections[connection_id].tx_time = time_ms();
    return res;
}

// create new transport connection
int en50221_tl_new_tc(en50221_transport_layer tl, uint8_t slot_id, uint8_t connection_id)
{
    struct en50221_transport_layer_private *private = (struct en50221_transport_layer_private *) tl;

    if (slot_id >= private->max_slots) {
        private->error = EN50221ERR_BADSLOTID;
        return -1;
    }
    if (connection_id > private->max_connections_per_slot) {
        private->error_slot = slot_id;
        private->error = EN50221ERR_BADCONNECTIONID;
        return -1;
    }

    // allocate a new connection if possible
    int conid = en50221_tl_alloc_new_tc(private, slot_id);
    if (conid == -1) {
        private->error_slot = slot_id;
        private->error = EN50221ERR_OUTOFCONNECTIONS;
        return -1;
    }

    // create the TPDU with CREATE_T_C and send it
    struct en50221_tpdu command;
    command.tpdu_tag = T_CREATE_T_C;
    if ((command.length_field_len = asn_1_encode(1, command.length_field, 3)) < 0) {
        private->error_slot = slot_id;
        private->error = EN50221ERR_ASNENCODE;
        return -1;
    }
    command.connection_id = conid;
    command.data = NULL;
    command.data_length = 0;
    if (en50221_tl_write_tpdu(private, slot_id, connection_id, &command) < 0) {
        private->slots[slot_id].connections[conid].state = T_STATE_IDLE;
        return -1;
    }
    private->slots[slot_id].connections[conid].tx_time = time_ms();

    return conid;
}

// handle delete request
int en50221_tl_del_tc(en50221_transport_layer tl, uint8_t slot_id, uint8_t connection_id)
{
    struct en50221_transport_layer_private *private = (struct en50221_transport_layer_private *) tl;

    if (slot_id >= private->max_slots) {
        private->error = EN50221ERR_BADSLOTID;
        return -1;
    }
    if ((connection_id == 0) || (connection_id >= private->max_connections_per_slot)) {
        private->error_slot = slot_id;
        private->error = EN50221ERR_BADCONNECTIONID;
        return -1;
    }
    if (private->slots[slot_id].connections[connection_id].state != T_STATE_ACTIVE) {
        private->error_slot = slot_id;
        private->error = EN50221ERR_BADSTATE;
        return -1;
    }

    // create the TPDU with DELETE_T_C and send it
    struct en50221_tpdu command;
    command.tpdu_tag = T_DELETE_T_C;
    if ((command.length_field_len = asn_1_encode(1, command.length_field, 3)) < 0) {
        private->error_slot = slot_id;
        private->error = EN50221ERR_ASNENCODE;
        return -1;
    }
    command.connection_id = connection_id;
    command.data = NULL;
    command.data_length = 0;

    private->slots[slot_id].connections[connection_id].state = T_STATE_IN_DELETION;
    if (private->slots[slot_id].connections[connection_id].chain_buffer) {
        free(private->slots[slot_id].connections[connection_id].chain_buffer);
        private->slots[slot_id].connections[connection_id].chain_buffer = NULL;
    }

    int res = en50221_tl_write_tpdu(private, slot_id, connection_id, &command);
    if (!res)
        private->slots[slot_id].connections[connection_id].tx_time = time_ms();
    return res;
}



// ask the module for new data
static int en50221_tl_poll_tc(struct en50221_transport_layer_private *private, uint8_t slot_id, uint8_t connection_id)
{
    // create the TPDU with T_DATA_LAST(0x00) and send it
    struct en50221_tpdu command;
    command.tpdu_tag = T_DATA_LAST;
    if ((command.length_field_len = asn_1_encode(1, command.length_field, 3)) < 0) {
        private->error_slot = slot_id;
        private->error = EN50221ERR_ASNENCODE;
        return -1;
    }
    command.connection_id = connection_id;
    command.data = NULL;
    command.data_length = 0;

    private->slots[slot_id].connections[connection_id].tx_time = time_ms();

    int res = en50221_tl_write_tpdu(private, slot_id, connection_id, &command);
    if (!res)
        private->slots[slot_id].connections[connection_id].tx_time = time_ms();
    return res;
}

// handle incoming data
static int en50221_tl_proc_data_tc(struct en50221_transport_layer_private *private, uint8_t slot_id, uint8_t *data, uint32_t data_length)
{
    struct en50221_tpdu units[2];
    int num_units = 1;

    // first step is parsing the data into a tpdu block
    // the data is not copied, because it's only a
    // 'frontend' to the data-block and won't be needed
    // any longer then the data-block exists
    if (data_length < 1) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        private->error_slot = slot_id;
        private->error = EN50221ERR_BADCAMDATA;
        return -1;
    }
    units[0].tpdu_tag = data[0];
    if ((units[0].length_field_len = asn_1_decode(&units[0].data_length, data + 1, data_length - 1)) < 0) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        private->error_slot = slot_id;
        private->error = EN50221ERR_BADCAMDATA;
        return -1;
    }
    if ((units[0].data_length < 1) || ((1UL + units[0].length_field_len + units[0].data_length) > data_length)) {
        print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
        private->error_slot = slot_id;
        private->error = EN50221ERR_BADCAMDATA;
        return -1;
    }
    units[0].data_length--;
    units[0].connection_id = data[1 + units[0].length_field_len];
    units[0].data = data + 1 + units[0].length_field_len + 1;

    // all R_TPDUs, except from T_SB have a
    // T_SB attached as second unit, parse that, too
    if (units[0].tpdu_tag != T_SB)
    {
        uint8_t *data2 = data + 1 + units[0].length_field_len + 1 + units[0].data_length;
        data_length -= (data2 - data);

        if (data_length < 1) {
            print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
            private->error_slot = slot_id;
            private->error = EN50221ERR_BADCAMDATA;
            return -1;
        }
        units[1].tpdu_tag = data2[0];
        if ((units[1].length_field_len = asn_1_decode(&units[1].data_length, data2 + 1, data_length - 1)) < 0) {
            print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
            private->error_slot = slot_id;
            private->error = EN50221ERR_BADCAMDATA;
            return -1;
        }
        if ((units[1].data_length < 1) || ((1UL + units[1].length_field_len + units[1].data_length) > data_length)) {
            print(LOG_LEVEL, ERROR, 1, "Received data with invalid length from module on slot %02x\n", slot_id);
            private->error_slot = slot_id;
            private->error = EN50221ERR_BADCAMDATA;
            return -1;
        }
        units[1].data_length--;
        units[1].connection_id = data2[1 + units[1].length_field_len];
        units[1].data = data2 + 1 + units[1].length_field_len + 1;
        num_units = 2;
    }

    int8_t new_id;
    struct en50221_tpdu command;
    int i;
    int conid;
    for(i = 0; i < num_units; i++)
    {
        struct en50221_tpdu unit = units[i];
        if (unit.connection_id >= private->max_connections_per_slot) {
            print(LOG_LEVEL, ERROR, 1, "Received bad connection id from module on slot %02x\n", slot_id);
            private->error_slot = slot_id;
            private->error = EN50221ERR_BADCONNECTIONID;
            return -1;
        }

        switch(unit.tpdu_tag)
        {
            case T_C_T_C_REPLY:
                // set this connection to state active
                if (private->slots[slot_id].connections[unit.connection_id].state == T_STATE_IN_CREATION) {
                    private->slots[slot_id].connections[unit.connection_id].state = T_STATE_ACTIVE;
                    private->slots[slot_id].connections[unit.connection_id].tx_time = 0;
                } else {
                    print(LOG_LEVEL, ERROR, 1, "Received T_C_T_C_REPLY for connection not in "
                            "T_STATE_IN_CREATION from module on slot %02x\n", slot_id);
                    private->error_slot = slot_id;
                    private->error = EN50221ERR_BADCAMDATA;
                    return -1;
                }
                break;
            case T_DELETE_T_C:
                // immediately delete this connection and send D_T_C_REPLY
                if (private->slots[slot_id].connections[unit.connection_id].state == T_STATE_ACTIVE ||
                    private->slots[slot_id].connections[unit.connection_id].state == T_STATE_IN_DELETION)
                {
                    private->slots[slot_id].connections[unit.connection_id].state = T_STATE_IDLE;
                    private->slots[slot_id].connections[unit.connection_id].tx_time = 0;
                    if (private->slots[slot_id].connections[unit.connection_id].chain_buffer) {
                        free(private->slots[slot_id].connections[unit.connection_id].chain_buffer);
                        private->slots[slot_id].connections[unit.connection_id].chain_buffer = NULL;
                    }

                    // create the TPDU with D_T_C_REPLY and send it
                    command.tpdu_tag = T_D_T_C_REPLY;
                    if ((command.length_field_len = asn_1_encode(1, command.length_field, 3)) < 0) {
                        private->error_slot = slot_id;
                        private->error = EN50221ERR_ASNENCODE;
                        return -1;
                    }
                    command.connection_id = unit.connection_id;
                    command.data = NULL;
                    command.data_length = 0;
                    if (en50221_tl_write_tpdu(private, slot_id, unit.connection_id, &command) < 0) {
                        return -1;
                    }
                }
                else {
                    print(LOG_LEVEL, ERROR, 1, "Received T_DELETE_T_C for inactive connection from module on slot %02x\n",
                          slot_id);
                    private->error_slot = slot_id;
                    private->error = EN50221ERR_BADCAMDATA;
                    return -1;
                }
                break;
            case T_D_T_C_REPLY:
                // delete this connection, should be in T_STATE_IN_DELETION already
                if (private->slots[slot_id].connections[unit.connection_id].state == T_STATE_IN_DELETION) {
                    private->slots[slot_id].connections[unit.connection_id].state = T_STATE_IDLE;
                    private->slots[slot_id].connections[unit.connection_id].tx_time = 0;
                } else {
                    print(LOG_LEVEL, ERROR, 1, "Received T_D_T_C_REPLY received for connection not in "
                            "T_STATE_IN_DELETION from module on slot %02x\n", slot_id);
                    private->error_slot = slot_id;
                    private->error = EN50221ERR_BADCAMDATA;
                    return -1;
                }
                break;
            case T_REQUEST_T_C:
                // allocate a new connection if possible
                conid = en50221_tl_alloc_new_tc(private, slot_id);
                if (conid == -1) {
                    print(LOG_LEVEL, ERROR, 1, "Too many connections requested by module on slot %02x\n", slot_id);

                    // create the TPDU with T_T_C_ERROR and send it
                    uint8_t buf[1];
                    struct en50221_tpdu command;
                    command.tpdu_tag = T_T_C_ERROR;
                    if ((command.length_field_len = asn_1_encode(2, command.length_field, 3)) < 0) {
                        private->error_slot = slot_id;
                        private->error = EN50221ERR_ASNENCODE;
                        return -1;
                    }
                    command.connection_id = unit.connection_id;
                    command.data = buf;
                    command.data_length = 1;
                    buf[0] = 1;
                    if (en50221_tl_write_tpdu(private, slot_id, unit.connection_id, &command))
                        return -1;
                } else {
                    // send new_t_c back
                    char buf[1];
                    command.tpdu_tag = T_NEW_T_C;
                    if ((command.length_field_len = asn_1_encode(2, command.length_field, 3)) < 0) {
                        private->error_slot = slot_id;
                        private->error = EN50221ERR_ASNENCODE;
                        return -1;
                    }
                    command.connection_id = unit.connection_id;
                    command.data = buf;
                    command.data_length = 1;
                    buf[0] = new_id;
                    if (en50221_tl_write_tpdu(private, slot_id, unit.connection_id, &command) < 0) {
                        return -1;
                    }

                    // this is a reply - we don't expect a reply to the reply.
                    private->slots[slot_id].connections[unit.connection_id].tx_time = 0;
                }
                break;
            case T_DATA_MORE:
                // connection in correct state?
                if (private->slots[slot_id].connections[unit.connection_id].state != T_STATE_ACTIVE) {
                    print(LOG_LEVEL, ERROR, 1, "Received T_DATA_MORE for connection not in "
                            "T_STATE_ACTIVE from module on slot %02x\n", slot_id);
                    private->error_slot = slot_id;
                    private->error = EN50221ERR_BADCAMDATA;
                    return -1;
                }

                // a chained data packet is coming in, save
                // it to the buffer and wait for more
                private->slots[slot_id].connections[unit.connection_id].tx_time = 0;
                if (private->slots[slot_id].connections[unit.connection_id].chain_buffer == NULL) {
                    // empty buffer allocate some memory and save
                    private->slots[slot_id].connections[unit.connection_id].chain_buffer = malloc(unit.data_length);
                    if (private->slots[slot_id].connections[unit.connection_id].chain_buffer == NULL) {
                        private->error_slot = slot_id;
                        private->error = EN50221ERR_OUTOFMEMORY;
                        return -1;
                    }

                    memcpy(private->slots[slot_id].connections[unit.connection_id].chain_buffer,
                           unit.data, unit.data_length);
                    private->slots[slot_id].connections[unit.connection_id].buffer_length = unit.data_length;
                } else {
                    int new_data_length =
                            private->slots[slot_id].connections[unit.connection_id].buffer_length + unit.data_length;
                    uint8_t *new_data_buffer =
                            realloc(private->slots[slot_id].connections[unit.connection_id].chain_buffer,
                                    new_data_length);
                    if (new_data_buffer == NULL) {
                        private->error_slot = slot_id;
                        private->error = EN50221ERR_OUTOFMEMORY;
                        return -1;
                    }

                    private->slots[slot_id].connections[unit.connection_id].chain_buffer = new_data_buffer;

                    memcpy(private->slots[slot_id].connections[unit.connection_id].chain_buffer +
                            private->slots[slot_id].connections[unit.connection_id].buffer_length,
                           unit.data, unit.data_length);
                    private->slots[slot_id].connections[unit.connection_id].buffer_length = new_data_length;
                }
                break;
            case T_DATA_LAST:
                // connection in correct state?
                if (private->slots[slot_id].connections[unit.connection_id].state != T_STATE_ACTIVE) {
                    print(LOG_LEVEL, ERROR, 1, "Received T_DATA_LAST received for connection not in "
                            "T_STATE_ACTIVE from module on slot %02x\n", slot_id);
                    private->error_slot = slot_id;
                    private->error = EN50221ERR_BADCAMDATA;
                    return -1;
                }

                // last package of a chain or single package comes in
                private->slots[slot_id].connections[unit.connection_id].tx_time = 0;
                if (private->slots[slot_id].connections[unit.connection_id].chain_buffer == NULL)
                {
                    // single package => dispatch immediately
                    if (private->callback)
                        private->callback(private->callback_private, unit.data, unit.data_length,
                                          slot_id, unit.connection_id);
                }
                else
                {
                    int new_data_length =
                            private->slots[slot_id].connections[unit.connection_id].buffer_length + unit.data_length;
                    uint8_t *new_data_buffer =
                            realloc(private->slots[slot_id].connections[unit.connection_id].chain_buffer,
                                    new_data_length);
                    if (new_data_buffer == NULL) {
                        private->error_slot = slot_id;
                        private->error = EN50221ERR_OUTOFMEMORY;
                        return -1;
                    }

                    memcpy(new_data_buffer + private->slots[slot_id].connections[unit.connection_id].buffer_length,
                           unit.data, unit.data_length);

                    // clean the buffer position
                    private->slots[slot_id].connections[unit.connection_id].chain_buffer = NULL;
                    private->slots[slot_id].connections[unit.connection_id].buffer_length = 0;

                    if (private->callback)
                        private->callback(private->callback_private, new_data_buffer, new_data_length,
                                          slot_id, unit.connection_id);

                    free(new_data_buffer);
                }
                break;
            case T_SB:
                // is the connection id ok?
                if (private->slots[slot_id].connections[unit.connection_id].state != T_STATE_ACTIVE) {
                    print(LOG_LEVEL, ERROR, 1, "Received T_SB for connection not in T_STATE_ACTIVE from module on slot %02x\n",
                          slot_id);
                    private->error_slot = slot_id;
                    private->error = EN50221ERR_BADCAMDATA;
                    return -1;
                }

                // did we get enough data in the T_SB?
                if (unit.data_length != 1) {
                    print(LOG_LEVEL, ERROR, 1, "Recieved T_SB with invalid length from module on slot %02x\n", slot_id);
                    private->error_slot = slot_id;
                    private->error = EN50221ERR_BADCAMDATA;
                    return -1;
                }
                print(LOG_LEVEL, ERROR, 1, "Recieved unexpected TPDU tag %02x from module on slot %02x\n",
                      unit.tpdu_tag, slot_id);

                // tell it to send the data if it says there is some
                if (unit.data[0] & 0x80) {
                    // the module has data to send, request it to send it now
                    command.tpdu_tag = T_RCV;
                    if ((command.length_field_len = asn_1_encode(1, command.length_field, 3)) < 0) {
                        private->error_slot = slot_id;
                        private->error = EN50221ERR_ASNENCODE;
                        return -1;
                    }
                    command.connection_id = unit.connection_id;
                    command.data = NULL;
                    command.data_length = 0;
                    if (en50221_tl_write_tpdu(private, slot_id, unit.connection_id, &command) < 0) {
                        return -1;
                    }
                    private->slots[slot_id].connections[unit.connection_id].tx_time = time_ms();
                }
                break;

            default:
                print(LOG_LEVEL, ERROR, 1, "Recieved unexpected TPDU tag %02x from module on slot %02x\n",
                      unit.tpdu_tag, slot_id);
                private->error_slot = slot_id;
                private->error = EN50221ERR_BADCAMDATA;
                return -1;
        }
    }

    return 0;
}

static int en50221_tl_write_tpdu(struct en50221_transport_layer_private *private, uint8_t slot_id,
                                 uint8_t connection_id, struct en50221_tpdu *data)
{
    uint8_t tpdu_hdr[10];
    struct iovec iov_out[2];
    int iov_count = 0;

    // setup the header
    memcpy(tpdu_hdr, &data->tpdu_tag, 1);
    memcpy(tpdu_hdr + 1, data->length_field, data->length_field_len);
    memcpy(tpdu_hdr + 1 + data->length_field_len, &data->connection_id, 1);
    iov_out[iov_count].iov_base = tpdu_hdr;
    iov_out[iov_count].iov_len = 1 + data->length_field_len + 1;
    iov_count++;

    // setup the data
    if (data->data && (data->data_length)) {
        iov_out[iov_count].iov_base = data->data;
        iov_out[iov_count].iov_len = data->data_length;
        iov_count++;
    }

    // send it!
    int res = dvbca_link_writev(private->slots[slot_id].ca_hndl, connection_id, iov_out, iov_count);
    if (res < 0) {
        private->error_slot = slot_id;
        private->error = EN50221ERR_CAWRITE;
    }
    return res;
}

static int en50221_tl_alloc_new_tc(struct en50221_transport_layer_private *private, uint8_t slot_id)
{
    // we browse through the array of connection
    // types, to look for the first unused one
    int i, conid = -1;
    for(i=0; i < private->max_connections_per_slot; i++) {
        if (private->slots[slot_id].connections[i].state == T_STATE_IDLE) {
            conid = i;
            break;
        }
    }
    if (conid == -1) {
        print(LOG_LEVEL, ERROR, 1, "CREATE_T_C failed: no more connections available\n");
        return -1;
    }

    // set up the connection struct
    private->slots[slot_id].connections[conid].state = T_STATE_IN_CREATION;
    private->slots[slot_id].connections[conid].chain_buffer = NULL;
    private->slots[slot_id].connections[conid].buffer_length = 0;

    return conid;
}
