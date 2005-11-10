/**
 * dvbcfg_multiplex configuration file support.
 *
 * Copyright (c) 2005 by Andrew de Quincey <adq_dvb@lidskialf.net>
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

#ifndef DVBCFG_MULTIPLEX_H
#define DVBCFG_MULTIPLEX_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <dvbcfg/dvbcfg_common.h>
#include <dvbcfg/dvbcfg_source.h>
#include <stdint.h>

/**
 * dvbcfg_multiplex represents the set of multiplexes and their associated
 * services.
 */

/**
 * Extra PID identifiers for streams stored in private streams.
 */
#define DVBCFG_PIDTYPE_AC3 0x100
#define DVBCFG_PIDTYPE_DTS 0x101
#define DVBCFG_PIDTYPE_TT  0x102
#define DVBCFG_PIDTYPE_PCR 0x103

/**
 * Structure describing a particular PID
 *
 * Normally the type of a PID is specified using the standard MPEG2
 * PMT stream types as defined in ISO13818-1 (table 2-28). However, sometimes
 * this is not enough - e.g. AC3,DTS,teletext streams all use the same PMT stream
 * type, yet the contents differ radically. In this case, the extended DVBCFG_PIDTYPE_* values
 * should be used in the type field.
 */
struct dvbcfg_pid {
        uint16_t pid;
        uint16_t type;
};


#define DVBCFG_SERVICE_FLAG_IGNOREPMT 0x01

/**
 * In-memory representation of a single service.
 */
struct dvbcfg_service {
        struct dvbcfg_usid usid;
        char* name;
        char* short_name;               /* can be NULL */
        char* provider_name;            /* can be NULL */
        uint32_t service_flags;

        uint16_t* ca_systems;
        int ca_systems_count;

        struct dvbcfg_pid* zap_pids;
        int zap_pids_count;

        struct dvbcfg_pid* pmt_extra;
        int pmt_extra_count;

        struct dvbcfg_service *next;    /* NULL=> this is the last entry */
};

/**
 * In-memory representation of a single multiplex.
 */
struct dvbcfg_multiplex {
        struct dvbcfg_source* source;

        struct dvbcfg_umid umid;

        struct dvbcfg_delivery delivery;

        struct dvbcfg_service* services;

        struct dvbcfg_multiplex *next;     /* NULL=> this is the last entry */
};

/**
 * Multiplex backend API.
 */
struct dvbcfg_multiplex_backend {
        /**
         * Loads a single multiplex from the backend and add to the supplied list.
         *
         * @param backend Pointer to the backend structure concerned.
         * @param multiplexes Pointer to the list of multiplexes.
         * @return 0 on success, <0 on error, or 1 on end of file, or 2
         * indicating the entry is actually a service.
         */
        int (*get_multiplex)(struct dvbcfg_multiplex_backend* backend,
                             struct dvbcfg_multiplex** multiplexes);

        /**
         * Loads a single service from the backend and add to the supplied list.
         *
         * @param backend Pointer to the backend structure concerned.
         * @param multiplex Pointer to the multiplex to add to.
         * @return 0 on success, <0 on error, or 1 on end of file, or 2
         * indicating the entry is actually a multiplex.
         */
        int (*get_service)(struct dvbcfg_multiplex_backend* backend,
                           struct dvbcfg_multiplex* multiplex);


        /**
         * Stores a single multiplex to the backend (does NOT store services).
         *
         * @param backend Pointer to the backend structure concerned.
         * @param multiplex Multiplex to store.
         * @return 0 on success, <0 on error.
         */
        int (*put_multiplex)(struct dvbcfg_multiplex_backend* backend,
                             struct dvbcfg_multiplex* multiplex);

        /**
         * Stores a single service to the backend.
         *
         * @param backend Pointer to the backend structure concerned.
         * @param service Service to store.
         * @return 0 on success, <0 on error.
         */
        int (*put_service)(struct dvbcfg_multiplex_backend* backend,
                           struct dvbcfg_service* service);
};

/**
 * Convenience method to load all multiplexes/services from a backend.
 *
 * @param backend Backend to use.
 * @param multiplexes Where to put the pointer to the start of the loaded
 * multiplexes. If NULL, a new list will be created, if it points to an
 * already initialised list, the loaded multiplexes will be appended to it.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_multiplex_load(struct dvbcfg_multiplex_backend *backend,
                                 struct dvbcfg_multiplex **multiplexes);

/**
 * Convenience method to store all multiplexes to a backend.
 *
 * @param backend Backend to use.
 * @param multiplexes Pointer to the list of multiplexes to save.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_multiplex_save(struct dvbcfg_multiplex_backend *backend,
                                 struct dvbcfg_multiplex *multiplexes);

/**
 * Create a new multiplex.
 *
 * @param multiplexes List of multiplexes to add to.
 * @param source dvbcfg_source of the new multiplex.
 * @param umid UMID of the new multiplex.
 * @return new struct dvbcfg_multiplex structure, or NULL on error.
 */
extern struct dvbcfg_multiplex* dvbcfg_multiplex_new(struct dvbcfg_multiplex** multiplexes,
                                                     struct dvbcfg_source* source,
                                                     char* umid);

/**
 * Create a new multiplex using a dvbcfg_umid.
 *
 * @param multiplexes List of multiplexes to add to.
 * @param source dvbcfg_source of the new multiplex.
 * @param umid UMID of the new multiplex.
 * @return new struct dvbcfg_multiplex structure, or NULL on error.
 */
extern struct dvbcfg_multiplex* dvbcfg_multiplex_new2(struct dvbcfg_multiplex** multiplexes,
                                                     struct dvbcfg_source* source,
                                                     struct dvbcfg_umid* umid);

/**
 * Add a service to a multiplex.
 *
 * @param multiplex Multiplex to add to.
 * @param name Name of the service.
 * @param short_name Short name of the service.
 * @param provider_name Name of the provider of the service.
 * @param usid USID of the new service.
 * @param service_flags DVBCFG_SERVICE_FLAG_* values ORed together.
 * @return The new struct dvbcfg_service structure, or NULL on error.
 */
extern struct dvbcfg_service* dvbcfg_multiplex_add_service(struct dvbcfg_multiplex* multiplex,
                                                           char* name,
                                                           char* short_name,
                                                           char* provider_name,
                                                           char* usid,
                                                           uint32_t service_flags);

/**
 * Add a service to a multiplex using a dvbcfg_usid.
 *
 * @param multiplex Multiplex to add to.
 * @param name Name of the service.
 * @param short_name Short name of the service.
 * @param provider_name Name of the provider of the service.
 * @param usid USID of the new service.
 * @param service_flags DVBCFG_SERVICE_FLAG_* values ORed together.
 * @return The new struct dvbcfg_service structure, or NULL on error.
 */
extern struct dvbcfg_service* dvbcfg_multiplex_add_service2(struct dvbcfg_multiplex* multiplex,
                                                            char* name,
                                                            char* short_name,
                                                            char* provider_name,
                                                            struct dvbcfg_usid* usid,
                                                            uint32_t service_flags);

/**
 * Remove a service from a multiplex.
 *
 * @param multiplex The multiplex to remove from.
 * @param service The service to remove.
 */
extern void dvbcfg_multiplex_remove_service(struct dvbcfg_multiplex* multiplex,
                                            struct dvbcfg_service* service);

/**
 * Add a CA system to a service.
 *
 * @param service Service to add to.
 * @param ca_system_id The ID to add.
 * @return 0 on success, nonzero on error.
 */
extern int dvbcfg_multiplex_add_ca_system(struct dvbcfg_service* service,
                                          uint16_t ca_system_id);

/**
 * Remove a CA system from a service.
 *
 * @param service Service to remove from.
 * @param ca_system_id The ID to remove.
 * @return 0 on success, nonzero on error.
 */
extern int dvbcfg_multiplex_remove_ca_system(struct dvbcfg_service* service,
                                             uint16_t ca_system_id);

/**
 * Add a zap_pid to a service.
 *
 * @param service Service to add to.
 * @param pid The PID to add.
 * @param type Type of the PID (ISO13818 or DVBCFG_PIDTYPE_*)
 * @return 0 on success, nonzero on error.
 */
extern int dvbcfg_multiplex_add_zap_pid(struct dvbcfg_service* service,
                                        int pid,
                                        int type);

/**
 * Remove a zap_pid from a service. You can set pid and/or type to -1 for a wildcard.
 *
 * @param service Service to remove from.
 * @param pid The PID to remove (or -1 for any PID)
 * @param type Type of the PID (ISO13818 or DVBCFG_PIDTYPE_*) (or -1 for any type)
 * @return 0 on success, nonzero on error.
 */
extern int dvbcfg_multiplex_remove_zap_pid(struct dvbcfg_service* service,
                                           int pid,
                                           int type);

/**
 * Add a pmt_extra to a service.
 *
 * @param service Service to add to.
 * @param pid The PID to add.
 * @param type Type of the PID (ISO13818 or DVBCFG_PIDTYPE_*)
 * @return 0 on success, nonzero on error.
 */
extern int dvbcfg_multiplex_add_pmt_extra(struct dvbcfg_service* service,
                                        int pid,
                                        int type);

/**
 * Remove a pmt_extra from a service. You can set pid and/or type to -1 for a wildcard.
 *
 * @param service Service to remove from.
 * @param pid The PID to remove (or -1 for any PID)
 * @param type Type of the PID (ISO13818 or DVBCFG_PIDTYPE_*) (or -1 for any type)
 * @return 0 on success, nonzero on error.
 */
extern int dvbcfg_multiplex_remove_pmt_extra(struct dvbcfg_service* service,
                                            int pid,
                                            int type);

/**
 * Find a multiplex.
 *
 * @param multiplexes List to search in.
 * @param gmid GMID of the multiplex to search for.
 * @return struct dvbcfg_multiplex pointer, or NULL if not found.
 */
extern struct dvbcfg_multiplex *dvbcfg_multiplex_find(struct dvbcfg_multiplex *multiplexes, char* gmid);

/**
 * Find a multiplex using a dvbcfg_gmid.
 *
 * @param multiplexes List to search in.
 * @param gmid GMID of the multiplex to search for.
 * @return struct dvbcfg_multiplex pointer, or NULL if not found.
 */
extern struct dvbcfg_multiplex *dvbcfg_multiplex_find2(struct dvbcfg_multiplex *multiplexes, struct dvbcfg_gmid* gmid);

/**
 * Find a service in a specific multiplex.
 *
 * @param multiplex Multiplex to search in.
 * @param usid USID of the service to find.
 * @return struct dvbcfg_service pointer, or NULL if not found.
 */
extern struct dvbcfg_service *dvbcfg_multiplex_find_service_in_multiplex(struct dvbcfg_multiplex *multiplex, char* usid);

/**
 * Find a service in a specific multiplex using a dvbcfg_usid
 *
 * @param multiplex Multiplex to search in.
 * @param usid USID of the service to find.
 * @return struct dvbcfg_service pointer, or NULL if not found.
 */
extern struct dvbcfg_service *dvbcfg_multiplex_find_service_in_multiplex2(struct dvbcfg_multiplex *multiplex,
                                                                          struct dvbcfg_usid* usid);

/**
 * Find a service in all multiplexes.
 *
 * @param multiplexes Multiplexes to search in.
 * @param gsid GSID of the service to find.
 * @return struct dvbcfg_service pointer, or NULL if not found.
 */
extern struct dvbcfg_service *dvbcfg_multiplex_find_service(struct dvbcfg_multiplex *multiplexes, char* gsid);

/**
 * Find a service in all multiplexes using a dvbcfg_gsid.
 *
 * @param multiplexes Multiplexes to search in.
 * @param gsid GSID of the service to find.
 * @return struct dvbcfg_service pointer, or NULL if not found.
 */
extern struct dvbcfg_service *dvbcfg_multiplex_find_service2(struct dvbcfg_multiplex *multiplexes, struct dvbcfg_gsid* gsid);

/**
 * Unlink a single multiplex from a list, and free its memory.
 *
 * @param multiplexes The list of multiplexes.
 * @param tofree The service to free.
 */
extern void dvbcfg_multiplex_free(struct dvbcfg_multiplex** multiplexes,
                                  struct dvbcfg_multiplex* tofree);

/**
 * Free memory for all multiplexes in a list.
 *
 * @param multiplexes Pointer to list of multiplexes to free.
 */
extern void dvbcfg_multiplex_free_all(struct dvbcfg_multiplex* multiplexes);

/**
 * Postproces a list of multiplexes, removing duplicates and assinging differentiators where necessary.
 *
 * @param multiplexes List of multiplexes to process.
 */
extern void dvbcfg_multiplex_postprocess(struct dvbcfg_multiplex** multiplexes);

/**
 * Calculate the differentiator value for a multiplex.
 *
 * @param multiplex The multiplex concerned.
 * @return The differentiator value.
 */
extern uint32_t dvbcfg_multiplex_calculate_differentiator(struct dvbcfg_multiplex* multiplex);

#ifdef __cplusplus
}
#endif

#endif
