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

#ifndef __ASN_1_H__
#define __ASN_1_H__


#include <stdlib.h>
#include <stdint.h>

uint32_t asn_1_decode(uint8_t *asn_1_array);
uint8_t *asn_1_encode(uint16_t length, uint32_t *asn_1_words);

#endif
