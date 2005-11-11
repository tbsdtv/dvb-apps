#include <ucsi/mpeg/descriptor.h>
#include <ucsi/mpeg/section.h>
#include <ucsi/dvb/descriptor.h>
#include <ucsi/dvb/section.h>
#include <ucsi/transport_packet.h>
#include <ucsi/section_buf.h>
#include <stdio.h>
#include <unistd.h>
#include <dvbdemux.h>

int main(int argc, char *argv[])
{
	int demuxfd;
	int dvrfd;
	int adapter;
	unsigned char databuf[TRANSPORT_PACKET_LENGTH*20];
	int sz;
	int pid;
	int i;
	unsigned char continuities[TRANSPORT_MAX_PIDS];
	struct section_buf *section_bufs[TRANSPORT_MAX_PIDS];
	struct transport_packet *tspkt;
	struct transport_values tsvals;

	if (argc != 2) {
		fprintf(stderr, "Syntax: testucsi <adapter id>\n");
		exit(1);
	}
	adapter = atoi(argv[1]);
	printf("Using adapter %i\n", adapter);

	// open devices
	if ((demuxfd = dvbdemux_open_demux(adapter, 0)) < 0) {
		perror("demux");
		exit(1);
	}
	if ((dvrfd = dvbdemux_open_dvr(adapter, 0, 0)) < 0) {
		perror("dvr");
		exit(1);
	}

	// setup filter to capture everything
	if (dvbdemux_set_pid_filter(demuxfd, -1, DVBDEMUX_INPUT_FRONTEND, DVBDEMUX_OUTPUT_DVR, 1)) {
		perror("set pid filter");
		exit(1);
	}

	// process the data
	memset(continuities, 0, sizeof(continuities));
	memset(section_bufs, 0, sizeof(section_bufs));
	while(1) {
		if ((sz = read(dvrfd, databuf, sizeof(databuf))) < 0) {
			perror("read error");
			exit(1);
		}
		for(i=0; i < sz; i+=TRANSPORT_PACKET_LENGTH) {
			// parse the transport packet
			tspkt = transport_packet_init(databuf + i);
			if (tspkt == NULL) {
				fprintf(stderr, "Bad sync byte\n");
				continue;
			}
			pid = transport_packet_pid(tspkt);
			if (transport_packet_values_extract(tspkt, &tsvals, 0)) {
				fprintf(stderr, "Bad packet received (pid:%04x)\n", pid);
				continue;
			}

			// check continuity
			if (transport_packet_continuity_check(tspkt,
			    tsvals.flags & transport_adaptation_flag_discontinuity,
			    continuities + pid)) {
				    fprintf(stderr, "Continuity error (pid:%04x)\n", pid);
				continuities[pid] = 0;
				continue;
			}

			// allocate section buf if we don't have one already
			if (section_bufs[pid] == NULL) {
				section_bufs[pid] = (struct section_buf*)
					malloc(sizeof(struct section_buf) + DVB_MAX_SECTION_BYTES);
				if (section_bufs[pid] == NULL) {
					fprintf(stderr, "Failed to allocate section buf (pid:%04x)\n", pid);
					exit(1);
				}
				section_buf_init(section_bufs[pid], DVB_MAX_SECTION_BYTES);
			}

			// FIXME: do stuff
		}
	}

	return 0;
}
