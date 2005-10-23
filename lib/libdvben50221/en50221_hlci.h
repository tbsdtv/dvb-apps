/*
	en50221 encoder An implementation for libdvb
	an implementation for the High Level Common Interface

	Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)

	This library is free software; you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as
	published by the Free Software Foundation; either version 2.1 of
	the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.

	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
*/


#ifndef __EN50221_HLCI_H__
#define __EN50221_HLCI_H__


#include <stdlib.h>
#include <stdint.h>
#include "si.h"
#include "pmt.h"


#define	CA_APP_INFO_ENQUIRY	0x9f8020
#define	CA_APP_INFO		0x9f8021
#define	CA_ENTER_MENU		0x9f8022
#define CA_INFO_ENQUIRY		0x9f8030
#define	CA_INFO			0x9f8031
#define CA_PMT			0x9f8032
#define CA_PMT_REPLY		0x9f8033

#define CA_CLOSE_MMI		0x9f8800
#define CA_DISPLAY_CONTROL	0x9f8801
#define CA_DISPLAY_REPLY	0x9f8802
#define CA_TEXT_LAST		0x9f8803
#define CA_TEXT_MORE		0x9f8804
#define CA_KEYPAD_CONTROL	0x9f8805
#define CA_KEYPRESS		0x9f8806

#define CA_ENQUIRY		0x9f8807
#define CA_ANSWER		0x9f8808
#define CA_MENU_LAST		0x9f8809
#define CA_MENU_MORE		0x9f880a
#define CA_MENU_ANSWER		0x9f880b
#define CA_LIST_LAST		0x9f880c
#define CA_LIST_MORE		0x9f880d

#define SIZE_INDICATOR		1

#define OK_DESCRAMBLING		0x01
#define OK_MMI			0x02
#define QUERY			0x03
#define NOT_SELECTED		0x04

#define MORE			0x00
#define FIRST			0x01
#define LAST			0x02
#define ONLY			0x03
#define ADD			0x04
#define UPDATE			0x05


struct en50221_stream {
	unsigned stream_type: 8;
	unsigned reserved_1: 3;
	unsigned elementary_pid: 13;
	unsigned reserved_2: 4;
	unsigned es_info_length: 12;

	unsigned ca_pmt_cmd_id: 8;

	uint8_t streams_desc_count;
	struct descriptor *p_descriptors;
};

struct en50221_pmt_object {
	unsigned ca_pmt_tag: 24;
	uint8_t *asn_1_length;
	unsigned ca_pmt_list_mgmt: 8;
	unsigned program_number: 16;
	unsigned reserved_1: 2;
	unsigned version_number: 5;
	unsigned current_next: 1;
	unsigned reserved_2: 4;
	unsigned program_info_length: 12;
	unsigned ca_pmt_cmd_id: 8;

	uint8_t program_desc_count;
	struct descriptor *p_en50221_prog_desc;

	uint8_t stream_count;
	struct en50221_stream *p_en50221_streams;
};

uint16_t do_en50221_pmt_object(struct en50221_pmt_object *p_en50221_pmt_object, struct service_info *p_si, uint8_t pmt_list_mgmt, uint8_t pmt_command, int move_to_programme);
uint16_t write_en50221_pmt_object(struct en50221_pmt_object *p_en50221_pmt_object, char *ca_dev);

#endif
