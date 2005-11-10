/**
 * dvbcfg_zapchannel (i.e. linuxtv *zap format) configuration file support.
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

#ifndef DVBCFG_ZAPCHANNEL_H
#define DVBCFG_ZAPCHANNEL_H 1

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include <linux/dvb/frontend.h>

struct dvbcfg_zapchannel
{
        char* name;
        fe_type_t fe_type;
        struct dvb_frontend_parameters fe_params;
        int polarization;
        int satellite_switch;
        int video_pid;
        int audio_pid;
        int channel_number;

        struct dvbcfg_zapchannel *prev; /* NULL=> first entry */
        struct dvbcfg_zapchannel *next; /* NULL=> last entry */
};

/**
 * Load a *zap format channels file.
 *
 * @param config_file Config filename to load.
 * @param channels Where to put the pointer to the start of the loaded
 * channels. If NULL, a new list will be created, if it points to an already initialised list,
 * the loaded channels will be appended to it.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_zapchannel_load(const char *config_file,
                                  struct dvbcfg_zapchannel **channels);

/**
 * Save *zap format channels to a config file.
 *
 * @param config_file Config filename to save.
 * @param sources Pointer to the list of channels to save.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_zapchannel_save(const char *config_file,
                                  struct dvbcfg_zapchannel *channels);

/**
 * Find the entry for a particular channel name.
 *
 * @param channels Pointer to the list to search.
 * @param name Name of channel to find.
 * @return A dvbcfg_zapchannel structure if found, or NULL if not.
 */
extern struct dvbcfg_zapchannel *dvbcfg_zapchannel_find(struct dvbcfg_zapchannel* channels, char *name);

/**
 * Unlink a single dvbcfg_zapchannel from a list, and free its memory.
 *
 * @param channels The list of dvbcfg_zapchannels.
 * @param tofree The source to free.
 */
extern void dvbcfg_zapchannel_free(struct dvbcfg_zapchannel **channels,
                                   struct dvbcfg_zapchannel *tofree);

/**
 * Free memory for all dvbcfg_zapchannels in a list.
 *
 * @param sources Pointer to list of dvbcfg_zapchannels to free.
 */
extern void dvbcfg_zapchannel_free_all(struct dvbcfg_zapchannel *channels);

#ifdef __cplusplus
}
#endif

#endif
