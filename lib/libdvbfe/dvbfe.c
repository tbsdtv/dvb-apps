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

int dvbfe_get_info(int fd, struct dvb_frontend_info *info)
{
	return ioctl(fd, FE_GET_INFO, info);
}

int dvbfe_get_status(int fd, int statusmask, struct dvbfe_status *result)
{
	int returnval = 0;

	if (statusmask & DVBFE_STATUS_FE) {
		if (!ioctl(fd, FE_READ_STATUS, &result->status))
			returnval |= DVBFE_STATUS_FE;
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

int dvbfe_set_frontend(int fd, struct dvb_frontend_parameters *params)
{
	return ioctl(fd, FE_SET_FRONTEND, params);
}

int dvbfe_get_frontend(int fd, struct dvb_frontend_parameters *params)
{
	return ioctl(fd, FE_GET_FRONTEND, params);
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
