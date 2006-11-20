/*
	dvbscan utility

	Copyright (C) 2006 Andrew de Quincey (adq_dvb@lidskialf.net)

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the

	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include <stdlib.h>
#include "dvbscan.h"

// transponders we have yet to scan
static struct transponder *toscan = NULL;
static struct transponder *toscan_end = NULL;

// transponders we have scanned
static struct transponder *scanned = NULL;
static struct transponder *scanned_end = NULL;


int main(int argc, char *argv[])
{
	// FIXME: parse args.

	// FIXME: load the initial scan file

	// main scan loop
	while(toscan) {
		// FIXME: have we already scanned this one?

		// FIXME: tune it

		// FIXME: scan it

		// add to scanned list.
		if (scanned_end == NULL) {
			scanned = toscan;
		} else {
			scanned_end->next = toscan;
		}
		scanned_end = toscan;
		toscan->next = NULL;

		// remove from toscan list
		toscan = toscan->next;
		if (toscan == NULL)
			toscan_end = NULL;
	}

	// FIXME: output the data

	return 0;
}
