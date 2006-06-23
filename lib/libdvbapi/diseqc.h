/*
	libdvbfe - a DVB frontend library

	Copyright (C) 2005 Manu Abraham <manu@kromtek.com>
	Copyright (C) 2006 Andrew de Quincey <adq_dvb@lidskialf.net>

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

enum dvbfe_diseqc_framing {
	DISEQC_FRAMING_MASTER_NOREPLY		= 0xE0,
	DISEQC_FRAMING_MASTER_NOREPLY_REPEAT	= 0xE1,
	DISEQC_FRAMING_MASTER_REPLY		= 0xE2,
	DISEQC_FRAMING_MASTER_REPLY_REPEAT	= 0xE3,
	DISEQC_FRAMING_SLAVE_OK			= 0xE4,
	DISEQC_FRAMING_SLAVE_UNSUPPORTED	= 0xE5,
	DISEQC_FRAMING_SLAVE_PARITY_ERROR	= 0xE6,
	DISEQC_FRAMING_SLAVE_UNRECOGNISED 	= 0xE7,
};

enum dvbfe_diseqc_address {
	DISEQC_ADDRESS_MASTER			= 0x00,
	DISEQC_ADDRESS_SMATV_MASTER		= 0x10,
	DISEQC_ADDRESS_LNB_MASTER		= 0x10,
	DISEQC_ADDRESS_SWITCHER_MASTER		= 0x10,
	DISEQC_ADDRESS_LNB			= 0x11,
	DISEQC_ADDRESS_LNB_LOOP			= 0x12,
	DISEQC_ADDRESS_SWITCHER_DC		= 0x14,
	DISEQC_ADDRESS_SWITCHER_DC_LOOP		= 0x15,
	DISEQC_ADDRESS_POL_CONTROLLER		= 0x21,
	DISEQC_ADDRESS_POSITIONER_MASTER	= 0x30,
	DISEQC_ADDRESS_POSITIONER_POLAR		= 0x31,
	DISEQC_ADDRESS_POSITIONER_ELEVATION	= 0x32,
};

enum dvbfe_diseqc_reset {
	DISEQC_RESET,
	DISEQC_RESET_CLEAR,
};

enum dvbfe_diseqc_power {
	DISEQC_POWER_OFF,
	DISEQC_POWER_ON,
};

enum dvbfe_diseqc_listen {
	DISEQC_LISTEN_SLEEP,
	DISEQC_LISTEN_AWAKE,
};

enum dvbfe_diseqc_oscillator {
	DISEQC_OSCILLATOR_LOW,
	DISEQC_OSCILLATOR_HIGH,
	DISEQC_OSCILLATOR_UNCHANGED,
};

enum dvbfe_diseqc_polarisation {
	DISEQC_POLARISATION_H,
	DISEQC_POLARISATION_V,
	DISEQC_POLARISATION_L,
	DISEQC_POLARISATION_R,
	DISEQC_POLARISATION_UNCHANGED,
};

enum dvbfe_diseqc_switch {
	DISEQC_SWITCH_A,
	DISEQC_SWITCH_B,
	DISEQC_SWITCH_UNCHANGED,
};

enum dvbfe_diseqc_analog_id {
	DISEQC_ANALOG_ID_A0,
	DISEQC_ANALOG_ID_A1,
};

enum dvbfe_diseqc_drive_mode {
	DISEQC_DRIVE_MODE_STEPS,
	DISEQC_DRIVE_MODE_TIMEOUT,
};

enum dvbfe_diseqc_direction {
	DISEQC_DIRECTION_EAST,
	DISEQC_DIRECTION_WEST,
};

/**
 * Control the reset status of an attached DISEQC device.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @param state The state to set.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_set_reset(struct dvbfe_handle *fe,
				  enum dvbfe_diseqc_address address,
				  enum dvbfe_diseqc_reset state);

/**
 * Control the power status of an attached DISEQC peripheral.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @param state The state to set.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_set_power(struct dvbfe_handle *fe,
				  enum dvbfe_diseqc_address address,
				  enum dvbfe_diseqc_power state);

/**
 * Control the listening status of an attached DISEQC peripheral.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @param state The state to set.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_set_listen(struct dvbfe_handle *fe,
				   enum dvbfe_diseqc_address address,
				   enum dvbfe_diseqc_listen state);

/**
 * Set the state of the committed switches of a DISEQC device.
 * These are switches which are defined to have a standard name.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @param oscillator Value to set the lo/hi switch to.
 * @param polarisation Value to set the polarisation switch to.
 * @param sat_pos Value to set the satellite position switch to.
 * @param switch_option Value to set the switch option switch to.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_set_committed_switches(struct dvbfe_handle *fe,
					       enum dvbfe_diseqc_address address,
					       enum dvbfe_diseqc_oscillator oscillator,
					       enum dvbfe_diseqc_polarisation polarisation,
					       enum dvbfe_diseqc_switch sat_pos,
					       enum dvbfe_diseqc_switch switch_option);

/**
 * Set the state of the uncommitted switches of a DISEQC device.
 * These provide another four switching possibilities.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @param s1 Value to set the S1 switch to.
 * @param s2 Value to set the S2 switch to.
 * @param s3 Value to set the S3 switch to.
 * @param s3 Value to set the S4 switch to.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_set_uncommitted_switches(struct dvbfe_handle *fe,
						 enum dvbfe_diseqc_address address,
						 enum dvbfe_diseqc_switch s1,
						 enum dvbfe_diseqc_switch s2,
						 enum dvbfe_diseqc_switch s3,
						 enum dvbfe_diseqc_switch s4);

/**
 * Set an analogue value.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @param id The id of the analogue value to set.
 * @param value The value to set.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_set_analog_value(struct dvbfe_handle *fe,
					 enum dvbfe_diseqc_address address,
					 enum dvbfe_diseqc_analog_id id,
					 uint8_t value);

/**
 * Set the desired frequency.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @param frequency The frequency to set in GHz.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_set_frequency(struct dvbfe_handle *fe,
				      enum dvbfe_diseqc_address address,
				      uint32_t frequency);

/**
 * Set the desired channel.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @param channel ID of the channel to set.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_set_channel(struct dvbfe_handle *fe,
				    enum dvbfe_diseqc_address address,
				    uint16_t channel);

/**
 * Halt the positioner.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_halt_positioner(struct dvbfe_handle *fe,
					enum dvbfe_diseqc_address address);

/**
 * Disable positioner limits.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_disable_limits(struct dvbfe_handle *fe,
				       enum dvbfe_diseqc_address address);

/**
 * Set positioner limits.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_set_limit(struct dvbfe_handle *fe,
				  enum dvbfe_diseqc_address address,
				  enum dvbfe_diseqc_direction direction);

/**
 * Drive positioner motor.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @param direction Direction to drive in.
 * @param mode Drive mode to use
 * 	       (TIMEOUT=>value is a timeout in seconds, or STEPS=>value is a count of steps to use)
 * @param value Value associated with the drive mode (range 0->127)
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_drive_motor(struct dvbfe_handle *fe,
				    enum dvbfe_diseqc_address address,
				    enum dvbfe_diseqc_direction direction,
				    enum dvbfe_diseqc_drive_mode mode,
				    uint8_t value);

/**
 * Store positioner preset id at current position.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @param id ID of the preset.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_store_preset(struct dvbfe_handle *fe,
				     enum dvbfe_diseqc_address address,
				     uint8_t id);

/**
 * Goto a pre-set position.
 *
 * @param fe Frontend concerned.
 * @param address Address of the device.
 * @param id ID of the preset.
 * @return 0 on success, or nonzero on error.
 */
extern int dvbfe_diseqc_goto_preset(struct dvbfe_handle *fe,
				    enum dvbfe_diseqc_address address,
				    uint8_t id);

extern int dvbfe_diseqc_goto_angle(struct dvbfe_handle *fe,
				   enum dvbfe_diseqc_address address,
				   int angle);

#endif
