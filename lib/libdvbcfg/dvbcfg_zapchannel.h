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

/**
 * The zapchannel file format specifies tuning parameters for channels. Each line describes
 * a single channel, and consists of multiple options separated by ':'. The exact
 * format of each line depends on the DVB type of the channel (i.e. DVBS, DVBT, DVBC, or ATSC).
 *
 * Note: the lines have been split across multiple lines in the following due to length issues.
 *
 * The format for DVBT channels is:
 *
 * <name>:<frequency>:<inversion>:<bandwidth>:<fec_hp>:<fec_lp>:
 * <constellation>:<transmission>:<guard_interval>:<hierarchy>:
 * <video_pid>:<audio_pid>:<channel_number>
 *
 * 	name: name of the channel
 * 	frequency: frequency in Hz
 * 	inversion: one of INVERSION_OFF, INVERSION_ON, or INVERSION_AUTO.
 * 	bandwidth: one of BANDWIDTH_6_MHZ, BANDWIDTH_7_MHZ, or BANDWIDTH_8_MHZ.
 * 	fec_hp: FEC of the high priority stream, one of: FEC_1_2, FEC_2_3,
 * 		FEC_3_4, FEC_4_5, FEC_5_6, FEC_6_7, FEC_7_8, FEC_8_9, or FEC_AUTO.
 * 	fec_lp: FEC of the low priority stream, one of: FEC_1_2, FEC_2_3,
 * 		FEC_3_4, FEC_4_5, FEC_5_6, FEC_6_7, FEC_7_8, FEC_8_9, FEC_AUTO, or FEC_NONE.
 * 	constellation: one of QPSK, QAM_128, QAM_16, QAM_256, QAM_32, or QAM_64.
 * 	transmission: one of TRANSMISSION_MODE_2K, or TRANSMISSION_MODE_8K.
 * 	guard_interval: one of GUARD_INTERVAL_1_32, GUARD_INTERVAL_1_16, GUARD_INTERVAL_1_8, or GUARD_INTERVAL_1_4.
 * 	hierarchy: one of HIERARCHY_NONE, HIERARCHY_1, HIERARCHY_2, or HIERARCHY_4.
 * 	video_pid: PID of the video stream.
 * 	audio_pid: PID of the audio stream.
 * 	channel_number: Transport stream channel number of the program.
 *
 * DVBC:
 *
 * <name>:<frequency>:<inversion>:<symbol_rate>:<fec>:
 * <modulation>:<video_pid>:<audio_pid>:<channel_number>
 *
 * 	name: name of the channel
 * 	frequency: frequency in Hz
 * 	inversion: one of INVERSION_OFF, INVERSION_ON, or INVERSION_AUTO.
 * 	symbol_rate: Symbol rate of the channel in ksyms.
 * 	fec: One of: FEC_1_2, FEC_2_3, FEC_3_4, FEC_4_5, FEC_5_6, FEC_6_7,
 * 			FEC_7_8, FEC_8_9, or FEC_AUTO.
 * 	modulation: one of QAM_16, QAM_32, QAM_64, QAM_128, QAM_256, QAM_AUTO.
 * 	video_pid: PID of the video stream.
 * 	audio_pid: PID of the audio stream.
 * 	channel_number: Transport stream channel number of the program.
 *
 * DVBS:
 *
 * <name>:<frequency>:<polarization>:<satellite_switches>:<symbol_rate>:<video_pid>:<audio_pid>:<channel_number>
 *
 * 	name: name of the channel
 * 	frequency: frequency in kHz
 * 	polarization: one of H,V,L, or R.
 * 	satellite_switches: Treated as a 2 bit value controlling switches in SEC equipment:
 * 		bit 0: controls "satellite switch", 0: A, 1: B
 * 		bit 1: controls "switch option", 0: A, 1: B
 * 	symbol_rate: Symbol rate of the channel in ksyms.
 * 	video_pid: PID of the video stream.
 * 	audio_pid: PID of the audio stream.
 * 	channel_number: Transport stream channel number of the program.
 *
 * ATSC:
 *
 * <name>:<frequency>:<inversion>:<modulation>:<video_pid>:<audio_pid>:<channel_number>
 *
 * 	name: name of the channel
 * 	frequency: frequency in GHz
 * 	inversion: one of INVERSION_OFF, INVERSION_ON, or INVERSION_AUTO.
 * 	modulation: one of 8VSB, 16VSB, QAM_64, or QAM_256.
 * 	video_pid: PID of the video stream.
 * 	audio_pid: PID of the audio stream.
 * 	channel_number: Transport stream channel number of the program.
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
#include <libdvbapi/sec.h>

struct dvbcfg_zapchannel
{
        char name[128];
        enum dvbfe_type fe_type;
        struct dvbfe_parameters fe_params;
	enum dvbfe_diseqc_switch sat_pos;
	enum dvbfe_diseqc_switch switch_option;
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
