#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/poll.h>
#include <sys/time.h>
#include <string.h>

#include <linux/dvb/dmx.h>

#define BSIZE 188

int pidt[0x2001];

int main(int argc, char **argv)
{
	int fd, ffd, packets = 0;
	struct timeval startt;
	struct dmx_pes_filter_params flt;
	char *search;
	unsigned char buffer[BSIZE];

	fd = open("/dev/dvb/adapter0/dvr0", O_RDONLY);

	ioctl(fd, DMX_SET_BUFFER_SIZE, 1024 * 1024);

	ffd = open("/dev/dvb/adapter0/demux0", O_RDWR);
	if (ffd < 0) {
		perror("/dev/dvb/adapter0/demux0");
		return -fd;
	}

	flt.pid = 0x2000;
	flt.input = DMX_IN_FRONTEND;
	flt.output = DMX_OUT_TS_TAP;
	flt.pes_type = DMX_PES_OTHER;
	flt.flags = 0;

	if (ioctl(ffd, DMX_SET_PES_FILTER, &flt) < 0) {
		perror("DMX_SET_PES_FILTER");
		return -1;
	}

	if (ioctl(ffd, DMX_START, 0) < 0) {
		perror("DMX_SET_PES_FILTER");
		return -1;
	}

	gettimeofday(&startt, 0);

	if (argc > 1)
		search = argv[1];
	else
		search = 0;

	while (1) {
		int pid, r, ok;
		if ((r = read(fd, buffer, 188)) <= 0) {
			perror("read");
			break;
		}
		if (r != 188) {
			printf("only read %d\n", r);
			break;
		}
		if (buffer[0] != 0x47) {
			continue;
			printf("desync (%x)\n", buffer[0]);
			while (buffer[0] != 0x47)
				read(fd, buffer, 1);
			continue;
		}
		ok = 1;
		pid = ((((unsigned) buffer[1]) << 8) |
		       ((unsigned) buffer[2])) & 0x1FFF;

		if (search) {
			int i, sl = strlen(search);
			ok = 0;
			if (pid != 0x1fff) {
				for (i = 0; i < (188 - sl); ++i) {
					if (!memcmp(buffer + i, search, sl))
						ok = 1;
				}
			}
		}

		if (ok) {
			pidt[pid]++;
			pidt[0x2000]++;
		}

		packets++;

		if (!(packets & 0xFF)) {
			struct timeval now;
			int diff;
			gettimeofday(&now, 0);
			diff =
			    (now.tv_sec - startt.tv_sec) * 1000 +
			    (now.tv_usec - startt.tv_usec) / 1000;
			if (diff > 1000) {
				int pid = 0;
				for (pid = 0; pid < 0x2001; pid++) {
					if (pidt[pid]) {
						printf("%04x %5d p/s %5d kb/s %5d kbit\n",
						     pid,
						     pidt[pid] * 1000 / diff,
						     pidt[pid] * 1000 / diff * 188 / 1024,
						     pidt[pid] * 8 * 1000 / diff * 188 / 1000);
					}
					pidt[pid] = 0;
				}
				printf("-PID--FREQ-----BANDWIDTH-BANDWIDTH-\n");
				startt = now;
			}
		}
	}

	close(ffd);
	close(fd);
	return 0;
}

