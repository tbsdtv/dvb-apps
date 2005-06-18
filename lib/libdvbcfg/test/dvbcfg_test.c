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
#include "dvbcfg_source.h"
#include "dvbcfg_diseqc.h"
#include "dvbcfg_vdrchannel.h"
#include "dvbcfg_adapter.h"

void syntax();


int main(int argc, char* argv[])
{
  if (argc != 4) {
    syntax();
  }

  if (!strcmp(argv[1], "-source")) {
    struct dvbcfg_source* sources = NULL;

    dvbcfg_source_load(argv[2], &sources);
    dvbcfg_source_save(argv[3], sources);
    dvbcfg_source_free_all(sources);

  } else if (!strcmp(argv[1], "-diseqc")) {
    struct dvbcfg_diseqc* diseqcs = NULL;

    dvbcfg_diseqc_load(argv[2], &diseqcs);
    dvbcfg_diseqc_save(argv[3], diseqcs);
    dvbcfg_diseqc_free_all(diseqcs);

  } else if (!strcmp(argv[1], "-vdrchannel")) {
    struct dvbcfg_vdrchannel* vdrchannels = NULL;

    dvbcfg_vdrchannel_load(argv[2], &vdrchannels);
    dvbcfg_vdrchannel_free_all(vdrchannels);

  } else if (!strcmp(argv[1], "-adapter")) {
    struct dvbcfg_adapter* adapters = NULL;

    dvbcfg_adapter_load(argv[2], &adapters);
    dvbcfg_adapter_save(argv[3], adapters);
    dvbcfg_adapter_free_all(adapters);

  } else {
    syntax();
  }

  exit(0);
}

void syntax()
{
  fprintf(stderr, "Syntax: dvcfg_test <-source|-diseqc|-vdrchannel|-adapter> <input filename> <output filename>\n");
  exit(1);
}
