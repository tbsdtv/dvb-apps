/*
	ASN.1 routines, implementation for libdvben50221
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

#include <stdio.h>
#include "asn_1.h"

uint32_t asn_1_decode(uint8_t *asn_1_array)
{
	uint8_t length_field = 0, count = 0;
	uint32_t asn_1_words = 0, length = 0;

	length_field = asn_1_array[0];
	if (length_field < 128)
		// there is only one word
		length = length_field & 0x7f;
	else {
		asn_1_words = length_field & 0x7f;
		while (count < asn_1_words) {
			length = length << asn_1_array[count + 1];
			count++;
		}
	}

	return length;
}

uint8_t *asn_1_encode(uint16_t length, uint32_t *asn_1_words)
{
	uint32_t temp = 0;
	uint8_t *asn_1_array;

	printf("%s: Length=[%d]\n", __FUNCTION__, length);
	if (length < 0x80) {
		uint8_t *length_indicator = (uint8_t *) malloc(sizeof(uint8_t));
		*length_indicator = length & 0x7f;
		printf("%s: length indicator=[%02x]\n", __FUNCTION__, *length_indicator);
		*asn_1_words = 1;	// there is only one word in this case.

		return length_indicator;
	}
	else if (length > 127) {
		temp = length, *asn_1_words = 0;

		while (temp) {
			temp = temp >> 8;
			*asn_1_words++;
		}
		if ((asn_1_array = (uint8_t *) malloc(*asn_1_words * sizeof (uint8_t))) == NULL) {
			printf("%s: Memory allocation failed.\n", __FUNCTION__);
			return NULL;
		}
		printf("%s: Allocated %d bytes.\n", __FUNCTION__, *asn_1_words);

		while (length) {
			asn_1_array[*asn_1_words--] = length & 0xff;
			length = length >> 8;
		}
		return asn_1_array;
	}
	else
		printf("%s: Error in length.\n", __FUNCTION__);

	return NULL;
}
