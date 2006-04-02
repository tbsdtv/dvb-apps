/*
  CA-ZAP utility

  Copyright (C) 2004, 2005 Manu Abraham (manu@kromtek.com)
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

#include <stdio.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <dvben50221/en50221_session.h>
#include <dvben50221/en50221_transport.h>
#include <dvben50221/en50221_app_utils.h>
#include <dvben50221/en50221_app_ai.h>
#include <dvben50221/en50221_app_ca.h>
#include <dvben50221/en50221_app_mmi.h>
#include <dvben50221/en50221_app_tags.h>
#include <dvbapi/dvbca.h>
#include "ca_zap.h"
#include "ca_zap_hlci.h"

struct en50221_app_send_functions sendfuncs;

static int hlci_send_data(void *arg, uint16_t session_number, uint8_t *data, uint16_t data_length);
static int hlci_send_datav(void *arg, uint16_t session_number, struct iovec *vector, int iov_count);

static int cafd = -1;



int hlci_init()
{
	// create the sendfuncs
	sendfuncs.arg        = NULL;
	sendfuncs.send_data  = hlci_send_data;
	sendfuncs.send_datav = hlci_send_datav;

	// create the application information resource
	ai_resource = en50221_app_ai_create(&sendfuncs);

	// create the CA resource
	ca_resource = en50221_app_ca_create(&sendfuncs);

	// create the MMI resource
//	mmi_resource = en50221_app_mmi_create(&sendfuncs);

	// no CAM present just now
	cafd = -1;

	return 0;
}

int hlci_cam_added(int _cafd)
{
	uint8_t buf[256];
	int size;

	// remember the ca fd
	cafd = _cafd;

	// get application information
	if (en50221_app_ai_enquiry(ai_resource, 0)) {
		fprintf(stderr, "Failed to send AI INFO enquiry\n");
		cafd = -1;
		return -1;
	}
	if ((size = dvbca_hlci_read(cafd, TAG_APP_INFO, buf, sizeof(buf))) < 0) {
		fprintf(stderr, "Failed to read AI INFO\n");
		cafd = -1;
		return -1;
	}
	if (en50221_app_ai_message(ai_resource, 0, 0, EN50221_APP_AI_RESOURCEID, buf, size)) {
		fprintf(stderr, "Failed to parse AI INFO\n");
		cafd = -1;
		return -1;
	}

	// we forge a fake CA_INFO here so the main ca_zap code works -
	// this will be replaced with a proper call (below) when the driver support is there
	buf[0] = TAG_CA_INFO >> 16;
	buf[1] = TAG_CA_INFO >> 8;
	buf[2] = TAG_CA_INFO;
	buf[3] = 0;
	if (en50221_app_ca_message(ca_resource, 0, 0, EN50221_APP_CA_RESOURCEID, buf, 4)) {
		fprintf(stderr, "Failed to parse AI INFO\n");
		cafd = -1;
		return -1;
	}

	// get CA information
	/*
	if (en50221_app_ca_info_enq(ca_resource, 0)) {
		fprintf(stderr, "Failed to send CA INFO enquiry\n");
		cafd = -1;
		return -1;
	}
	if ((size = dvbca_hlci_read(cafd, TAG_CA_INFO, buf, sizeof(buf))) < 0) {
		fprintf(stderr, "Failed to read CA INFO\n");
		cafd = -1;
		return -1;
	}
	if (en50221_app_ca_message(ca_resource, 0, 0, EN50221_APP_CA_RESOURCEID, buf, size)) {
		fprintf(stderr, "Failed to parse CA INFO\n");
		cafd = -1;
		return -1;
	}
	*/

	// done
	return 0;
}

void hlci_cam_removed()
{
	cafd = -1;
}

void hlci_poll()
{
	// we do nothing here for the moment
	usleep(10);
}

void hlci_shutdown()
{
	en50221_app_ai_destroy(ai_resource);
	en50221_app_ca_destroy(ca_resource);
//	en50221_app_mmi_destroy(mmi_resource);
}

static int hlci_send_data(void *arg, uint16_t session_number, uint8_t *data, uint16_t data_length)
{
	(void)arg;
	(void)session_number;

	return dvbca_hlci_write(cafd, data, data_length);
}

static int hlci_send_datav(void *arg, uint16_t session_number, struct iovec *vector, int iov_count)
{
	(void)arg;
	(void)session_number;

	// calculate the total length of the data to send
	uint32_t data_size = 0;
	int i;
	for(i=0; i< iov_count; i++) {
		data_size += vector[i].iov_len;
	}

	// allocate memory for it
	uint8_t *buf = malloc(data_size);
	if (buf == NULL) {
		return -1;
	}

	// merge the iovecs
	uint32_t pos = 0;
	for(i=0; i< iov_count; i++) {
		memcpy(buf+pos, vector[i].iov_base, vector[i].iov_len);
		pos += vector[i].iov_len;
	}

	// sendit and cleanup
	int status = dvbca_hlci_write(cafd, buf, data_size);
	free(buf);
	return status;
}
