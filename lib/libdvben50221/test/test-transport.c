#include <stdio.h>
#include <unistd.h>
#include <dvben50221/en50221_transport.h>
#include <dvbapi/dvbca.h>
#include <pthread.h>

void *stackthread_func(void* arg);
void test_callback(void *arg, int reason,
                   uint8_t *data, uint32_t data_length,
                   uint8_t slot_id, uint8_t connection_id);

int shutdown_stackthread = 0;

int main(int argc, char * argv[])
{
    int i;
    pthread_t stackthread;

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

    // start another thread to running the stack
    pthread_create(&stackthread, NULL, stackthread_func, tl);

    // register callback
    en50221_tl_register_callback(tl, test_callback, tl);

    // create a new connection
    for(i=0; i<slot_count; i++) {
        int tc = en50221_tl_new_tc(tl, i, 0);
        printf("tcid: %i\n", tc);
    }

    // sleep a bit
    sleep(10);

    // destroy slots
    for(i=0; i<slot_count; i++) {
        en50221_tl_destroy_slot(tl, i);
    }
    shutdown_stackthread = 1;
    pthread_join(stackthread, NULL);

    // destroy transport layer
    en50221_tl_destroy(tl);

    return 0;
}

void test_callback(void *arg, int reason,
                   uint8_t *data, uint32_t data_length,
                   uint8_t slot_id, uint8_t connection_id)
{
    printf("-----------------------------------\n");
    printf("CALLBACK SLOTID:%i %i %i\n", slot_id, connection_id, reason);

    int i;
    for(i=0; i< data_length; i++) {
        printf("%02x %02x\n", i, data[i]);
    }
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
