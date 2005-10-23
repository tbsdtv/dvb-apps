/*
	libdvbsi, A SI parser implementation for libdvb
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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "si.h"
#include "pat.h"
#include "pmt.h"
#include "filter.h"

#include <linux/dvb/dmx.h>
#define MAX_SECTION_SIZE	8192

int parse_si(struct service_info *p_si, struct channel_params *p_params, char *adapter, char *frontend, char *dmxdev)
{
	uint16_t pid = 0, tid = 0, pmt_pid = 0;
	int pat_demux_fd, pmt_demux_fd, bytes = 0;
	uint8_t i;
	uint8_t pat_buf[MAX_SECTION_SIZE];
	uint8_t pmt_buf[MAX_SECTION_SIZE];
	uint8_t polarity = 0;

	struct pat *p_pat = (struct pat *) malloc(sizeof (struct pat));
	struct pmt *p_pmt = (struct pmt *) malloc(sizeof (struct pmt));
	struct program_descriptor *p_prog_desc = malloc(sizeof (struct program_descriptor) * MAX_PROGRAMS);

	/*		Initialize		*/
	p_pat->p_program_descriptor = NULL;
	p_pmt->p_descriptors = NULL;
	p_pmt->p_streams = NULL;


	/*		PAT parsing		*/
	if ((pat_demux_fd = open(dmxdev, O_RDWR)) < 0){
		perror("Open");
		return -1;
	}
	/*	PID 0 filter	*/
	add_filter(pat_demux_fd, pid, tid, 0);
	bytes = read(pat_demux_fd, pat_buf, sizeof(pat_buf));
	if (bytes < 0) {
		perror("Read");
		return -1;
	}
	p_pat->p_program_descriptor = p_prog_desc;
	parse_pat(p_pat, pat_buf, pid, bytes);

	/*	Get PMT PID	*/
	for (i = 0; i < p_pat->program_count; i++) {
		p_pat->p_program_descriptor = &p_prog_desc[i];
		if (p_params->service_id == p_pat->p_program_descriptor->program_number) {
			pmt_pid = p_pat->p_program_descriptor->pid;
			printf("%s: PMT PID = [%d]\n", __FUNCTION__, pmt_pid);
		}
	}

	printf("%s: PAT: Close Demux %s\n", __FUNCTION__, dmxdev);
	close(pat_demux_fd);

	if (p_params->polarity == 'h')
		polarity = 0;
	else
		polarity = 1;

	/*		PMT parsing		*/
	tid = 0x02;
	if ((pmt_demux_fd = open(dmxdev, O_RDWR)) < 0){
		perror("Open");
		return -1;
	}

	add_filter(pmt_demux_fd, pmt_pid, tid, p_params->service_id);
	bytes = read(pmt_demux_fd, pmt_buf, sizeof(pmt_buf));
	if (bytes < 0) {
		perror("Read");
		return -1;
	}
	parse_pmt(p_pmt, pmt_buf, pmt_pid, bytes);
	close(pmt_demux_fd);

	p_si->p_pat = p_pat;
	p_si->p_pmt = p_pmt;

	return 0;
}
