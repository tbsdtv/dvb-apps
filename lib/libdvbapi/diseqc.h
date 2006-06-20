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

#ifndef DVBFE_DISEQC_H
#define DVBFE_DISEQC_H 1

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

enum dvbfe_diseqc_direction {
	DISEQC_DIRECTION_EAST,
	DISEQC_DIRECTION_WEST,
};

extern int dvbfe_diseqc_set_reset(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address, int state);
extern int dvbfe_diseqc_set_power(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address, int state);
extern int dvbfe_diseqc_set_bus_state(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address, int state);

extern int dvbfe_diseqc_set_committed_switches(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address,
					       int lo_hi, int polarisation, int sat_pos, int switch_option);
extern int dvbfe_diseqc_set_uncommitted_switches(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address,
						 int sw1, int sw2, int sw3, int sw4);

extern int dvbfe_diseqc_set_frequency(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address, uint32_t frequency);
extern int dvbfe_diseqc_set_channel(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address, uint32_t frequency);

extern int dvbfe_diseqc_halt_positioner(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address);
extern int dvbfe_diseqc_disable_limits(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address);
extern int dvbfe_diseqc_set_limit(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address, enum dvbfe_diseqc_direction direction);
extern int dvbfe_diseqc_drive_motor(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address, enum dvbfe_diseqc_direction direction, int timeout);
extern int dvbfe_diseqc_store_preset(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address, int preset);

extern int dvbfe_diseqc_goto_preset(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address, int preset);
extern int dvbfe_diseqc_goto_angle(struct dvbfe_handle *fe, enum dvbfe_diseqc_address address, int angle);



struct diseqc_cmd {
	uint8_t message[6];
	uint8_t length;
};

struct cmd_types {
	uint8_t command[7];
	char *cmd_descr;
};

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


#endif
