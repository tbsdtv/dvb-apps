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
#include <errno.h>
#include "dvbcfg_multiplex.h"
#include "dvbcfg_util.h"


int dvbcfg_multiplex_load(char *config_file,
                          struct dvbcfg_source** sources,
                          struct dvbcfg_multiplex** multiplexes,
                          int create_sources)
{
        FILE *in;
        char curline[256];
        char *linepos;
        struct dvbcfg_multiplex *newmultiplex;
        char* multiplex_id;
        int numtokens;
        int error = 0;

        /* open the file */
        in = fopen(config_file, "r");
        if (in == NULL)
                return errno;

        while (fgets(curline, sizeof(curline), in)) {
                linepos = curline;

                /* clean any comments/ whitespace */
                if (dvbcfg_cleanline(linepos) == 0)
                        continue;

                // FIXME
        }

exit:
        /* tidy up and return */
        fclose(in);
        return error;
}

int dvbcfg_multiplex_save(char *config_file, struct dvbcfg_multiplex *multiplexes)
{
        FILE *out;
        char* tmp;

        /* open the file */
        out = fopen(config_file, "w");
        if (out == NULL)
                return errno;

        while (multiplexes) {

                // FIXME

                multiplexes = multiplexes->next;
        }

        fclose(out);
        return 0;
}

struct dvbcfg_multiplex* dvbcfg_multiplex_new(struct dvbcfg_multiplex** multiplexes,
                                              struct dvbcfg_source* source,
                                              char* umid)
{
        struct dvbcfg_multiplex* newmultiplex;
        struct dvbcfg_multiplex* curmultiplex;

        /* create new structure */
        newmultiplex = (struct dvbcfg_multiplex*) malloc(sizeof(struct dvbcfg_multiplex));
        if (newmultiplex == NULL)
                return NULL;
        memset(newmultiplex, 0, sizeof(struct dvbcfg_multiplex));
        newmultiplex->source = source;

        /* parse the umid */
        if (dvbcfg_umid_from_string(umid, &newmultiplex->umid)) {
                free(newmultiplex);
                return NULL;
        }

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
                                                    char* usid,
                                                    uint32_t service_flags)
{
  // FIXME
}

int dvbcfg_multiplex_remove_service(struct dvbcfg_multiplex* multiplex,
                                    struct dvbcfg_service* service)
{
  // FIXME
}

int dvbcfg_multiplex_add_ca_system(struct dvbcfg_service* service,
                                   uint16_t ca_system_id)
{
  // FIXME
}

int dvbcfg_multiplex_remove_ca_system(struct dvbcfg_service* service,
                                      uint16_t ca_system_id)
{
  // FIXME
}

int dvbcfg_multiplex_add_zap_pid(struct dvbcfg_service* service,
                                 uint16_t pid,
                                 uint16_t type)
{
  // FIXME
}

int dvbcfg_multiplex_remove_zap_pid(struct dvbcfg_service* service,
                                    uint16_t pid,
                                    uint16_t type)
{
  // FIXME
}

int dvbcfg_multiplex_add_pmt_pid(struct dvbcfg_service* service,
                                 uint16_t pid,
                                 uint16_t type)
{
  // FIXME
}

int dvbcfg_multiplex_remove_pmt_pid(struct dvbcfg_service* service,
                                    uint16_t pid,
                                    uint16_t type)
{
  // FIXME
}


struct dvbcfg_multiplex *dvbcfg_multiplex_find(struct dvbcfg_multiplex *multiplexes, struct dvbcfg_gmid* gmid)
{
        while (multiplexes) {
          if (dvbcfg_source_id_equal(&gmid->source_id, &multiplexes->source->source_id, 0) &&
              dvbcfg_umid_equal(&gmid->umid, &multiplexes->umid))
                        return multiplexes;

                multiplexes = multiplexes->next;
        }

        return NULL;
}

struct dvbcfg_service *dvbcfg_multiplex_find_service_in_multiplex(struct dvbcfg_multiplex *multiplex,
                                                                  struct dvbcfg_usid* usid)
{
  // FIXME
}

struct dvbcfg_service *dvbcfg_multiplex_find_service(struct dvbcfg_multiplex *multiplexes, struct dvbcfg_gsid* gsid)
{
  // FIXME
}

void dvbcfg_multiplex_free(struct dvbcfg_multiplex **multiplexes,
                           struct dvbcfg_multiplex *tofree)
{
        struct dvbcfg_multiplex *next;
        struct dvbcfg_multiplex *cur;

        next = tofree->next;

        /* free internal structures */
        while(tofree->services)
                dvbcfg_remove_service(tofree, tofree->services);
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
