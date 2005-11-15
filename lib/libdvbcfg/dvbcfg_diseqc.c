/**
 * dvbcfg_diseqc configuration file support.
 *
 * Copyright (c) 2005 by Andrew de Quincey <adq_dvb@lidskialf.net>
 *
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
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include "dvbcfg_diseqc.h"
#include "dvbcfg_common.h"
#include "dvbcfg_util.h"

int dvbcfg_diseqc_load(struct dvbcfg_diseqc_backend *backend,
                       struct dvbcfg_diseqc **diseqcs)
{
        int status;
        while(!(status = backend->get(backend, diseqcs)));

        if (status < 0)
                return status;

        return 0;
}

int dvbcfg_diseqc_save(struct dvbcfg_diseqc_backend *backend,
                       struct dvbcfg_diseqc *diseqcs)
{
        int status;

        while(diseqcs) {
                if ((status = backend->put(backend, diseqcs)) != 0)
                        return status;

                diseqcs = diseqcs->next;
        }

        return 0;
}

struct dvbcfg_diseqc* dvbcfg_diseqc_new(struct dvbcfg_diseqc** diseqcs,
                                        struct dvbcfg_source* source)
{
        struct dvbcfg_diseqc* newdiseqc;
        struct dvbcfg_diseqc* curdiseqc;

        /* create new structure */
        newdiseqc = (struct dvbcfg_diseqc*) malloc(sizeof(struct dvbcfg_diseqc));
        if (newdiseqc == NULL)
                return NULL;
        memset(newdiseqc, 0, sizeof(struct dvbcfg_diseqc));
        newdiseqc->source = source;

        /* add it to the list */
        if (*diseqcs == NULL)
                *diseqcs = newdiseqc;
        else {
                curdiseqc = *diseqcs;
                while(curdiseqc->next)
                        curdiseqc = curdiseqc->next;
                curdiseqc->next = newdiseqc;
        }

        return newdiseqc;
}

struct dvbcfg_diseqc_entry* dvbcfg_diseqc_add_entry(struct dvbcfg_diseqc* diseqc,
                                                    uint32_t slof,
                                                    uint8_t polarization,
                                                    uint32_t lof, char *command)
{
        struct dvbcfg_diseqc_entry* newentry;
        struct dvbcfg_diseqc_entry* curentry;

        /* create new structure */
        newentry = (struct dvbcfg_diseqc_entry*) malloc(sizeof(struct dvbcfg_diseqc_entry));
        if (newentry == NULL)
                return NULL;
        memset(newentry, 0, sizeof(struct dvbcfg_diseqc_entry));
        newentry->slof = slof;
        newentry->polarization = polarization;
        newentry->lof = lof;
        newentry->command = dvbcfg_strdupandtrim(command, -1);

        /* add it to the list */
        if (diseqc->entries == NULL)
                diseqc->entries = newentry;
        else {
                curentry = diseqc->entries;
                while(curentry->next)
                        curentry = curentry->next;
                curentry->next = newentry;
        }

        return newentry;
}

void dvbcfg_diseqc_remove_entry(struct dvbcfg_diseqc* diseqc,
                                struct dvbcfg_diseqc_entry* entry)
{
        struct dvbcfg_diseqc_entry *next;
        struct dvbcfg_diseqc_entry *cur;

        next = entry->next;

        /* free internal structures */
        if (entry->command)
                free(entry->command);
        free(entry);

        /* adjust pointers */
        if (diseqc->entries == entry)
                diseqc->entries = next;
        else {
                cur = diseqc->entries;
                while((cur->next != entry) && (cur->next))
                        cur = cur->next;
                if (cur->next == entry)
                        cur->next = next;
        }
}

struct dvbcfg_diseqc *dvbcfg_diseqc_find(struct dvbcfg_diseqc *diseqcs,
                                         struct dvbcfg_source* source)
{
        while (diseqcs) {
		if (source == diseqcs->source) {
                        return diseqcs;
		}

                diseqcs = diseqcs->next;
        }

        return NULL;
}

struct dvbcfg_diseqc_entry *dvbcfg_diseqc_find_entry(struct dvbcfg_diseqc* diseqc,
                                                     uint32_t frequency,
                                                     int polarization)
{
        struct dvbcfg_diseqc_entry *entry;
        struct dvbcfg_diseqc_entry *candidate = NULL;

        entry = diseqc->entries;
        while (entry) {
                if ((polarization == entry->polarization) &&
                    (frequency < entry->slof)) {
                        if (candidate) {
                                if (entry->slof < candidate->slof)
                                        candidate = entry;
                        } else {
                                candidate = entry;
                        }
                }

                entry = entry->next;
        }

        return candidate;
}

void dvbcfg_diseqc_free(struct dvbcfg_diseqc **diseqcs,
                        struct dvbcfg_diseqc *tofree)
{
        struct dvbcfg_diseqc *next;
        struct dvbcfg_diseqc *cur;

        next = tofree->next;

        /* free internal structures */
        while(tofree->entries)
                dvbcfg_diseqc_remove_entry(tofree, tofree->entries);
        free(tofree);

        /* adjust pointers */
        if (*diseqcs == tofree)
                *diseqcs = next;
        else {
                cur = *diseqcs;
                while((cur->next != tofree) && (cur->next))
                        cur = cur->next;
                if (cur->next == tofree)
                        cur->next = next;
        }
}

void dvbcfg_diseqc_free_all(struct dvbcfg_diseqc *diseqcs)
{
        while (diseqcs)
                dvbcfg_diseqc_free(&diseqcs, diseqcs);
}
