/*

   dvbdate - a program to set the system date and time from a TDT multiplex

   Copyright (C) Laurence Culhane 2002 <dvbdate@holmes.demon.co.uk>

   Mercilessly ripped off from dvbtune, Copyright (C) Dave Chapman 2001

   Revamped by Johannes Stezenbach <js@convergence.de>
   and Michael Hunold <hunold@convergence.de>
  
   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
   Or, point your browser to http://www.gnu.org/copyleft/gpl.html

   Copyright (C) Laurence Culhane 2002 <dvbdate@holmes.demon.co.uk>

*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <errno.h>
#include <getopt.h>
#include <stdarg.h>

#include <linux/dvb/dmx.h>

#define bcdtoint(i) ((((i & 0xf0) >> 4) * 10) + (i & 0x0f))

/* How many seconds can the system clock be out before we get warned? */
#define ALLOWABLE_DELTA 30*60

char *ProgName;
int do_print;
int do_set;
int do_force;
int do_quiet;
int timeout = 25;

void errmsg(char *message, ...)
{
	va_list ap;

	va_start(ap, message);
	fprintf(stderr, "%s: ", ProgName);
	vfprintf(stderr, message, ap);
	va_end(ap);
}

void usage()
{
	fprintf(stderr, "usage: %s [-p] [-s] [-f] [-q] [-h]\n", ProgName);
	_exit(1);
}

void help()
{
	fprintf(stderr, "\nhelp:\n" "%s [-p] [-s] [-f] [-q] [-h] [-t n]\n" "  --print	(print current time, TDT time and delta)\n" "  --set	(set the system clock to TDT time)\n" "  --force	(force the setting of the clock)\n" "  --quiet	(be silent)\n" "  --help	(display this message)\n""  --timout n	(max seconds to wait, default: 25)\n", ProgName);
	_exit(1);
}

int do_options(int arg_count, char **arg_strings)
{
	static struct option Long_Options[] = {
		{"print", 0, 0, 'p'},
		{"set", 0, 0, 's'},
		{"force", 0, 0, 'f'},
		{"quiet", 0, 0, 'q'},
		{"help", 0, 0, 'h'},
		{"timeout", 1, 0, 't'},
		{0, 0, 0, 0}
	};
	int c;
	int Option_Index = 0;

	while (1) {
		c = getopt_long(arg_count, arg_strings, "psfqht:", Long_Options, &Option_Index);
		if (c == EOF)
			break;
		switch (c) {
		case 't':
			timeout = atoi(optarg);
			if (0 == timeout) {
				fprintf(stderr, "%s: invalid timeout value\n", ProgName);
				usage();
			}
			break;
		case 'p':
			do_print = 1;
			break;
		case 's':
			do_set = 1;
			break;
		case 'f':
			do_force = 1;
			break;
		case 'q':
			do_quiet = 1;
			break;
		case 'h':
			help();
			break;
		case '?':
			usage();
			break;
		case 0:
/*
 * Which long option has been selected?  We only need this extra switch
 * to cope with the case of wanting to assign two long options the same
 * short character code.
 */
			printf("long option index %d\n", Option_Index);
			switch (Option_Index) {
			case 0:	/* Print */
			case 1:	/* Set */
			case 2:	/* Force */
			case 3:	/* Quiet */
			case 4:	/* Help */
			case 5:	/* timout */
				break;
			default:
				fprintf(stderr, "%s: unknown long option %d\n", ProgName, Option_Index);
				usage();
			}
			break;
/*
 * End of Special Long-opt handling code
 */
		default:
			fprintf(stderr, "%s: unknown getopt error - returned code %02x\n", ProgName, c);
			_exit(1);
		}
	}
	return 0;
}

/*
 * return the TDT time in UNIX time_t format
 */

time_t convert_date(unsigned char *dvb_buf)
{
	int i;
	int year, month, day, hour, min, sec;
	long int mjd;
	struct tm dvb_time;

	mjd = (dvb_buf[0] & 0xff) << 8;
	mjd += (dvb_buf[1] & 0xff);
	hour = bcdtoint(dvb_buf[2] & 0xff);
	min = bcdtoint(dvb_buf[3] & 0xff);
	sec = bcdtoint(dvb_buf[4] & 0xff);
/*
 * Use the routine specified in ETSI EN 300 468 V1.4.1,
 * "Specification for Service Information in Digital Video Broadcasting"
 * to convert from Modified Julian Date to Year, Month, Day.
 */
	year = (int) ((mjd - 15078.2) / 365.25);
	month = (int) ((mjd - 14956.1 - (int) (year * 365.25)) / 30.6001);
	day = mjd - 14956 - (int) (year * 365.25) - (int) (month * 30.6001);
	if (month == 14 || month == 15)
		i = 1;
	else
		i = 0;
	year += i;
	month = month - 1 - i * 12;

	dvb_time.tm_sec = sec;
	dvb_time.tm_min = min;
	dvb_time.tm_hour = hour;
	dvb_time.tm_mday = day;
	dvb_time.tm_mon = month - 1;
	dvb_time.tm_year = year;
	dvb_time.tm_isdst = -1;
	dvb_time.tm_wday = 0;
	dvb_time.tm_yday = 0;
	return (timegm(&dvb_time));
}


/*
 * Get the next UTC date packet from the TDT multiplex
 */

int scan_date(time_t *dvb_time, unsigned int to)
{
	int fd_date;
	int n, seclen;
	time_t t;
	unsigned char buf[4096];
	struct dmx_sct_filter_params sctFilterParams;
	struct pollfd ufd;
	int found = 0;
	
	t = 0;
	if ((fd_date = open("/dev/dvb/adapter0/demux0", O_RDWR | O_NONBLOCK)) < 0) {
		perror("fd_date DEVICE: ");
		return -1;
	}

	memset(&sctFilterParams, 0, sizeof(sctFilterParams));
	sctFilterParams.pid = 0x14;
	sctFilterParams.timeout = 0;
	sctFilterParams.flags = DMX_IMMEDIATE_START;
	sctFilterParams.filter.filter[0] = 0x70;
	sctFilterParams.filter.mask[0] = 0xff;

	if (ioctl(fd_date, DMX_SET_FILTER, &sctFilterParams) < 0) {
		perror("DATE - DMX_SET_FILTER:");
		close(fd_date);
		return -1;
	}

	while (to > 0) {
		int res;

		memset(&ufd,0,sizeof(ufd));
		ufd.fd=fd_date;
		ufd.events=POLLIN;

		res = poll(&ufd,1,1000);
		if (0 == res) {
			fprintf(stdout, ".");
			fflush(stdout);
			to--;
			continue;
  		}
		if (1 == res) {
			found = 1;
			break;
		}
		errmsg("error polling for data");
		close(fd_date);
		return -1;
	}
	fprintf(stdout, "\n");
	if (0 == found) {
		errmsg("timeout - try tuning to a multiplex?\n");
		close(fd_date);
		return -1;
	}

	if ((n = read(fd_date, buf, 4096)) >= 3) {
		seclen = ((buf[1] & 0x0f) << 8) | (buf[2] & 0xff);
		if (n == seclen + 3) {
			t = convert_date(&(buf[3]));
		} else {
			errmsg("Under-read bytes for DATE - wanted %d, got %d\n", seclen, n);
			return 0;
		}
	} else {
		errmsg("Nothing to read from fd_date - try tuning to a multiplex?\n");
		return 0;
	}
	close(fd_date);
	*dvb_time = t;
	return 0;
}


/*
 * Set the system time
 */
int set_time(time_t * new_time)
{
	if (stime(new_time)) {
		perror("Unable to set time");
		return -1;
	}
	return 0;
}


int main(int argc, char **argv)
{
	time_t dvb_time;
	time_t real_time;
	time_t offset;
	int ret;

	do_print = 0;
	do_force = 0;
	do_set = 0;
	do_quiet = 0;
	ProgName = argv[0];

/*
 * Process command line arguments
 */
	do_options(argc, argv);
	if (do_quiet && do_print) {
		errmsg("quiet and print options are mutually exclusive.\n");
		exit(1);
	}
/*
 * Get the date from the currently tuned TDT multiplex
 */
	ret = scan_date(&dvb_time, timeout);
	if (ret != 0) {
		errmsg("Unable to get time from multiplex.\n");
		exit(1);
	}
	time(&real_time);
	offset = dvb_time - real_time;
	if (do_print) {
		fprintf(stdout, "System time: %s", ctime(&real_time));
		fprintf(stdout, "   TDT time: %s", ctime(&dvb_time));
		fprintf(stdout, "     Offset: %ld seconds\n", offset);
	} else if (!do_quiet) {
		fprintf(stdout, "%s", ctime(&dvb_time));
	}
	if (do_set) {
		if (labs(offset) > ALLOWABLE_DELTA) {
			if (do_force) {
				if (0 != set_time(&dvb_time)) {
					errmsg("setting the time failed\n");
				}
			} else {
				errmsg("multiplex time differs by more than %d from system.\n", ALLOWABLE_DELTA);
				errmsg("use -f to force system clock to new time.\n");
				exit(1);
			}
		} else {
			set_time(&dvb_time);
		}
	}			/* #end if (do_set) */
	return (0);
}
