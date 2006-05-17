/* femon -- monitor frontend status
 *
 * Copyright (C) 2003 convergence GmbH
 * Johannes Stezenbach <js@convergence.de>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <stdint.h>
#include <sys/time.h>

#include <linux/dvb/frontend.h>

#ifndef TRUE
#define TRUE (1==1)
#endif
#ifndef FALSE
#define FALSE (1==0)
#endif


#define FRONTENDDEVICE "/dev/dvb/adapter%d/frontend%d"

static char *usage_str =
    "\nusage: femon [options]\n"
    "     -r        : human readable output\n"
    "     -a number : use given adapter (default 0)\n"
    "     -f number : use given frontend (default 0)\n\n";


static void usage(void)
{
   fprintf(stderr, usage_str);
   exit(1);
}


static
int check_frontend (int fe_fd, int human_readable)
{
   fe_status_t status;
   uint16_t snr, signal;
   uint32_t ber, uncorrected_blocks;

   do {
      ioctl(fe_fd, FE_READ_STATUS, &status);
      ioctl(fe_fd, FE_READ_SIGNAL_STRENGTH, &signal);
      ioctl(fe_fd, FE_READ_SNR, &snr);
      ioctl(fe_fd, FE_READ_BER, &ber);
      ioctl(fe_fd, FE_READ_UNCORRECTED_BLOCKS, &uncorrected_blocks);

	  if (human_readable) {
	      printf ("status %02x | signal %02d | snr %02d | ber %08x | unc %08x | ",
		      status, (signal * 100) / 0xffff, (snr * 100) / 0xffff, ber, uncorrected_blocks);
	  } else {
	      printf ("status %02x | signal %04x | snr %04x | ber %08x | unc %08x | ",
		      status, signal, snr, ber, uncorrected_blocks);
	  }

      if (status & FE_HAS_LOCK)
	 printf("FE_HAS_LOCK");

      printf("\n");
      usleep(1000000);
   } while (1);

   return 0;
}


static
int do_mon(unsigned int adapter, unsigned int frontend, int human_readable)
{
   char fedev[128];
   int fefd;
   int result;
   struct dvb_frontend_info fe_info;

   snprintf(fedev, sizeof(fedev), FRONTENDDEVICE, adapter, frontend);
   printf("using '%s'\n", fedev);

   if ((fefd = open(fedev, O_RDONLY | O_NONBLOCK)) < 0) {
      perror("opening frontend failed");
      return FALSE;
   }

   result = ioctl(fefd, FE_GET_INFO, &fe_info);

   if (result < 0) {
      perror("ioctl FE_GET_INFO failed");
      close(fefd);
      return FALSE;
   }

   printf("FE: %s (%s)\n", fe_info.name, fe_info.type == FE_QPSK ? "SAT" :
		   fe_info.type == FE_QAM ? "CABLE": "TERRESTRIAL");

   check_frontend (fefd, human_readable);

   close(fefd);

   return result;
}

int main(int argc, char *argv[])
{
   unsigned int adapter = 0, frontend = 0;
   int human_readable = 0;
   int opt;

   while ((opt = getopt(argc, argv, "hra:f:")) != -1) {
      switch (opt)
      {
	 case '?':
	 case 'h':
	 default:
	    usage();
	 case 'a':
	    adapter = strtoul(optarg, NULL, 0);
	    break;
	 case 'f':
	    frontend = strtoul(optarg, NULL, 0);
	 case 'r':
		human_readable = 1;
      }
   }

   do_mon(adapter, frontend, human_readable);

   return FALSE;
}

