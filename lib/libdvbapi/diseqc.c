/*
	libdvbfe - a DVB frontend library

	Copyright (C) 2005 Manu Abraham <manu@kromtek.com>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
*/

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "diseqc.h"
#include "dvbmisc.h"

int verbose = 1;

int set_tone_data_burst(int fd, fe_sec_tone_mode_t tone)
{
	int ret = 0;

	if (fd > 0) {
		switch (tone) {
			case SEC_TONE_OFF:
				ret = ioctl(fd, FE_SET_TONE, SEC_TONE_OFF);
				break;
			case SEC_TONE_ON:
				ret = ioctl(fd, FE_SET_TONE, SEC_TONE_OFF);
				break;
			default:
				print(verbose, ERROR, 1, "Invalid command !");
				break;
		}
		if (ret == -1)
			print(verbose, ERROR, 1, "IOCTL failed !");
	} else
		print(verbose, ERROR, 1, "Device open error !");

		return ret;
}

int set_22k_tone(int fd, fe_sec_mini_cmd_t minicmd)
{
	int ret = 0;

	if (fd > 0) {
		switch (minicmd) {
			case SEC_MINI_A:
				ret = ioctl(fd, FE_DISEQC_SEND_BURST, SEC_MINI_A);
				break;
			case SEC_MINI_B:
				ret = ioctl(fd, FE_DISEQC_SEND_BURST, SEC_MINI_B);
				break;
			default:
				print(verbose, ERROR, 1, "Invalid command");
				break;
		}
		if (ret == -1)
			print(verbose, ERROR, 1, "IOCTL failed");
	} else
		print(verbose, ERROR, 1, "Device open error !");

	return ret;
}

int set_polarization(int fd, fe_sec_voltage_t polarization)
{
	int ret = 0;

	if (fd > 0) {
		switch (polarization) {
			case SEC_VOLTAGE_13:
				ret = ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13);
				break;
			case SEC_VOLTAGE_18:
				ret = ioctl(fd, FE_SET_VOLTAGE, SEC_VOLTAGE_18);
				break;
			default:
				print(verbose, ERROR, 1, "Invalid command");
				break;
		}
		if (ret == -1)
			print(verbose, ERROR, 1, "IOCTL failed");
	} else
		print(verbose, ERROR, 1, "Device open error !");

	return ret;
}

int do_diseqc_cmd(int fd, uint8_t cmd, uint8_t  address, uint8_t *data)
{
	int ret = 0;
	uint8_t length;
	struct diseqc_cmd diseqc_message;

	memcpy(&diseqc_message.message[0], &msgtbl->command[1], 6);
	length = msgtbl->command[0];
	diseqc_message.length = length;

	/*	Set Address	*/
	diseqc_message.message[2] = address;
	switch (length) {
		case 6:
			/*	Set Data		*/
			diseqc_message.message[6] = data[2];
		case 5:
			/*	Set Data		*/
			diseqc_message.message[5] = data[1];
		case 4:
			/*	Set Data		*/
			diseqc_message.message[4] = data[0];
		case 3:
			/*	Only cmd	*/
			break;
		default:
			return -EINVAL;
	}

	if (fd > 0) {
		ret = ioctl(fd, diseqc_message);
		if (ret == -1)
			print(verbose, ERROR, 1, "IOCTL failed");

	} else
		print(verbose, ERROR, 1, "Device open error !");

	return ret;
}
