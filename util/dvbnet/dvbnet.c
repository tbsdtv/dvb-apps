/* 
 * dvbnet.c
 *
 * Copyright (C) 2003 TV Files S.p.A
 *                    L.Y.Mesentsev <lymes@tiscalinet.it>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 * Or, point your browser to http://www.gnu.org/copyleft/gpl.html
 * 
 */

#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/dvb/net.h>

#ifndef VERSION_INFO
#define VERSION_INFO "1.1.0"
#endif

#define OK    0
#define FAIL -1
#define DVB_NET_DEVICE "/dev/dvb/adapter%d/net%d"
#define DVB_NET_DEVICES_MAX 10
#define IFNAME_DVB "dvb"


enum Mode {
	UNKNOWN,
	LST_INTERFACE,
	ADD_INTERFACE,
	DEL_INTERFACE
} op_mode;

static int adapter = 0;
static int netdev = 0;
static struct dvb_net_if net_data;

static void hello(void);
static void usage(char *);
static void parse_args(int, char **);
static int queryInterface(int, int);

static char dvb_net_device[40];

int main(int argc, char **argv)
{
	int fd_net;

	hello();

	parse_args(argc, argv);

	sprintf(dvb_net_device, DVB_NET_DEVICE, adapter, netdev);

	printf("Device: %s\n", dvb_net_device);

	if ((fd_net = open(dvb_net_device, O_RDWR | O_NONBLOCK)) < 0) {
		fprintf(stderr, "Error: couldn't open device %s: %d %m\n",
			dvb_net_device, errno);
		return FAIL;
	}

	switch (op_mode) {
	case DEL_INTERFACE:
		if (ioctl(fd_net, NET_REMOVE_IF, net_data.if_num))
			fprintf(stderr,
				"Error: couldn't remove interface %d: %d %m.\n",
				net_data.if_num, errno);
		else
			printf("Status: device %d removed successfully.\n",
			       net_data.if_num);
		break;

	case ADD_INTERFACE:
		if (ioctl(fd_net, NET_ADD_IF, &net_data))
			fprintf(stderr,
				"Error: couldn't add interface for pid %d: %d %m.\n",
				net_data.pid, errno);
		else
			printf
			    ("Status: device dvb%d_%d for pid %d created successfully.\n",
			     adapter, net_data.if_num, net_data.pid);
		break;

	case LST_INTERFACE:
		queryInterface(fd_net, 0);
		break;

	default:
		usage(argv[0]);
		return FAIL;
	}

	close(fd_net);
	return OK;
}


int queryInterface(int fd_net, int dev)
{
	struct dvb_net_if data;
	int IF, nIFaces = 0, ret = FAIL;

	printf("Query DVB network interfaces:\n");
	printf("-----------------------------\n");
	for (IF = 0; IF < DVB_NET_DEVICES_MAX; IF++) {
		data.if_num = IF;
		if (ioctl(fd_net, NET_GET_IF, &data))
			continue;

		if (dev == data.if_num)
			ret = OK;

		printf("Found device %d: interface dvb%d_%d, "
		       "listening on PID %d\n",
		       IF, adapter, data.if_num, data.pid);

		nIFaces++;
	}

	printf("-----------------------------\n");
	printf("Found %d interface(s).\n\n", nIFaces);
	return ret;
}


void parse_args(int argc, char **argv)
{
	char c, *s;
	op_mode = UNKNOWN;
	net_data.feedtype = DVB_NET_FEEDTYPE_MPE;
	while ((c = getopt(argc, argv, "a:n:p:d:lUvh")) != EOF) {
		switch (c) {
		case 'a':
			adapter = strtol(optarg, NULL, 0);
			break;
		case 'n':
			netdev = strtol(optarg, NULL, 0);
			break;
		case 'p':
			net_data.pid = strtol(optarg, NULL, 0);
			op_mode = ADD_INTERFACE;
			break;
		case 'd':
			net_data.if_num = strtol(optarg, NULL, 0);
			op_mode = DEL_INTERFACE;
			break;
		case 'l':
			op_mode = LST_INTERFACE;
			break;
		case 'U':
			net_data.feedtype = DVB_NET_FEEDTYPE_ULE;
			break;
		case 'v':
			exit(OK);
		case 'h':
		default:
			s = strrchr(argv[0], '/') + 1;
			usage((s) ? s : argv[0]);
			exit(FAIL);
		}
	}
}


void usage(char *prog_name)
{
	fprintf(stderr, "Usage: %s [options]\n", prog_name);
	fprintf(stderr, "Where options are:\n");
	fprintf(stderr, "\t-a AD  : Adapter card AD (default 0)\n");
	fprintf(stderr, "\t-n NET : Net demux NET (default 0)\n");
	fprintf(stderr, "\t-p PID : Add interface listening on PID\n");
	fprintf(stderr, "\t-d NUM : Remove interface dvbAD_NUM\n");
	fprintf(stderr,
		"\t-l     : List currently available interfaces\n");
	fprintf(stderr, "\t-U     : use ULE framing (defualt: MPE)\n" );
	fprintf(stderr, "\t-v     : Print current version\n\n");
}


void hello(void)
{
	printf("\nDVB Network Interface Manager\n");
	printf("Version %s\n", VERSION_INFO);
	printf("Copyright (C) 2003, TV Files S.p.A\n\n");
}
