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
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>
#include <errno.h>
#include <linux/dvb/frontend.h>
#include "dvbfe.h"

int dvbfe_open(int adapter, int frontend, int readonly)
{
	char filename[PATH_MAX+1];

	sprintf(filename, "/dev/dvb/adapter%i/frontend%i", adapter, frontend);

	if (readonly) {
		return open(filename, O_RDONLY);
	} else {
		return open(filename, O_RDWR);
	}
}

int dvbfe_get_type(int fd)
{
	struct dvb_frontend_info info;
	int res;

	if ((res = ioctl(fd, FE_GET_INFO, &info)) != 0)
		return res;

	switch(info.type) {
	case FE_QPSK:
		return DVBFE_TYPE_DVBS;

	case FE_QAM:
		return DVBFE_TYPE_DVBC;

	case FE_OFDM:
		return DVBFE_TYPE_DVBT;

	case FE_ATSC:
		return DVBFE_TYPE_ATSC;

	}
	return -EINVAL;
}

int dvbfe_get_status(int fd, int statusmask, struct dvbfe_status *result)
{
	int returnval = 0;
	fe_status_t status;

	memset(result, 0, sizeof(result));
	if (statusmask & DVBFE_STATUS_FE) {
		if (!ioctl(fd, FE_READ_STATUS, &status)) {
			returnval |= DVBFE_STATUS_FE;
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
	if (statusmask & DVBFE_STATUS_BER) {
		if (!ioctl(fd, FE_READ_BER, &result->ber))
			returnval |= DVBFE_STATUS_BER;
	}
	if (statusmask & DVBFE_STATUS_SIGNAL_STRENGTH) {
		if (!ioctl(fd, FE_READ_SIGNAL_STRENGTH, &result->signal_strength))
			returnval |= DVBFE_STATUS_SIGNAL_STRENGTH;
	}
	if (statusmask & DVBFE_STATUS_SNR) {
		if (!ioctl(fd, FE_READ_SNR, &result->snr))
			returnval |= DVBFE_STATUS_SNR;
	}
	if (statusmask & DVBFE_STATUS_UNCORRECTED_BLOCKS) {
		if (!ioctl(fd, FE_READ_UNCORRECTED_BLOCKS, &result->ucblocks))
			returnval |= DVBFE_STATUS_UNCORRECTED_BLOCKS;
	}

	return returnval;
}

int dvbfe_set_frontend(int fd, struct dvbfe_parameters *params)
{
	struct dvb_frontend_parameters kparams;
	struct dvb_frontend_info info;
	int res;

	if ((res = ioctl(fd, FE_GET_INFO, &info)) != 0)
		return res;

	// FIXME: these should all be done with switches and not directly copied
	kparams.frequency = params->frequency;
	kparams.inversion = params->inversion;
	switch(info.type) {
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

	return ioctl(fd, FE_SET_FRONTEND, &kparams);
}

int dvbfe_get_frontend(int fd, struct dvbfe_parameters *params)
{
	struct dvb_frontend_parameters kparams;
	struct dvb_frontend_info info;
	int res;

	if ((res = ioctl(fd, FE_GET_INFO, &info)) != 0)
		return res;
	if ((res = ioctl(fd, FE_GET_FRONTEND, &kparams)) != 0)
		return res;

	// FIXME: these should all be done with switches and not directly copied
	params->frequency = kparams.frequency;
	params->inversion = kparams.inversion;
	switch(info.type) {
	case FE_QPSK:
		params->u.dvbs.symbol_rate = kparams.u.qpsk.symbol_rate;
		params->u.dvbs.fec_inner = kparams.u.qpsk.fec_inner;
		break;

	case FE_QAM:
		params->u.dvbc.symbol_rate = kparams.u.qam.symbol_rate;
		params->u.dvbc.fec_inner = kparams.u.qam.fec_inner;
		params->u.dvbc.modulation = kparams.u.qam.modulation;
		break;

	case FE_OFDM:
		params->u.dvbt.bandwidth = kparams.u.ofdm.bandwidth;
		params->u.dvbt.code_rate_HP = kparams.u.ofdm.code_rate_HP;
		params->u.dvbt.code_rate_LP= kparams.u.ofdm.code_rate_LP;
		params->u.dvbt.constellation= kparams.u.ofdm.constellation;
		params->u.dvbt.transmission_mode= kparams.u.ofdm.transmission_mode;
		params->u.dvbt.guard_interval= kparams.u.ofdm.guard_interval;
		params->u.dvbt.hierarchy_information= kparams.u.ofdm.hierarchy_information;
		break;

	case FE_ATSC:
		params->u.atsc.modulation = kparams.u.vsb.modulation;
		break;

	default:
		return -EINVAL;
	}

	return 0;
}

int dvbfe_diseqc_command(int fd, char *command)
{
	int i = 0;
	int waittime;
	int status;
	struct dvb_diseqc_master_cmd master_cmd;
	unsigned int tmpcmd[6];

	while(command[i]) {
		/* kill whitespace */
		if (isspace(command[i])) {
			i++;
			continue;
		}

		switch(command[i]) {
			case 't':
				if ((status = ioctl(fd, FE_SET_TONE, SEC_TONE_OFF)) != 0)
					return status;
				break;

			case 'T':
				if ((status = ioctl(fd, FE_SET_TONE, SEC_TONE_ON)) != 0)
					return status;
				break;

			case '_':
				if ((status = ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_OFF)) != 0)
					return status;
				break;

			case 'v':
				if ((status = ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13)) != 0)
					return status;
				break;

			case 'V':
				if ((status = ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_18)) != 0)
					return status;
				break;

			case 'A':
				if ((status = ioctl(fd, FE_DISEQC_SEND_BURST, SEC_MINI_A)) != 0)
					return status;
				break;

			case 'B':
				if ((status = ioctl(fd, FE_DISEQC_SEND_BURST, SEC_MINI_B)) != 0)
					return status;
				break;

			case '+':
				ioctl(fd, FE_ENABLE_HIGH_LNB_VOLTAGE, 1);
				/* don't care if this one is not supported */
				break;

			case '-':
				ioctl(fd, FE_ENABLE_HIGH_LNB_VOLTAGE, 0);
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

				if ((status = ioctl(fd, FE_DISEQC_SEND_MASTER_CMD, &master_cmd)) != 0)
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

int dvbfe_diseqc_read(int fd, int timeout, unsigned char *buf, unsigned int len)
{
	struct dvb_diseqc_slave_reply reply;
	int result;

	if (len > 4)
		len = 4;

	reply.timeout = timeout;
	reply.msg_len = len;

	if ((result = ioctl(fd, FE_DISEQC_RECV_SLAVE_REPLY, reply)) != 0)
		return result;

	if (reply.msg_len < len)
		len = reply.msg_len;
	memcpy(buf, reply.msg, len);

	return len;
}
