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
#define DVBCFG_MULTIPLEX_H

#include <dvbcfg_common.h>
#include <dvbcfg_source.h>
#include <stdint.h>
#include <linux/dvb/frontend.h>

/**
 * The dvbcfg_multiplex file consists of multiple sections, each describing a
 * particular multiplex or a service belonging to a multiplex. Its format is as
 * follows:
 * [multiplexes]
 * version=0.1
 * date=<unixdate>
 *
 * [m]
 * gmid = <GMID of this multiplex>
 * d = <delivery specific parameters>
 *
 * [s]
 * usid= <USID of this service>
 * name= <name of this service>
 * sname= <short name of this service>
 * pname= <provider name of this service>
 * flags= <service specific flags>
 * ca= <list of ca systems supported by this multiplex>
 * zap= <list of pids and their types for accelerated channel locking>
 * pmt= <list of pids and their types used as a supplement/replacement to the standard PMT SI tables>
 *
 * There may only be one [dvbmultiplex] section, and it must be the first section within the file.
 *
 * There can be multiple [m] and [s] sections in any one file. A [s] belongs
 * to the most recent preceding [m] section.
 *
 * All keys within the sections are mandatory, except for the following:
 * flags, sname, pname, ca, zap, pmt.
 *
 * GMID and USID are described in dvbcfg_common.h.
 *
 * The <delivery specific parameters> vary depending on the source_type of the multiplex. They are as follows:
 * DVBS: <frequency> <inversion> <polarization> <symbol_rate> <fec_inner>
 * DVBC: <frequency> <inversion> <symbol_rate> <fec_inner> <modulation>
 * DVBT: <frequency> <inversion> <bandwidth> <code_rate_HP> <code_rate_LP> <constellation> <tranmission_mode> <guard_interval> <hierarchy_information>
 * ATSC: <frequency> <inversion> <modulation>
 *
 * All numerical values are in the units used in the "struct dvb_frontend_parameters".
 * For other parameters, the numerical value as defined in the enumerations in frontend.h is used.
 *
 * Currently only the flag "nopmt" is defined for the <service specific flags>. If this is present, the
 * PMT should be ignored completely, and the pmt entries used instead.
 *
 * The <list of ca systems supported by this multiplex> is only used for encrypted services to identify the
 * type of CAM/subscription card the user must have in order to successfully decrypt a service. This is just
 * a list of numbers seperated by whitespace.
 *
 * Both zap and pmt have the same format: <pid>:<type>. <type> may either be one of the standard
 * PMT stream types as defined in ISO13818-1, table 2-29, or it may be one of the special values "_ac3", "_dts",
 * "_tt", or "_pcr".
 *
 * zap is to provide accelerated channel zapping. It gives a pre-extracted list of PIDs
 * (usually just audio, video, and pcr) so that a channel can be zapped to without having to wait for the
 * PAT or PMT tables. They should be verified against the PAT/PMT when it is received, in case they
 * have changed in the meantime. Use of these is optional.
 *
 * pmt gives a list of extra PID/type pairs for PIDS that are for some reason missing from the PMT
 * table for a channel. If the "nopmt" flag is also set, these entries will be used instead of a PMT.
 * If not, they will be used in addition to the PMT (however the pmt entries should override any clashing
 * entries in the PMT).
 *
 * This file format can also be used for "seed" transponders - if this is the case, the file should consist
 * only of [m] sections.
 *
 * Comments begin with '#' - any characters after this will be ignored
 * to the end of the line.
 *
 * Example:
 * [dvbmultiplexes]
 * version=0.1
 * date=5798475834
 *
 * [m]
 * gmid=S5E:0x0001:0x002:0x000
 * d= 12345 0 H 27500000 9
 *
 * [s]
 * usid=0x0001:0x000
 * name=service 1
 * zap=0x55:_ac3 0x56:0x78 0x57:_pcr 0x59:0x01
 * pmt=0x100:_dts 0x101:_tt 0x102:0x76
 *
 * [s]
 * usid=0x0002:0x000
 * name=service 2
 * zap=0x55:_ac3 0x56:0x78 0x57:_pcr 0x59:0x01
 * pmt=0x100:_dts 0x101:_tt 0x102:0x76
 *
 * [m]
 * gmid=Cde-de-Berlin:0x0001:0x002:0x000
 * d = 12345 0 27500000 9 4
 */




#define DVBCFG_PIDTYPE_AC3 0x100
#define DVBCFG_PIDTYPE_DTS 0x200
#define DVBCFG_PIDTYPE_TT  0x300
#define DVBCFG_PIDTYPE_PCR 0x400

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
        uint32_t service_flags;

        uint16_t* ca_systems;
        int ca_systems_count;

        struct dvbcfg_pid* zap_pids;
        int zap_pids_count;

        struct dvbcfg_pid* pmt_extra;
        int pmt_extra_count;

        struct dvbcfg_service *next;     /* NULL=> this is the last entry */
};


/**
 * In-memory representation of a single multiplex.
 */
struct dvbcfg_multiplex {
        struct dvbcfg_source* source;

        struct dvbcfg_umid umid;

        union {
                struct {
                        struct dvb_frontend_parameters fe_params;
                        uint8_t polarization:2;            /* DVBS only */
                } dvb;
        } delivery;

        struct dvbcfg_service* services;

        struct dvbcfg_multiplex *next;     /* NULL=> this is the last entry */
};


/**
 * Load multiplexes from a config file.
 *
 * @param config_file Config filename to load.
 * @param sources List of known dvbcfg_source structures.
 * @param multiplexes Where to put the pointer to the start of the loaded
 * multiplexes. If NULL, a new list will be created, if it points to an already initialised list,
 * the loaded multiplexes will be appended to it.
 * @param create_sources If 1, and a multiplex refers to an unknown source, the unknown source will be created. If 0, the
 * multiplex will be ignored.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_multiplex_load(const char *config_file,
                                 struct dvbcfg_source** sources,
                                 struct dvbcfg_multiplex** multiplexes,
                                 int create_sources);

/**
 * Save multiplexes to a config file.
 *
 * @param config_file Config filename to save.
 * @param multiplexes Pointer to the list of multiplexes to save.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_multiplex_save(const char *config_file,
                                 struct dvbcfg_multiplex* multiplexes);

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
 * @param usid USID of the new service.
 * @param service_flags DVBCFG_SERVICE_FLAG_* values ORed together.
 * @return The new struct dvbcfg_service structure, or NULL on error.
 */
extern struct dvbcfg_service* dvbcfg_multiplex_add_service(struct dvbcfg_multiplex* multiplex,
                                                           char* name,
                                                           char* usid,
                                                           uint32_t service_flags);

/**
 * Add a service to a multiplex using a dvbcfg_usid.
 *
 * @param multiplex Multiplex to add to.
 * @param name Name of the service.
 * @param usid USID of the new service.
 * @param service_flags DVBCFG_SERVICE_FLAG_* values ORed together.
 * @return The new struct dvbcfg_service structure, or NULL on error.
 */
extern struct dvbcfg_service* dvbcfg_multiplex_add_service2(struct dvbcfg_multiplex* multiplex,
                                                            char* name,
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

#endif                          // DVBCFG_MULTIPLEX_H
