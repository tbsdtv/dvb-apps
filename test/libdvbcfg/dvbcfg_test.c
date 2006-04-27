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
#include <string.h>
#include <libdvbcfg/dvbcfg_vdrchannel.h>
#include <libdvbcfg/dvbcfg_zapchannel.h>

void syntax(void);

int main(int argc, char *argv[])
{
        if (argc != 4) {
                syntax();
        }

        if (!strcmp(argv[1], "-vdrchannel")) {
                struct dvbcfg_vdrchannel *vdrchannels = NULL;

                dvbcfg_vdrchannel_load(argv[2], &vdrchannels);
                dvbcfg_vdrchannel_free_all(vdrchannels);

        } else if (!strcmp(argv[1], "-zapchannel")) {
                struct dvbcfg_zapchannel *zapchannels = NULL;

                dvbcfg_zapchannel_load(argv[2], &zapchannels);
                dvbcfg_zapchannel_save(argv[3], zapchannels);
                dvbcfg_zapchannel_free_all(zapchannels);

        } else {
                syntax();
        }

        exit(0);
}

void syntax()
{
        fprintf(stderr,
                "Syntax: dvcfg_test <-vdrchannel|-zapchannel> <input filename> <output filename>\n");
        exit(1);
}
