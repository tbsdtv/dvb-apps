/**
 * dvbcfg_vdrchannel configuration file support.
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

#ifndef DVBCFG_VDRCHANNEL_H
#define DVBCFG_VDRCHANNEL_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <linux/dvb/frontend.h>


#define DVBCFG_VDRCHANNEL_AUDIO_MPEG 0
#define DVBCFG_VDRCHANNEL_AUDIO_DD_AC3 1

struct dvbcfg_vdrchannel_audio {
        uint16_t pid;
        char lang[4];
        char type:1;

        struct dvbcfg_vdrchannel_audio *next;
};

#define DVBCFG_VDRCHANNEL_MAXCAIDS 8

struct dvbcfg_vdrchannel {
        char *channel_name;
        char *provider_name;
        char *source_name;
        char *short_name;
        fe_type_t fe_type;
        uint16_t video_pid;
        uint16_t pcr_pid;
        struct dvbcfg_vdrchannel_audio *audio_streams;
        uint16_t teletext_pid;
        uint16_t service_id;
        uint16_t original_network_id;
        uint16_t transport_stream_id;
        uint16_t radio_id;
        uint16_t caids[DVBCFG_VDRCHANNEL_MAXCAIDS];
        int num_caids;
        struct dvb_frontend_parameters fe_params;
        uint8_t polarization;

        struct dvbcfg_vdrchannel *prev; /* NULL=> first entry */
        struct dvbcfg_vdrchannel *next; /* NULL=> last entry */
};

/**
 * Load dvbcfg_vdrchannels a VDR format channels file.
 *
 * @param config_file Config filename to load.
 * @param channels Where to put the pointer to the start of the loaded
 * channels. If NULL, a new list will be created, if it points to an already initialised list,
 * the loaded channels will be appended to it.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_vdrchannel_load(const char *config_file,
                                 struct dvbcfg_vdrchannel **channels);

/**
 * Unlink a single dvbcfg_vdrchannel from a list, and free its memory.
 *
 * @param channels The list of dvbcfg_vdrchannels.
 * @param tofree The source to free.
 */
extern void dvbcfg_vdrchannel_free(struct dvbcfg_vdrchannel **channels,
                                   struct dvbcfg_vdrchannel *tofree);

/**
 * Free memory for all dvbcfg_vdrchannels in a list.
 *
 * @param sources Pointer to list of dvbcfg_vdrchannels to free.
 */
extern void dvbcfg_vdrchannel_free_all(struct dvbcfg_vdrchannel *channels);

#ifdef __cplusplus
}
#endif

#endif
