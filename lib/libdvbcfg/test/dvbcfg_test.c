/**
 * dvbcfg testing.
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

#include <stdlib.h>
#include <stdio.h>
#include "dvbcfg_source_backend_file.h"
#include "dvbcfg_diseqc_backend_file.h"
#include "dvbcfg_adapter_backend_file.h"
#include "dvbcfg_multiplex_backend_file.h"
#include "dvbcfg_seed_backend_file.h"
#include "dvbcfg_vdrchannel.h"
#include "dvbcfg_zapchannel.h"

void syntax();


int main(int argc, char *argv[])
{
        struct dvbcfg_source *sources = NULL;
        struct dvbcfg_source_backend* source_backend;
        struct dvbcfg_diseqc_backend* diseqc_backend;
        struct dvbcfg_adapter_backend* adapter_backend;
        struct dvbcfg_multiplex_backend* multiplex_backend;
        struct dvbcfg_seed_backend* seed_backend;

        if (argc != 5) {
                syntax();
        }

        if (!strcmp(argv[1], "-source")) {
                dvbcfg_source_backend_file_create(argv[2], &source_backend);
                dvbcfg_source_load(source_backend, &sources);
                dvbcfg_source_backend_file_destroy(source_backend);

                dvbcfg_source_backend_file_create(argv[3], &source_backend);
                dvbcfg_source_save(source_backend, sources);
                dvbcfg_source_backend_file_destroy(source_backend);
                dvbcfg_source_free_all(sources);

        } else if (!strcmp(argv[1], "-diseqc")) {
                struct dvbcfg_diseqc *diseqcs = NULL;

                dvbcfg_source_backend_file_create(argv[4], &source_backend);
                dvbcfg_source_load(source_backend, &sources);
                dvbcfg_source_backend_file_destroy(source_backend);

                dvbcfg_diseqc_backend_file_create(argv[2], &sources, 1, &diseqc_backend);
                dvbcfg_diseqc_load(diseqc_backend, &diseqcs);
                dvbcfg_diseqc_backend_file_destroy(diseqc_backend);

                dvbcfg_diseqc_backend_file_create(argv[3], &sources, 1, &diseqc_backend);
                dvbcfg_diseqc_save(diseqc_backend, diseqcs);
                dvbcfg_diseqc_backend_file_destroy(diseqc_backend);

                dvbcfg_diseqc_free_all(diseqcs);
                dvbcfg_source_free_all(sources);

        } else if (!strcmp(argv[1], "-vdrchannel")) {
                struct dvbcfg_vdrchannel *vdrchannels = NULL;

                dvbcfg_vdrchannel_load(argv[2], &vdrchannels);
                dvbcfg_vdrchannel_free_all(vdrchannels);

        } else if (!strcmp(argv[1], "-zapchannel")) {
                struct dvbcfg_zapchannel *zapchannels = NULL;

                dvbcfg_zapchannel_load(argv[2], &zapchannels);
                dvbcfg_zapchannel_save(argv[3], zapchannels);
                dvbcfg_zapchannel_free_all(zapchannels);

        } else if (!strcmp(argv[1], "-adapter")) {
                struct dvbcfg_adapter *adapters = NULL;

                dvbcfg_source_backend_file_create(argv[4], &source_backend);
                dvbcfg_source_load(source_backend, &sources);
                dvbcfg_source_backend_file_destroy(source_backend);

                dvbcfg_adapter_backend_file_create(argv[2], &sources, 1, &adapter_backend);
                dvbcfg_adapter_load(adapter_backend, &adapters);
                dvbcfg_adapter_backend_file_destroy(adapter_backend);

                dvbcfg_adapter_backend_file_create(argv[3], &sources, 1, &adapter_backend);
                dvbcfg_adapter_save(adapter_backend, adapters);
                dvbcfg_adapter_backend_file_destroy(adapter_backend);

                dvbcfg_adapter_free_all(adapters);
                dvbcfg_source_free_all(sources);

        } else if (!strcmp(argv[1], "-multiplex-long")) {
                struct dvbcfg_multiplex *multiplexes = NULL;

                dvbcfg_source_backend_file_create(argv[4], &source_backend);
                dvbcfg_source_load(source_backend, &sources);
                dvbcfg_source_backend_file_destroy(source_backend);

                dvbcfg_multiplex_backend_file_create(argv[2], &sources, 1, 1, &multiplex_backend);
                dvbcfg_multiplex_load(multiplex_backend, &multiplexes);
                dvbcfg_multiplex_backend_file_destroy(multiplex_backend);

                dvbcfg_multiplex_backend_file_create(argv[3], &sources, 1, 1, &multiplex_backend);
                dvbcfg_multiplex_save(multiplex_backend, multiplexes);
                dvbcfg_multiplex_backend_file_destroy(multiplex_backend);

                dvbcfg_multiplex_free_all(multiplexes);
                dvbcfg_source_free_all(sources);

        } else if (!strcmp(argv[1], "-multiplex-short")) {
                struct dvbcfg_multiplex *multiplexes = NULL;

                dvbcfg_source_backend_file_create(argv[4], &source_backend);
                dvbcfg_source_load(source_backend, &sources);
                dvbcfg_source_backend_file_destroy(source_backend);

                dvbcfg_multiplex_backend_file_create(argv[2], &sources, 1, 0, &multiplex_backend);
                dvbcfg_multiplex_load(multiplex_backend, &multiplexes);
                dvbcfg_multiplex_backend_file_destroy(multiplex_backend);

                dvbcfg_multiplex_backend_file_create(argv[3], &sources, 1, 0, &multiplex_backend);
                dvbcfg_multiplex_save(multiplex_backend, multiplexes);
                dvbcfg_multiplex_backend_file_destroy(multiplex_backend);

                dvbcfg_multiplex_free_all(multiplexes);
                dvbcfg_source_free_all(sources);

        } else if (!strcmp(argv[1], "-seed")) {
                struct dvbcfg_seed seed;

                dvbcfg_seed_init(&seed);

                dvbcfg_seed_backend_file_create(".", argv[2], 1, DVBCFG_SOURCETYPE_DVBS, &seed_backend);
                dvbcfg_seed_load(seed_backend, &seed);
                dvbcfg_seed_backend_file_destroy(seed_backend);

                dvbcfg_seed_backend_file_create(".", argv[3], 1, DVBCFG_SOURCETYPE_DVBS, &seed_backend);
                dvbcfg_seed_save(seed_backend, &seed);
                dvbcfg_seed_backend_file_destroy(seed_backend);

                dvbcfg_seed_clear(&seed);

        } else {
                syntax();
        }

        exit(0);
}

void syntax()
{
        fprintf(stderr,
                "Syntax: dvcfg_test <-source|-diseqc|-vdrchannel|-zapchannel|-adapter|-multiplex-long|-multiplex-short|-seed> <input filename> <output filename> <sources filename>\n");
        exit(1);
}
