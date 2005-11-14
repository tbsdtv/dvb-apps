/*
 * libdvbfe - a DVB frontend library
 *
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <linux/dvb/frontend.h>
#include "dvbfe.h"

#define GET_INFO_MIN_DELAY_US 100000

struct dvbfe_handle_prv {
	int fd;
	dvbfe_type_t type;
	struct timeval nextinfotime;
	struct dvbfe_info cachedinfo;
	int cachedreturnval;
};

dvbfe_handle_t dvbfe_open(int adapter, int frontend, int readonly)
{
	char filename[PATH_MAX+1];
	struct dvbfe_handle_prv *fehandle;
	int fd;
	struct dvb_frontend_info info;

	// open it
	sprintf(filename, "/dev/dvb/adapter%i/frontend%i", adapter, frontend);
	if (readonly) {
		fd = open(filename, O_RDONLY);
	} else {
		fd = open(filename, O_RDWR);
	}
	if (fd < 0)
		return NULL;

	// determine fe type
	if (ioctl(fd, FE_GET_INFO, &info)) {
		close(fd);
		return NULL;
	}

	// setup structure
	fehandle = (struct dvbfe_handle_prv*) malloc(sizeof(struct dvbfe_handle_prv));
	memset(fehandle, 0, sizeof(struct dvbfe_handle_prv));
	fehandle->fd = fd;
	switch(info.type) {
	case FE_QPSK:
		fehandle->type = DVBFE_TYPE_DVBS;

	case FE_QAM:
		fehandle->type = DVBFE_TYPE_DVBC;

	case FE_OFDM:
		fehandle->type = DVBFE_TYPE_DVBT;

	case FE_ATSC:
		fehandle->type = DVBFE_TYPE_ATSC;
	}

	// done
	return fehandle;
}

void dvbfe_close(dvbfe_handle_t _fehandle)
{
	struct dvbfe_handle_prv *fehandle = (struct dvbfe_handle_prv*) _fehandle;

	close(fehandle->fd);
	free(fehandle);
}

int dvbfe_get_info(dvbfe_handle_t _fehandle, dvbfe_info_mask_t querymask, struct dvbfe_info *result)
{
	int returnval = 0;
	fe_status_t status;
	struct dvb_frontend_parameters kparams;
	struct dvbfe_handle_prv *fehandle = (struct dvbfe_handle_prv*) _fehandle;
	struct timeval curtime;

	// limit how often this is called to reduce bus traffic
	gettimeofday(&curtime, NULL);
	if ((curtime.tv_sec < fehandle->nextinfotime.tv_sec) ||
	    ((curtime.tv_sec == fehandle->nextinfotime.tv_sec) && (curtime.tv_usec < fehandle->nextinfotime.tv_usec))) {
		memcpy(result, &fehandle->cachedinfo, sizeof(struct dvbfe_info));
		return fehandle->cachedreturnval;
	}

	// retrieve the requested values
	memset(result, 0, sizeof(result));
	result->type = fehandle->type;
	if (querymask & DVBFE_INFO_LOCKSTATUS) {
		if (!ioctl(fehandle->fd, FE_READ_STATUS, &status)) {
			returnval |= DVBFE_INFO_LOCKSTATUS;
			if (status & FE_HAS_SIGNAL)
				result->signal = 1;

			if (status & FE_HAS_CARRIER)
				result->carrier = 1;

			if (status & FE_HAS_VITERBI)
				result->viterbi = 1;

			if (status & FE_HAS_SYNC)
				result->sync = 1;

			if (status & FE_HAS_LOCK)
				result->lock = 1;
		}
	}
	if (querymask & DVBFE_INFO_FEPARAMS) {
		if (!ioctl(fehandle->fd, FE_GET_FRONTEND, &kparams)) {
			returnval |= DVBFE_INFO_FEPARAMS;
			result->feparams.frequency = kparams.frequency;
			result->feparams.inversion = kparams.inversion;
			switch(fehandle->type) {
			case FE_QPSK:
				result->feparams.u.dvbs.symbol_rate = kparams.u.qpsk.symbol_rate;
				result->feparams.u.dvbs.fec_inner = kparams.u.qpsk.fec_inner;
				break;

			case FE_QAM:
				result->feparams.u.dvbc.symbol_rate = kparams.u.qam.symbol_rate;
				result->feparams.u.dvbc.fec_inner = kparams.u.qam.fec_inner;
				result->feparams.u.dvbc.modulation = kparams.u.qam.modulation;
				break;

			case FE_OFDM:
				result->feparams.u.dvbt.bandwidth = kparams.u.ofdm.bandwidth;
				result->feparams.u.dvbt.code_rate_HP = kparams.u.ofdm.code_rate_HP;
				result->feparams.u.dvbt.code_rate_LP= kparams.u.ofdm.code_rate_LP;
				result->feparams.u.dvbt.constellation= kparams.u.ofdm.constellation;
				result->feparams.u.dvbt.transmission_mode= kparams.u.ofdm.transmission_mode;
				result->feparams.u.dvbt.guard_interval= kparams.u.ofdm.guard_interval;
				result->feparams.u.dvbt.hierarchy_information= kparams.u.ofdm.hierarchy_information;
				break;

			case FE_ATSC:
				result->feparams.u.atsc.modulation = kparams.u.vsb.modulation;
				break;
			}
		}

	}
	if (querymask & DVBFE_INFO_BER) {
		if (!ioctl(fehandle->fd, FE_READ_BER, &result->ber))
			returnval |= DVBFE_INFO_BER;
	}
	if (querymask & DVBFE_INFO_SIGNAL_STRENGTH) {
		if (!ioctl(fehandle->fd, FE_READ_SIGNAL_STRENGTH, &result->signal_strength))
			returnval |= DVBFE_INFO_SIGNAL_STRENGTH;
	}
	if (querymask & DVBFE_INFO_SNR) {
		if (!ioctl(fehandle->fd, FE_READ_SNR, &result->snr))
			returnval |= DVBFE_INFO_SNR;
	}
	if (querymask & DVBFE_INFO_UNCORRECTED_BLOCKS) {
		if (!ioctl(fehandle->fd, FE_READ_UNCORRECTED_BLOCKS, &result->ucblocks))
			returnval |= DVBFE_INFO_UNCORRECTED_BLOCKS;
	}

	// setup for next poll
	gettimeofday(&fehandle->nextinfotime, NULL);
	fehandle->nextinfotime.tv_usec += GET_INFO_MIN_DELAY_US;
	if (fehandle->nextinfotime.tv_usec >= 1000000) {
		fehandle->nextinfotime.tv_usec -= 1000000;
		fehandle->nextinfotime.tv_sec++;
	}
	memcpy(&fehandle->cachedinfo, result, sizeof(struct dvbfe_info));
	fehandle->cachedreturnval = returnval;

	// done
	return returnval;
}

int dvbfe_set(dvbfe_handle_t _fehandle, struct dvbfe_parameters *params)
{
	struct dvb_frontend_parameters kparams;
	struct dvbfe_handle_prv *fehandle = (struct dvbfe_handle_prv*) _fehandle;

	// FIXME: these should all be done with switches and not directly copied
	kparams.frequency = params->frequency;
	kparams.inversion = params->inversion;
	switch(fehandle->type) {
	case FE_QPSK:
		kparams.u.qpsk.symbol_rate = params->u.dvbs.symbol_rate;
		kparams.u.qpsk.fec_inner = params->u.dvbs.fec_inner;
		break;

	case FE_QAM:
		kparams.u.qam.symbol_rate = params->u.dvbc.symbol_rate;
		kparams.u.qam.fec_inner = params->u.dvbc.fec_inner;
		kparams.u.qam.modulation = params->u.dvbc.modulation;
		break;

	case FE_OFDM:
                kparams.u.ofdm.bandwidth = params->u.dvbt.bandwidth;
                kparams.u.ofdm.code_rate_HP = params->u.dvbt.code_rate_HP;
                kparams.u.ofdm.code_rate_LP= params->u.dvbt.code_rate_LP;
                kparams.u.ofdm.constellation= params->u.dvbt.constellation;
                kparams.u.ofdm.transmission_mode= params->u.dvbt.transmission_mode;
                kparams.u.ofdm.guard_interval= params->u.dvbt.guard_interval;
                kparams.u.ofdm.hierarchy_information= params->u.dvbt.hierarchy_information;
                break;

	case FE_ATSC:
		kparams.u.vsb.modulation = params->u.atsc.modulation;
		break;

	default:
		return -EINVAL;
	}

	return ioctl(fehandle->fd, FE_SET_FRONTEND, &kparams);
}

void dvbfe_poll(dvbfe_handle_t fehandle)
{
	// no implementation required yet
}







int dvbfe_diseqc_command(dvbfe_handle_t _fehandle, char *command)
{
	int i = 0;
	int waittime;
	int status;
	struct dvb_diseqc_master_cmd master_cmd;
	unsigned int tmpcmd[6];
	struct dvbfe_handle_prv *fehandle = (struct dvbfe_handle_prv*) _fehandle;

	while(command[i]) {
		/* kill whitespace */
		if (isspace(command[i])) {
			i++;
			continue;
		}

		switch(command[i]) {
			case 't':
				if ((status = ioctl(fehandle->fd, FE_SET_TONE, SEC_TONE_OFF)) != 0)
					return status;
				break;

			case 'T':
				if ((status = ioctl(fehandle->fd, FE_SET_TONE, SEC_TONE_ON)) != 0)
					return status;
				break;

			case '_':
				if ((status = ioctl(fehandle->fd, FE_SET_VOLTAGE, SEC_VOLTAGE_OFF)) != 0)
					return status;
				break;

			case 'v':
				if ((status = ioctl(fehandle->fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13)) != 0)
					return status;
				break;

			case 'V':
				if ((status = ioctl(fehandle->fd, FE_SET_VOLTAGE, SEC_VOLTAGE_18)) != 0)
					return status;
				break;

			case 'A':
				if ((status = ioctl(fehandle->fd, FE_DISEQC_SEND_BURST, SEC_MINI_A)) != 0)
					return status;
				break;

			case 'B':
				if ((status = ioctl(fehandle->fd, FE_DISEQC_SEND_BURST, SEC_MINI_B)) != 0)
					return status;
				break;

			case '+':
				ioctl(fehandle->fd, FE_ENABLE_HIGH_LNB_VOLTAGE, 1);
				/* don't care if this one is not supported */
				break;

			case '-':
				ioctl(fehandle->fd, FE_ENABLE_HIGH_LNB_VOLTAGE, 0);
				/* don't care if this one is not supported */
				break;

			case 'W':
				waittime = atoi(command + i + 1);
				if (waittime == 0) {
					return -EINVAL;
				}
				usleep(waittime * 1000);
				while(command[i] && !isspace(command[i]))
					i++;
				break;

			case '[':
				master_cmd.msg_len = sscanf(command+i+1, "%x %x %x %x %x %x",
						tmpcmd, tmpcmd+1, tmpcmd+2, tmpcmd+3, tmpcmd+4, tmpcmd+5);
				if (master_cmd.msg_len == 0)
					return -EINVAL;
				master_cmd.msg[0] = tmpcmd[0];
				master_cmd.msg[1] = tmpcmd[1];
				master_cmd.msg[2] = tmpcmd[2];
				master_cmd.msg[3] = tmpcmd[3];
				master_cmd.msg[4] = tmpcmd[4];
				master_cmd.msg[5] = tmpcmd[5];

				if ((status = ioctl(fehandle->fd, FE_DISEQC_SEND_MASTER_CMD, &master_cmd)) != 0)
					return status;

				while(command[i] && (command[i] != ']'))
					i++;
				break;

			default:
				return -EINVAL;
		}

		i++;
	}

	return 0;
}

int dvbfe_diseqc_read(dvbfe_handle_t _fehandle, int timeout, unsigned char *buf, unsigned int len)
{
	struct dvb_diseqc_slave_reply reply;
	int result;
	struct dvbfe_handle_prv *fehandle = (struct dvbfe_handle_prv*) _fehandle;

	if (len > 4)
		len = 4;

	reply.timeout = timeout;
	reply.msg_len = len;

	if ((result = ioctl(fehandle->fd, FE_DISEQC_RECV_SLAVE_REPLY, reply)) != 0)
		return result;

	if (reply.msg_len < len)
		len = reply.msg_len;
	memcpy(buf, reply.msg, len);

	return len;
}
