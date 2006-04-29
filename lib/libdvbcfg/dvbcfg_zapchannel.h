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

#include <stdio.h>
#include <stdint.h>
#include <libdvbapi/dvbfe.h>

struct dvbcfg_zapchannel
{
        char name[128];
        dvbfe_type_t fe_type;
        struct dvbfe_parameters fe_params;
        int satellite_switch;
        int video_pid;
        int audio_pid;
        int channel_number;

	/* these two are not used by this library - they're provided as a
	 * convenience for applications to use */
	struct dvbcfg_zapchannel *next;
	struct dvbcfg_zapchannel *prev;
};

/**
 * Callback function used in dvbcfg_zapchannel_load().
 *
 * @param private Private information to caller.
 * @param channel The current channel details.
 * @return 0 to continue, 1 to stop loading.
 */
typedef int (*dvbcfg_zapchannel_callback)(void *private, struct dvbcfg_zapchannel *channel);

/**
 * Load a *zap format channels file.
 *
 * @param f File to load from.
 * @param private Value to pass to 'private' in callback above.
 * @param cb Callback function called for each channel loaded from the file.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_zapchannel_load(FILE *f, void *private,
				  dvbcfg_zapchannel_callback cb);

/**
 * Convenience function to parse a *zap config file and find details of a channel.
 *
 * @param config_file Config filename to load.
 * @param channel_name Name of channel to find.
 * @param channel Where to put the details if found.
 * @return 0 on success, nonzero on error.
 */
extern int dvbcfg_zapchannel_find(const char *config_file,
				  char *channel_name,
				  struct dvbcfg_zapchannel *channel);

/**
 * Save *zap format channels to a config file.
 *
 * @param f File to save to.
 * @param channels Pointer to array of channels to save.
 * @param count Number of entries in the above array.
 * @return 0 on success, or nonzero error code on failure.
 */
extern int dvbcfg_zapchannel_save(FILE *f,
                                  struct dvbcfg_zapchannel *channels,
				  int count);

#ifdef __cplusplus
}
#endif

#endif
