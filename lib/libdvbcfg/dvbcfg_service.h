/**
 * dvbcfg_service configuration file support.
 *
 * Copyright (c) 2005 by Andrew de Quincey <adq_dvb@lidskialf.net>
 *
 * Credits go to Klaus Schmidinger's VDR for coming up with this really
 * great idea.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef DVBCFG_SERVICE_H
#define DVBCFG_SERVICE_H

#include <dvbcfg_common.h>
#include <dvbcfg_source.h>

/**
 * FIXME: describe
 */


#define DVBCFG_PIDTYPE_AC3 0x100
#define DVBCFG_PIDTYPE_DTS 0x101
#define DVBCFG_PIDTYPE_TELETEXT 0x102
#define DVBCFG_PIDTYPE_PCR 0x103

struct dvbcfg_pid {
        uint16_t pid;
        uint16_t type;
}

#define DVBCFG_MULTIPLEX_FLAG_IGNOREPMT 0x01

/**
 * In-memory representation of a single service.
 */
struct dvbcfg_service {
        struct dvbcfg_usid usid;
        char* name;
        uint32_t flags;
        uint16_t* ca_systems;
        int ca_systems_count;

        struct dvbcfg_pid* zap_pids;
        int zap_pids_count;

        struct dvbcfg_pid* pmt_pids;
        int pmt_pids_count;

        struct dvbcfg_service *next;     /* NULL=> this is the last entry */
};


/**
 * In-memory representation of a single multiplex.
 */
struct dvbcfg_multiplex {
        struct dvbcfg_source* source;

        struct dvbcfg_umid umid;
        struct dvb_frontend_parameters dvb_delivery;
        uint8_t polarization:2;            /* DVBS only */

        struct dvbcfg_service* services;

        struct dvbcfg_multiplex *next;     /* NULL=> this is the last entry */
};


/**
 * Load services from a config file.
 *
 * @param config_file Config filename to load.
 * @param services Where to put the pointer to the start of the loaded
 * services. If NULL, a new list will be created, if it points to an already initialised list,
 * the loaded services will be appended to it.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_service_load(char *config_file,
                               struct dvbcfg_source *sources);

/**
 * Save services to a config file.
 *
 * @param config_file Config filename to save.
 * @param services Pointer to the list of services to save.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_service_save(char *config_file,
                              struct dvbcfg_service *services);

extern struct dvbcfg_service *dvbcfg_service_find_multiplex(struct dvbcfg_service *services, struct dvbcfg_umid* gmid);

extern struct dvbcfg_service *dvbcfg_service_find_service(struct dvbcfg_service *services, struct dvbcfg_gsid* gsid);

/**
 * Unlink a single service from a list, and free its memory.
 *
 * @param services The list of services.
 * @param tofree The service to free.
 */
extern void dvbcfg_service_free(struct dvbcfg_service **services,
                                struct dvbcfg_service *tofree);

/**
 * Free memory for all services in a list.
 *
 * @param services Pointer to list of services to free.
 */
extern void dvbcfg_service_free_all(struct dvbcfg_service *services);

#endif                          // DVBCFG_service_H
