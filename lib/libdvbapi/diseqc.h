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

#ifndef DISEQC_H
#define DISEQC_H

#include <stdlib.h>
#include <stdint.h>

/*		Addresses		*/
#define DISEQCxx_MASTER			0x00
#define SMATV_MASTER			0x10
#define LNB_MASTER			0x10
#define SWITCHER_MASTER			0x10
#define LNB				0x11
#define LNB_LOOP			0x12
#define SWITCHER_DC			0x14
#define SWITCHER_DC_LOOP		0x15
#define POL_CONTROLLER			0x21
#define POSITIONER_MASTER		0x30
#define POSITIONER_POLAR		0x31
#define POSITIONER_ELEVATION		0x32

/*		Commands		*/
#define DISEQC_SET_LOW_LO		0
#define DISEQC10_SET_VERT_POL		1
#define DISEQC10_SET_SAT_POS_A		2
#define DISEQC10_SET_SWITCH_OPT_A	3
#define DISEQC10_SET_HIGH_LO		4
#define DISEQC10_SET_HORIZ_POL		5
#define DISEQC10_SET_SAT_POS_B		6

#define DISEQC10_SET_SWITCH_OPT_B	7
#define DISEQC11_SET_SWITCH1_INP_A	8
#define DISEQC11_SET_SWITCH2_INP_A	9
#define DISEQC11_SET_SWITCH3_INP_A	10
#define DISEQC11_SET_SWITCH4_INP_A	11
#define DISEQC11_SET_SWITCH1_INP_B	12
#define DISEQC11_SET_SWITCH2_INP_B	13
#define DISEQC11_SET_SWITCH3_INP_B	14
#define DISEQC11_SET_SWITCH4_INP_B	15

#define DISEQCxx_SET_BUS_SLEEP		16
#define DISEQCxx_SET_BUS_AWAKE		17
#define DISEQC11_SET_CHAN_FREQ		18
#define DISEQC11_SET_RCVR_CHAN		19
#define DISEQC12_SET_MOTOR_HALT		20
#define DISEQC12_SET_LIMITS_OFF		21
#define DISEQC12_SET_LIMITS_ON		22
#define DISEQC22_GET_MOTOR_STATE	23
#define DISEQC12_SET_MOTOR_WEST		24
#define DISEQC12_SET_MOTOR_EAST		25
#define DISEQC12_SET_WEST_LIMIT		26
#define DISEQC12_SET_EAST_LIMIT		27
#define DISEQC12_STEP_MOTOR_UP		28  //???????
#define DISEQC12_STEP_MOTOR_DOWN	29

struct diseqc_cmd {
	uint8_t message[6];
	uint8_t length;
};

struct cmd_types {
	uint8_t command[7];
	char *cmd_descr;
};

typedef enum dvbfe_sec_voltage {
	DVBFE_SEC_VOLTAGE_13,
	DVBFE_SEC_VOLTAGE_18,
	DVBFE_SEC_VOLTAGE_OFF
} dvbfe_sec_voltage_t;

typedef enum dvbfe_sec_tone_mode {
	DVBFE_SEC_TONE_ON,
	DVBFE_SEC_TONE_OFF
} dvbfe_sec_tone_mode_t;

typedef enum dvbfe_sec_mini_cmd {
	DVBFE_SEC_MINI_A,
	DVBFE_SEC_MINI_B
} dvbfe_sec_mini_cmd_t;


struct cmd_types msgtbl[] = {

	{ { 0x03, 0xe0, 0x00, 0x20, 0x00, 0x00, 0x00 }, "DISEQC_SET_LOW_LO" },
	{ { 0x03, 0xe0, 0x00, 0x21, 0x00, 0x00, 0x00 }, "DISEQC10_SET_VERT_POL" },
	{ { 0x03, 0xe0, 0x00, 0x22, 0x00, 0x00, 0x00 }, "DISEQC10_SET_SAT_POS_A" },
	{ { 0x03, 0xe0, 0x00, 0x23, 0x00, 0x00, 0x00 }, "DISEQC10_SET_SWITCH_OPT_A" },
	{ { 0x03, 0xe0, 0x00, 0x24, 0x00, 0x00, 0x00 }, "DISEQC10_SET_HIGH_LO" },
	{ { 0x03, 0xe0, 0x00, 0x25, 0x00, 0x00, 0x00 }, "DISEQC10_SET_HORIZ_POL" },
	{ { 0x03, 0xe0, 0x00, 0x26, 0x00, 0x00, 0x00 }, "DISEQC10_SET_SAT_POS_B" },

	{ { 0x03, 0xe0, 0x00, 0x27, 0x00, 0x00, 0x00 }, "DISEQC10_SET_SWITCH_OPT_B" },
	{ { 0x03, 0xe0, 0x00, 0x28, 0x00, 0x00, 0x00 }, "DISEQC11_SET_SWITCH1_INP_A" },
	{ { 0x03, 0xe0, 0x00, 0x29, 0x00, 0x00, 0x00 }, "DISEQC11_SET_SWITCH2_INP_A" },
	{ { 0x03, 0xe0, 0x00, 0x2a, 0x00, 0x00, 0x00 }, "DISEQC11_SET_SWITCH3_INP_A" },
	{ { 0x03, 0xe0, 0x00, 0x2b, 0x00, 0x00, 0x00 }, "DISEQC11_SET_SWITCH4_INP_A" },
	{ { 0x03, 0xe0, 0x00, 0x2c, 0x00, 0x00, 0x00 }, "DISEQC11_SET_SWITCH1_INP_B" },
	{ { 0x03, 0xe0, 0x00, 0x2d, 0x00, 0x00, 0x00 }, "DISEQC11_SET_SWITCH2_INP_B" },
	{ { 0x03, 0xe0, 0x00, 0x2e, 0x00, 0x00, 0x00 }, "DISEQC11_SET_SWITCH3_INP_B" },
	{ { 0x03, 0xe0, 0x00, 0x2f, 0x00, 0x00, 0x00 }, "DISEQC11_SET_SWITCH4_INP_B" },

	{ { 0x03, 0xe0, 0x00, 0x30, 0x00, 0x00, 0x00 }, "DISEQCxx_SET_BUS_SLEEP" },
	{ { 0x03, 0xe0, 0x00, 0x31, 0x00, 0x00, 0x00 }, "DISEQCxx_SET_BUS_AWAKE" },
	{ { 0x06, 0x0e, 0x00, 0x05, 0x80, 0x00, 0x00 }, "DISEQC11_SET_CHAN_FREQ" },
	{ { 0x05, 0xe0, 0x00, 0x59, 0x00, 0x00, 0x00 }, "DISEQC11_SET_RCVR_CHAN" },

	{ { 0x03, 0xe0, 0x00, 0x60, 0x00, 0x00, 0x00 }, "DISEQC12_SET_MOTOR_HALT" },
	{ { 0x03, 0xe0, 0x00, 0x63, 0x00, 0x00, 0x00 }, "DISEQC12_SET_LIMITS_OFF" },
	{ { 0x04, 0xe0, 0x00, 0x6a, 0x00, 0x00, 0x00 }, "DISEQC12_SET_LIMITS_ON" },
	{ { 0x03, 0xe0, 0x00, 0x64, 0x00, 0x00, 0x00 }, "DISEQC22_GET_MOTOR_STATE" } ,
	{ { 0x04, 0xe1, 0x00, 0x69, 0x00, 0x00, 0x00 }, "DISEQC12_SET_MOTOR_WEST" },
	{ { 0x04, 0xe1, 0x00, 0x68, 0x00, 0x00, 0x00 }, "DISEQC12_SET_MOTOR_EAST" },
	{ { 0x03, 0xe1, 0x00, 0x67, 0x00, 0x00, 0x00 }, "DISEQC12_SET_WEST_LIMIT" },
	{ { 0x03, 0xe1, 0x00, 0x66, 0x00, 0x00, 0x00 }, "DISEQC12_SET_EAST_LIMIT" },
	{ { 0x04, 0xe2, 0x00, 0x68, 0x00, 0x00, 0x00 }, "DISEQC12_STEP_MOTOR_UP" },
	{ { 0x04, 0xe2, 0x00, 0x69, 0x00, 0x00, 0x00 }, "DISEQC12_STEP_MOTOR_DOWN"},

	{ {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, NULL }
};


/*
 *	Tone/Data Burst control
 *	@param fd, FD opened with libdvbfe_open().
 *	@param tone, SEC_TONE_ON/SEC_TONE_OFF
 */
extern int set_22k_tone(int fd, dvbfe_sec_tone_mode_t tone);

/*
 *	22khz Tone control
 *	@param fd, FD opened with libdvbfe_open().
 *	@param adapter, minicmd, SEC_MINI_A/SEC_MINI_B
 */
extern int set_tone_data_burst(int fd, dvbfe_sec_mini_cmd_t minicmd);

/*
 *	H/V polarization control
 *	@param fd, FD opened with libdvbfe_open().
 *	@param polarization, SEC_VOLTAGE_13/SEC_VOLTAGE_18/SEC_VOLTAGE_OFF
 */
extern int set_polarization(int fd, dvbfe_sec_voltage_t polarization);

/*
 *	Send a DiSEqC Command
 *	@param fd, FD opened with libdvbfe_open().
 *	@param cmd, the defined diseqc commands
 *	@param address, the address of the DiSEqC device to be controlled
 *	@param data, a pointer to am array containing the data to be sent
 *	max. length of data, that can be sent is 3 bytes
 */
extern int do_diseqc_command(int fd, uint8_t cmd, uint8_t address, uint8_t *data);

#endif
