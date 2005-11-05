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

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include "dvbcfg_multiplex.h"
#include "dvbcfg_util.h"

static int add_pid(int* count, struct dvbcfg_pid** pids, int pid, int type);
static int remove_pid(int* count, struct dvbcfg_pid** pids, int pid, int type);

int dvbcfg_multiplex_load(struct dvbcfg_multiplex_backend *backend,
                          struct dvbcfg_multiplex **multiplexes)
{
        int status;
        int read_multiplex = 1;
        struct dvbcfg_multiplex *curmultiplex = NULL;

        while(1) {
                /* read in the element */
                if (read_multiplex) {
                        status = backend->get_multiplex(backend, multiplexes);
                } else {
                        status = backend->get_service(backend, curmultiplex);
                }

                /* deal with what happened */
                switch(status) {
                case 0: /* ok! */
                        if (read_multiplex) {
                                if (curmultiplex == NULL) {
                                        curmultiplex = *multiplexes;
                                } else {
                                        curmultiplex = curmultiplex->next;
                                }
                        }
                        if (curmultiplex == NULL)
                                return -EIO;
                        break;

                case 1: /* EOF */
                        return 0;

                case 2: /* switch reading mode */
                        read_multiplex = (read_multiplex + 1) & 1;
                        break;

                default:  /* error */
                        return status;
                }
        }

        return 0;
}

int dvbcfg_multiplex_save(struct dvbcfg_multiplex_backend *backend,
                          struct dvbcfg_multiplex *multiplexes)
{
        int status;
        struct dvbcfg_service *service;

        while(multiplexes) {
                if ((status = backend->put_multiplex(backend, multiplexes)) != 0)
                        return status;

                service = multiplexes->services;
                while(service) {
                        if ((status = backend->put_service(backend, service)) != 0)
                                return status;

                        service = service->next;
                }

                multiplexes = multiplexes->next;
        }

        return 0;
}


struct dvbcfg_multiplex* dvbcfg_multiplex_new(struct dvbcfg_multiplex** multiplexes,
                                              struct dvbcfg_source* source,
                                              char* umidstr)
{
        struct dvbcfg_umid umid;

        /* parse the umid */
        if (dvbcfg_umid_from_string(umidstr, &umid)) {
                return NULL;
        }

        return dvbcfg_multiplex_new2(multiplexes, source, &umid);
}

struct dvbcfg_multiplex* dvbcfg_multiplex_new2(struct dvbcfg_multiplex** multiplexes,
                                              struct dvbcfg_source* source,
                                              struct dvbcfg_umid* umid)
{
        struct dvbcfg_multiplex* newmultiplex;
        struct dvbcfg_multiplex* curmultiplex;

        /* create new structure */
        newmultiplex = (struct dvbcfg_multiplex*) malloc(sizeof(struct dvbcfg_multiplex));
        if (newmultiplex == NULL)
                return NULL;
        memset(newmultiplex, 0, sizeof(struct dvbcfg_multiplex));
        newmultiplex->source = source;
        memcpy(&newmultiplex->umid, umid, sizeof(struct dvbcfg_umid));

        /* add it to the list */
        if (*multiplexes == NULL)
                *multiplexes = newmultiplex;
        else {
                curmultiplex = *multiplexes;
                while(curmultiplex->next)
                        curmultiplex = curmultiplex->next;
                curmultiplex->next = newmultiplex;
        }

        return newmultiplex;
}

struct dvbcfg_service* dvbcfg_multiplex_add_service(struct dvbcfg_multiplex* multiplex,
                                                    char* name,
                                                    char* short_name,
                                                    char* provider_name,
                                                    char* usidstr,
                                                    uint32_t service_flags)
{
        struct dvbcfg_usid usid;

        if (dvbcfg_usid_from_string(usidstr, &usid))
               return NULL;

        return dvbcfg_multiplex_add_service2(multiplex, name, short_name,
                                             provider_name, &usid, service_flags);
}

struct dvbcfg_service* dvbcfg_multiplex_add_service2(struct dvbcfg_multiplex* multiplex,
                                                     char* name,
                                                     char* short_name,
                                                     char* provider_name,
                                                     struct dvbcfg_usid* usid,
                                                     uint32_t service_flags)
{
        struct dvbcfg_service* service;
        struct dvbcfg_service* curservice;

        service = (struct dvbcfg_service*) malloc(sizeof(struct dvbcfg_service));
        if (service == NULL)
                return NULL;
        memset(service, 0, sizeof(struct dvbcfg_service));

        memcpy(&service->usid, usid, sizeof(struct dvbcfg_usid));
        service->name = dvbcfg_strdupandtrim(name, -1);
        if (short_name)
          service->short_name = dvbcfg_strdupandtrim(short_name, -1);
        if (provider_name)
          service->provider_name = dvbcfg_strdupandtrim(provider_name, -1);
        service->service_flags = service_flags;

        /* add it to the list */
        if (multiplex->services == NULL)
                multiplex->services = service;
        else {
                curservice = multiplex->services;
                while(curservice->next)
                        curservice = curservice->next;
                curservice->next = service;
        }

        return service;
}

void dvbcfg_multiplex_remove_service(struct dvbcfg_multiplex* multiplex,
                                    struct dvbcfg_service* service)
{
        struct dvbcfg_service *next;
        struct dvbcfg_service *cur;

        next = service->next;

        /* free internal structures */
        if (service->name)
                free(service->name);
        if (service->short_name)
                free(service->short_name);
        if (service->provider_name)
                free(service->provider_name);
        free(service->ca_systems);
        free(service->zap_pids);
        free(service->pmt_extra);
        free(service);

        /* adjust pointers */
        if (multiplex->services == service)
                multiplex->services = next;
        else {
                cur = multiplex->services;
                while((cur->next != service) && (cur->next))
                        cur = cur->next;
                if (cur->next == service)
                        cur->next = next;
        }
}

int dvbcfg_multiplex_add_ca_system(struct dvbcfg_service* service,
                                   uint16_t ca_system_id)
{
        uint16_t* tmp;
        int i;

        /* check it isn't already there */
        for (i=0; i< service->ca_systems_count; i++) {
                if (service->ca_systems[i] == ca_system_id)
                        return 0;
        }

        if (service->ca_systems_count == 0) {
                service->ca_systems = (uint16_t*) malloc(sizeof(uint16_t));
                if (service->ca_systems == NULL)
                        return -ENOMEM;
                service->ca_systems[0] = ca_system_id;
                service->ca_systems_count = 1;
        } else {
                tmp = service->ca_systems;
                service->ca_systems = (uint16_t*)
                    realloc(service->ca_systems, sizeof(uint16_t) * (service->ca_systems_count + 1));
                if (service->ca_systems == NULL) {
                        service->ca_systems = tmp;
                        return -ENOMEM;
                }
                service->ca_systems[service->ca_systems_count++] = ca_system_id;
        }

        return 0;
}

int dvbcfg_multiplex_remove_ca_system(struct dvbcfg_service* service,
                                      uint16_t ca_system_id)
{
        uint16_t* tmp;
        int i;

        if (service->ca_systems == NULL)
                return -EINVAL;

        for (i=0; i< service->ca_systems_count; i++) {
                if (service->ca_systems[i] == ca_system_id)
                        break;
        }
        if (i >= service->ca_systems_count)
                return -EINVAL;

        tmp = (uint16_t*) malloc(sizeof(uint16_t) * (service->ca_systems_count-1));
        if (tmp == NULL)
                return -ENOMEM;
        memcpy(tmp, service->ca_systems, sizeof(uint16_t) * i);
        memcpy(tmp + (sizeof(uint16_t) * i),
              service->ca_systems + (sizeof(uint16_t) * (i + 1)),
              sizeof(uint16_t) * (service->ca_systems_count - i - 1));

        free(service->ca_systems);
        service->ca_systems = tmp;
        service->ca_systems_count--;

        return 0;
}

int dvbcfg_multiplex_add_zap_pid(struct dvbcfg_service* service,
                                 int pid,
                                 int type)
{
        return add_pid(&service->zap_pids_count, &service->zap_pids, pid, type);
}

int dvbcfg_multiplex_remove_zap_pid(struct dvbcfg_service* service,
                                    int pid,
                                    int type)
{
        return remove_pid(&service->zap_pids_count, &service->zap_pids, pid, type);
}

int dvbcfg_multiplex_add_pmt_extra(struct dvbcfg_service* service,
                                 int pid,
                                 int type)
{
        return add_pid(&service->pmt_extra_count, &service->pmt_extra, pid, type);
}

int dvbcfg_multiplex_remove_pmt_extra(struct dvbcfg_service* service,
                                      int pid,
                                      int type)
{
        return remove_pid(&service->pmt_extra_count, &service->pmt_extra, pid, type);
}

static int add_pid(int* count, struct dvbcfg_pid** pids, int pid, int type) {
        struct dvbcfg_pid* tmp;
        int i;

        /* check it isn't already there */
        for (i=0; i< *count; i++) {
                if (((*pids)[i].pid == pid) && ((*pids[i]).type == type))
                        return 0;
        }

        if (*count == 0) {
                *pids = (struct dvbcfg_pid*) malloc(sizeof(struct dvbcfg_pid));
                if (*pids == NULL)
                        return -ENOMEM;
                (*pids)[0].pid = pid;
                (*pids)[0].type = type;
                *count = 1;
        } else {
                tmp = *pids;
                *pids = (struct dvbcfg_pid*) realloc(*pids, sizeof(struct dvbcfg_pid) * (*count + 1));
                if (*pids == NULL) {
                        *pids = tmp;
                        return -ENOMEM;
                }
                (*pids)[*count].pid = pid;
                (*pids)[*count].type = type;
                (*count)++;
        }

        return 0;
}

static int remove_pid(int* count, struct dvbcfg_pid** pids, int pid, int type)
{
        struct dvbcfg_pid* tmp;
        int i;

        if (*pids == NULL)
                return -EINVAL;

        if ((pid == -1) && (type == -1)) {
                free(*pids);
                *pids = NULL;
                *count = 0;
                return 0;
        }

        /* we keep removing until there are no more matches */
        while(1) {
                /* does the pid/type combo exist? */
                for (i=0; i< *count; i++) {
                        if (((pid == -1) || ((*pids)[i].pid == pid)) &&
                            ((type == -1) || ((*pids)[i].type == type)))
                                break;
                }
                if (i >= *count)
                        break;

                tmp = (struct dvbcfg_pid*) malloc(sizeof(struct dvbcfg_pid) * (*count-1));
                if (tmp == NULL)
                        return -ENOMEM;
                memcpy(tmp, *pids, sizeof(struct dvbcfg_pid) * i);
                memcpy(tmp + (sizeof(struct dvbcfg_pid) * i),
                      *pids + (sizeof(struct dvbcfg_pid) * (i + 1)),
                      sizeof(struct dvbcfg_pid) * (*count - i - 1));

                free(*pids);
                *pids = tmp;
                (*count)--;
        }

        return 0;
}

struct dvbcfg_multiplex *dvbcfg_multiplex_find(struct dvbcfg_multiplex *multiplexes, char* gmidstr)
{
        struct dvbcfg_gmid gmid;
        struct dvbcfg_multiplex* multiplex;

        if (dvbcfg_gmid_from_string(gmidstr, &gmid))
                return NULL;

        multiplex = dvbcfg_multiplex_find2(multiplexes, &gmid);

        dvbcfg_source_id_free(&gmid.source_id);
        return multiplex;
}

struct dvbcfg_multiplex *dvbcfg_multiplex_find2(struct dvbcfg_multiplex *multiplexes, struct dvbcfg_gmid* gmid)
{
        while (multiplexes) {
          if (dvbcfg_source_id_equal(&gmid->source_id, &multiplexes->source->source_id, 0) &&
              dvbcfg_umid_equal(&gmid->umid, &multiplexes->umid))
                        return multiplexes;

                multiplexes = multiplexes->next;
        }

        return NULL;
}

struct dvbcfg_service *dvbcfg_multiplex_find_service_in_multiplex(struct dvbcfg_multiplex *multiplex, char* usidstr)
{
        struct dvbcfg_usid usid;

        if (dvbcfg_usid_from_string(usidstr, &usid))
                return NULL;

        return dvbcfg_multiplex_find_service_in_multiplex2(multiplex, &usid);
}

struct dvbcfg_service *dvbcfg_multiplex_find_service_in_multiplex2(struct dvbcfg_multiplex *multiplex,
                                                                   struct dvbcfg_usid* usid)
{
        struct dvbcfg_service* service;

        service = multiplex->services;
        while(service) {
                if (dvbcfg_usid_equal(&service->usid, usid))
                        return service;

                service = service->next;
        }

        return NULL;
}

struct dvbcfg_service *dvbcfg_multiplex_find_service(struct dvbcfg_multiplex *multiplexes, char* gsidstr)
{
        struct dvbcfg_gsid gsid;
        struct dvbcfg_service* service;

        if (dvbcfg_gsid_from_string(gsidstr, &gsid))
                return NULL;

        service = dvbcfg_multiplex_find_service2(multiplexes, &gsid);
        dvbcfg_source_id_free(&gsid.gmid.source_id);

        return service;
}

struct dvbcfg_service *dvbcfg_multiplex_find_service2(struct dvbcfg_multiplex *multiplexes, struct dvbcfg_gsid* gsid)
{
        struct dvbcfg_multiplex* multiplex;

        multiplex = dvbcfg_multiplex_find2(multiplexes, &gsid->gmid);
        if (multiplex == NULL)
                return NULL;

        return dvbcfg_multiplex_find_service_in_multiplex2(multiplex, &gsid->usid);
}

void dvbcfg_multiplex_free(struct dvbcfg_multiplex **multiplexes,
                           struct dvbcfg_multiplex *tofree)
{
        struct dvbcfg_multiplex *next;
        struct dvbcfg_multiplex *cur;

        next = tofree->next;

        /* free internal structures */
        while(tofree->services)
                dvbcfg_multiplex_remove_service(tofree, tofree->services);
        free(tofree);

        /* adjust pointers */
        if (*multiplexes == tofree)
                *multiplexes = next;
        else {
                cur = *multiplexes;
                while((cur->next != tofree) && (cur->next))
                        cur = cur->next;
                if (cur->next == tofree)
                        cur->next = next;
        }
}

void dvbcfg_multiplex_free_all(struct dvbcfg_multiplex *multiplexes)
{
        while (multiplexes)
                dvbcfg_multiplex_free(&multiplexes, multiplexes);
}

void dvbcfg_multiplex_postprocess(struct dvbcfg_multiplex** multiplexes)
{
        struct dvbcfg_multiplex* curmultiplex;
        struct dvbcfg_service* curservice;
        struct dvbcfg_multiplex* testmultiplex;
        struct dvbcfg_service* testservice;
        int i;

        /* first of all, set all differentiators to 0 */
        curmultiplex = *multiplexes;
        while(curmultiplex) {
                curmultiplex->umid.multiplex_differentiator = 0;

                curservice = curmultiplex->services;
                while(curservice) {
                        curservice->usid.service_differentiator = 0;
                        curservice = curservice->next;
                }

                curmultiplex = curmultiplex->next;
        }

        /* next, filter out any duplicate multiplexes */
        curmultiplex = *multiplexes;
        while(curmultiplex) {

restart_dupefilter:
                testmultiplex = *multiplexes;
                while(testmultiplex) {
                        if (testmultiplex != curmultiplex) {
                                if (dvbcfg_source_id_equal(&testmultiplex->source->source_id, &curmultiplex->source->source_id, 0) &&
                                    dvbcfg_umid_equal(&testmultiplex->umid, &curmultiplex->umid) &&
                                    (dvbcfg_multiplex_calculate_differentiator(testmultiplex) == dvbcfg_multiplex_calculate_differentiator(curmultiplex))) {
                                        dvbcfg_multiplex_free(multiplexes, testmultiplex);
                                        goto restart_dupefilter;
                                }
                        }

                        testmultiplex = testmultiplex->next;
                }

                curmultiplex = curmultiplex->next;
        }

        /* now, differentiate each multiplex and service where necessary */
        curmultiplex = *multiplexes;
        while(curmultiplex) {

                /* do the multiplex */
restart_differentiator:
                testmultiplex = *multiplexes;
                while(testmultiplex) {
                        if (testmultiplex != curmultiplex) {
                                if (dvbcfg_source_id_equal(&testmultiplex->source->source_id, &curmultiplex->source->source_id, 0) &&
                                    dvbcfg_umid_equal(&testmultiplex->umid, &curmultiplex->umid)) {
                                        testmultiplex->umid.multiplex_differentiator = dvbcfg_multiplex_calculate_differentiator(testmultiplex);
                                        goto restart_differentiator;
                                }
                        }

                        testmultiplex = testmultiplex->next;
                }

                /* now differentiate the services for this multiplex */
                curservice = curmultiplex->services;
                while(curservice) {

restart_differentiator_service:
                        i = 0;
                        testservice = curmultiplex->services;
                        while(testservice) {
                                if (testservice != curservice) {
                                        if (dvbcfg_usid_equal(&testservice->usid, &curservice->usid)) {
                                                testservice->usid.service_differentiator = i;
                                                goto restart_differentiator_service;
                                        }
                                }

                                i++;
                                testservice = testservice->next;
                        }

                        curservice = curservice->next;
                }

                /* next multiplex! */
                curmultiplex = curmultiplex->next;
        }
}

uint32_t dvbcfg_multiplex_calculate_differentiator(struct dvbcfg_multiplex* multiplex)
{
        uint32_t tmp = 0;

        switch(multiplex->source->source_id.source_type) {
        case DVBCFG_SOURCETYPE_DVBS:
                tmp = multiplex->delivery.dvb.fe_params.frequency / (multiplex->delivery.dvb.fe_params.u.qpsk.symbol_rate / 1000);
                tmp <<= 2;
                tmp |= (multiplex->delivery.dvb.polarization & 3);
                return tmp;

        case DVBCFG_SOURCETYPE_DVBC:
                return multiplex->delivery.dvb.fe_params.frequency / multiplex->delivery.dvb.fe_params.u.qam.symbol_rate;

        case DVBCFG_SOURCETYPE_DVBT:
                switch(multiplex->delivery.dvb.fe_params.u.ofdm.bandwidth) {
                case BANDWIDTH_8_MHZ:
                        tmp = 8000000;
                        break;

                case BANDWIDTH_7_MHZ:
                        tmp = 7000000;
                        break;

                case BANDWIDTH_AUTO:
                case BANDWIDTH_6_MHZ:
                        tmp = 6000000;
                        break;
                }
                return multiplex->delivery.dvb.fe_params.frequency / tmp;

        case DVBCFG_SOURCETYPE_ATSC:
                return multiplex->delivery.dvb.fe_params.frequency / 6000000;
        }

        return 0;
}
