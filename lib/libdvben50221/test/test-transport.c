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

    // try and find a ca slot
    int cafd= -1;
    for(i=0; i<20; i++) {
        if ((cafd = dvbca_open(i, 0)) > 0) {
            break;
        }
    }
    if (cafd == -1) {
        fprintf(stderr, "Could not find CA device\n");
        exit(1);
    }
    dvbca_reset(cafd);
    printf("Found a CAM on adapter%i... waiting...\n", i);
    while(dvbca_get_cam_state(cafd) != DVBCA_CAMSTATE_READY) {
        usleep(1000);
    }

    // create transport layer
    en50221_transport_layer tl = en50221_tl_create(1, 32);
    if (tl == NULL) {
        fprintf(stderr, "Failed to create transport layer\n");
        exit(1);
    }

    // start another thread to running the stack
    pthread_create(&stackthread, NULL, stackthread_func, tl);

    // register callback
    en50221_tl_register_callback(tl, test_callback, tl);

    // register a slot with it
    int slot_id = 0;
    if ((slot_id = en50221_tl_register_slot(tl, cafd, 5000, 100)) < 0) {
        fprintf(stderr, "Slot registration failed\n");
        exit(1);
    }
    printf("slotid: %i\n", slot_id);

    // create a new connection
    int tc = en50221_tl_new_tc(tl, slot_id, 0);
    printf("tcid: %i\n", tc);

    // sleep a bit
    sleep(10);

    // destroy slot
    en50221_tl_destroy_slot(tl, slot_id);
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
    printf("CALLBACK %i %i %i\n", slot_id, connection_id, reason);

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
