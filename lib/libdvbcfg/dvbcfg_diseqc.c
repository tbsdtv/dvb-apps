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

int dvbcfg_diseqc_load(const char *config_file,
                       struct dvbcfg_source** sources,
                       struct dvbcfg_diseqc** diseqcs,
                       int create_sources)
{
        FILE *in;
        char curline[256];
        char *linepos;
        struct dvbcfg_source_id source_id;
        uint32_t slof;
        uint8_t polarization;
        uint32_t lof;
        char *command;
        struct dvbcfg_source *source;
        struct dvbcfg_diseqc *curdiseqc;
        int numtokens;
        int error = 0;
        int val;

        /* open the file */
        in = fopen(config_file, "r");
        if (in == NULL)
                return errno;

        while (fgets(curline, sizeof(curline), in)) {
                linepos = curline;

                /* clean any comments/ whitespace */
                if (dvbcfg_cleanline(linepos) == 0)
                        continue;

                /* tokenise the line */
                numtokens = dvbcfg_tokenise(linepos, " \t", 4, 1);
                if ((numtokens < 4) || (numtokens > 5))
                        continue;

                /* the source id */
                if (dvbcfg_source_id_from_string(linepos, &source_id)) {
                  error = -ENOMEM;
                  goto exit;
                }

                /* try to find the source */
                source = dvbcfg_source_find2(*sources, &source_id);
                if (source == NULL) {
                        if (!create_sources) {
                                dvbcfg_source_id_free(&source_id);
                                continue;
                        }

                        source = dvbcfg_source_new2(sources, &source_id, "???");
                        dvbcfg_source_id_free(&source_id);
                        if (source == NULL) {
                                error = -ENOMEM;
                                goto exit;
                        }
                } else {
                        dvbcfg_source_id_free(&source_id);
                }

                /* find/create the diseqc */
                curdiseqc = dvbcfg_diseqc_find(*diseqcs, source);
                if (curdiseqc == NULL) {
                        /* create the diseqc */
                        curdiseqc = dvbcfg_diseqc_new(diseqcs, source);
                        if (curdiseqc == NULL) {
                                error = -ENOMEM;
                                goto exit;
                        }
                }
                linepos = dvbcfg_nexttoken(linepos);

                /* the SLOF */
                if (sscanf(linepos, "%d", &val) != 1)
                        continue;
                slof = val * 1000;  // want it in kHz
                linepos = dvbcfg_nexttoken(linepos);

                /* the polarization */
                if (linepos[0] == 'H')
                        polarization = DVBCFG_POLARIZATION_H;
                else if (linepos[0] == 'V')
                        polarization = DVBCFG_POLARIZATION_V;
                else if (linepos[0] == 'L')
                        polarization = DVBCFG_POLARIZATION_L;
                else if (linepos[0] == 'R')
                        polarization = DVBCFG_POLARIZATION_R;
                else
                        continue;
                linepos = dvbcfg_nexttoken(linepos);

                /* LOF */
                if (sscanf(linepos, "%d", &val) != 1)
                        continue;
                lof = val * 1000;   // want it in kHz
                linepos = dvbcfg_nexttoken(linepos);

                /* command */
                if (numtokens == 5)
                        command = linepos;
                else
                        command = "";

                /* create a new entry */
                if (dvbcfg_diseqc_add_entry(curdiseqc, slof, polarization, lof, command) == NULL) {
                        error = -ENOMEM;
                        goto exit;
                }
        }

exit:
        /* tidy up and return */
        fclose(in);
        return error;
}

int dvbcfg_diseqc_save(const char *config_file,
		       struct dvbcfg_diseqc *diseqcs)
{
        FILE *out;
        char polarization = 'H';
        struct dvbcfg_diseqc_entry *entry;
        char* source_id;

        /* open the file */
        out = fopen(config_file, "w");
        if (out == NULL)
                return errno;

        while (diseqcs) {

                source_id = dvbcfg_source_id_to_string(&diseqcs->source->source_id);
                if (source_id == NULL)
                        break;

                entry = diseqcs->entries;
                while (entry) {
                        switch (entry->polarization) {
                        case DVBCFG_POLARIZATION_H:
                                polarization = 'H';
                                break;

                        case DVBCFG_POLARIZATION_V:
                                polarization = 'V';
                                break;

                        case DVBCFG_POLARIZATION_L:
                                polarization = 'L';
                                break;

                        case DVBCFG_POLARIZATION_R:
                                polarization = 'R';
                                break;
                        }

                        fprintf(out, "%s %d %c %d %s\n",
                                source_id, entry->slof / 1000,
                                polarization, entry->lof / 1000,
                                entry->command);

                        entry = entry->next;
                }

                free(source_id);
                diseqcs = diseqcs->next;
        }

        fclose(out);
        return 0;
}

struct dvbcfg_diseqc* dvbcfg_diseqc_new(struct dvbcfg_diseqc** diseqcs, struct dvbcfg_source* source)
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
                                                    uint32_t slof, uint8_t polarization, uint32_t lof, char *command)
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

void dvbcfg_diseqc_remove_entry(struct dvbcfg_diseqc* diseqc, struct dvbcfg_diseqc_entry* entry)
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
                if (source == diseqcs->source)
                        return diseqcs;

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
                if ((polarization == entry->polarization) && (frequency < entry->slof)) {
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
