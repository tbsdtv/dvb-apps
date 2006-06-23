#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "dvbfe.h"
#include "sec.h"

int dvbfe_sec_std_sequence(struct dvbfe_handle *fe,
			   enum dvbfe_diseqc_oscillator oscillator,
			   enum dvbfe_diseqc_polarisation polarisation,
			   enum dvbfe_diseqc_switch sat_pos)
{
	dvbfe_set_22k_tone(fe, DVBFE_SEC_TONE_OFF);

	switch(polarisation) {
	case DISEQC_POLARISATION_V:
	case DISEQC_POLARISATION_R:
		dvbfe_set_voltage(fe, DVBFE_SEC_VOLTAGE_13);
		break;
	case DISEQC_POLARISATION_H:
	case DISEQC_POLARISATION_L:
		dvbfe_set_voltage(fe, DVBFE_SEC_VOLTAGE_18);
		break;
	default:
		return -EINVAL;
	}

	usleep(15000);

	dvbfe_diseqc_set_committed_switches(fe,
					    DISEQC_ADDRESS_MASTER,
					    oscillator,
					    polarisation,
					    sat_pos,
					    DISEQC_SWITCH_UNCHANGED);

	usleep(15000);

	switch(sat_pos) {
	case DISEQC_SWITCH_A:
		dvbfe_set_tone_data_burst(fe, DVBFE_SEC_MINI_A);
		break;
	case DISEQC_SWITCH_B:
		dvbfe_set_tone_data_burst(fe, DVBFE_SEC_MINI_B);
		break;
	default:
		return -EINVAL;
	}

	usleep(15000);

	switch(oscillator) {
	case DISEQC_OSCILLATOR_LOW:
		dvbfe_set_22k_tone(fe, DVBFE_SEC_TONE_OFF);
		break;
	case DISEQC_OSCILLATOR_HIGH:
		dvbfe_set_22k_tone(fe, DVBFE_SEC_TONE_ON);
		break;
	default:
		return -EINVAL;
	}

	return 0;
}

int dvbfe_diseqc_set_reset(struct dvbfe_handle *fe,
			   enum dvbfe_diseqc_address address,
			   enum dvbfe_diseqc_reset state)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x00 };

	if (state == DISEQC_RESET_CLEAR)
		data[2] = 0x01;

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_set_power(struct dvbfe_handle *fe,
			   enum dvbfe_diseqc_address address,
			   enum dvbfe_diseqc_power state)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x02 };

	if (state == DISEQC_POWER_ON)
		data[2] = 0x03;

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_set_listen(struct dvbfe_handle *fe,
			    enum dvbfe_diseqc_address address,
			    enum dvbfe_diseqc_listen state)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x30 };

	if (state == DISEQC_LISTEN_AWAKE)
		data[2] = 0x31;

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_set_committed_switches(struct dvbfe_handle *fe,
					enum dvbfe_diseqc_address address,
					enum dvbfe_diseqc_oscillator oscillator,
					enum dvbfe_diseqc_polarisation polarisation,
					enum dvbfe_diseqc_switch sat_pos,
					enum dvbfe_diseqc_switch switch_option)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x38, 0x00 };

	switch(oscillator) {
	case DISEQC_OSCILLATOR_LOW:
		data[3] |= 0x10;
		break;
	case DISEQC_OSCILLATOR_HIGH:
		data[3] |= 0x01;
		break;
	case DISEQC_OSCILLATOR_UNCHANGED:
		break;
	}
	switch(polarisation) {
	case DISEQC_POLARISATION_V:
	case DISEQC_POLARISATION_R:
		data[3] |= 0x20;
		break;
	case DISEQC_POLARISATION_H:
	case DISEQC_POLARISATION_L:
		data[3] |= 0x02;
		break;
	case DISEQC_POLARISATION_UNCHANGED:
		break;
	}
	switch(sat_pos) {
	case DISEQC_SWITCH_A:
		data[3] |= 0x40;
		break;
	case DISEQC_SWITCH_B:
		data[3] |= 0x04;
		break;
	case DISEQC_SWITCH_UNCHANGED:
		break;
	}
	switch(switch_option) {
	case DISEQC_SWITCH_A:
		data[3] |= 0x80;
		break;
	case DISEQC_SWITCH_B:
		data[3] |= 0x08;
		break;
	case DISEQC_SWITCH_UNCHANGED:
		break;
	}

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_set_uncommitted_switches(struct dvbfe_handle *fe,
					  enum dvbfe_diseqc_address address,
					  enum dvbfe_diseqc_switch s1,
					  enum dvbfe_diseqc_switch s2,
					  enum dvbfe_diseqc_switch s3,
					  enum dvbfe_diseqc_switch s4)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x39, 0x00 };

	switch(s1) {
	case DISEQC_SWITCH_A:
		data[3] |= 0x10;
		break;
	case DISEQC_SWITCH_B:
		data[3] |= 0x01;
		break;
	case DISEQC_SWITCH_UNCHANGED:
		break;
	}
	switch(s2) {
	case DISEQC_SWITCH_A:
		data[3] |= 0x20;
		break;
	case DISEQC_SWITCH_B:
		data[3] |= 0x02;
		break;
	case DISEQC_SWITCH_UNCHANGED:
		break;
	}
	switch(s3) {
	case DISEQC_SWITCH_A:
		data[3] |= 0x40;
		break;
	case DISEQC_SWITCH_B:
		data[3] |= 0x04;
		break;
	case DISEQC_SWITCH_UNCHANGED:
		break;
	}
	switch(s4) {
	case DISEQC_SWITCH_A:
		data[3] |= 0x80;
		break;
	case DISEQC_SWITCH_B:
		data[3] |= 0x08;
		break;
	case DISEQC_SWITCH_UNCHANGED:
		break;
	}

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_set_analog_value(struct dvbfe_handle *fe,
				  enum dvbfe_diseqc_address address,
				  enum dvbfe_diseqc_analog_id id,
				  uint8_t value)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x48, value };

	if (id == DISEQC_ANALOG_ID_A1)
		data[2] = 0x49;

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_set_frequency(struct dvbfe_handle *fe,
			       enum dvbfe_diseqc_address address,
			       uint32_t frequency)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x58, 0x00, 0x00, 0x00 };
	int len = 5;

	uint32_t bcdval = 0;
	int i;
	for(i=0; i<=24;i+=4) {
		bcdval |= ((frequency % 10) << i);
		frequency /= 10;
	}

	data[3] = bcdval >> 16;
	data[4] = bcdval >> 8;
	if (bcdval & 0xff) {
		data[5] = bcdval;
		len++;
	}

	return dvbfe_do_diseqc_command(fe, data, len);
}

int dvbfe_diseqc_set_channel(struct dvbfe_handle *fe,
			     enum dvbfe_diseqc_address address,
			     uint16_t channel)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x59, 0x00, 0x00};

	data[3] = channel >> 8;
	data[4] = channel;

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_halt_positioner(struct dvbfe_handle *fe,
				 enum dvbfe_diseqc_address address)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x60};

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_disable_limits(struct dvbfe_handle *fe,
				enum dvbfe_diseqc_address address)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x63};

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_set_limit(struct dvbfe_handle *fe,
			   enum dvbfe_diseqc_address address,
			   enum dvbfe_diseqc_direction direction)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x66};

	if (direction == DISEQC_DIRECTION_WEST)
		data[2] = 0x67;

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_drive_motor(struct dvbfe_handle *fe,
			     enum dvbfe_diseqc_address address,
			     enum dvbfe_diseqc_direction direction,
			     enum dvbfe_diseqc_drive_mode mode,
			     uint8_t value)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x68, 0x00};

	if (direction == DISEQC_DIRECTION_WEST)
		data[2] = 0x69;

	switch(mode) {
	case DISEQC_DRIVE_MODE_STEPS:
		data[3] = (value & 0x7f) | 0x80;
		break;
	case DISEQC_DRIVE_MODE_TIMEOUT:
		data[3] = value & 0x7f;
		break;
	}

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_store_preset(struct dvbfe_handle *fe,
			      enum dvbfe_diseqc_address address,
			      uint8_t id)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x6A, id};

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

int dvbfe_diseqc_goto_preset(struct dvbfe_handle *fe,
			     enum dvbfe_diseqc_address address,
			     uint8_t id)
{
	uint8_t data[] = { DISEQC_FRAMING_MASTER_NOREPLY, address, 0x6B, id};

	return dvbfe_do_diseqc_command(fe, data, sizeof(data));
}

/*
				int integer = 0;
				int fraction = 0;
				char *tmp;

				if (sscanf(command+i+12, "%i %s", &addr, value_s) != 2)
					return -EINVAL;

				// parse the integer and fractional parts using fixed point
				integer = atoi(value_s);
				tmp = strchr(value_s, '.');
				if (tmp != NULL) {
					tmp++;
					tmp[3] = 0;
					fraction = ((atoi(tmp) * 16000) / 1000000) & 0xf;
				}

				// generate the command
				master_cmd.msg[0] = 0xe0;
				master_cmd.msg[1] = addr;
				master_cmd.msg[2] = 0x6e;
				if (integer < -256) {
					return -EINVAL;
				} else if (integer < 0) {
					integer = -integer;
					master_cmd.msg[3] = 0xf0;
				} else if (integer < 256) {
					master_cmd.msg[3] = 0x00;
				} else if (integer < 512) {
					integer -= 256;
					master_cmd.msg[3] = 0x10;
				} else {
					return -EINVAL;
				}
				master_cmd.msg[3] |= ((integer / 16) & 0x0f);
				integer = integer % 16;
				master_cmd.msg[4] |= ((integer & 0x0f) << 4) | fraction;
				master_cmd.msg_len = 5;
*/
