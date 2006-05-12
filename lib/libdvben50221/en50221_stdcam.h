/*
	en50221 encoder An implementation for libdvb
	an implementation for the en50221 transport layer

	Copyright (C) 2006 Andrew de Quincey (adq_dvb@lidskialf.net)

	This program is free software; you can redistribute it and/or modify
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

#ifndef EN50221_STDCAM_H
#define EN50221_STDCAM_H 1

#include <libdvben50221/en50221_app_ai.h>
#include <libdvben50221/en50221_app_ca.h>
#include <libdvben50221/en50221_app_mmi.h>

enum en50221_stdcam_status {
	EN50221_STDCAM_CAM_MISSING,
	EN50221_STDCAM_CAM_OK,
};

struct en50221_stdcam {
	/* one of more of the following may be NULL if a CAM does not support it */
	struct en50221_app_ai *ai_resource;
	struct en50221_app_ca *ca_resource;
	struct en50221_app_mmi *mmi_resource;

	/* destroy the stdcam instance */
	void (*destroy)(struct en50221_stdcam *stdcam);

	/* poll the stdcam instance */
	enum en50221_stdcam_status (*poll)(struct en50221_stdcam *stdcam);
};

extern struct en50221_stdcam *en50221_stdcam_hlci_create(int cafd, int slotid);

#endif
