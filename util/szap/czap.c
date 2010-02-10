#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>

#include <linux/dvb/frontend.h>
#include <linux/dvb/dmx.h>

#include "util.h"


static char FRONTEND_DEV [80];
static char DEMUX_DEV [80];
static int exit_after_tuning;

#define CHANNEL_FILE "channels.conf"

#define ERROR(x...)							\
	do {								\
		fprintf(stderr, "ERROR: ");				\
		fprintf(stderr, x);					\
		fprintf (stderr, "\n");					\
	} while (0)

#define PERROR(x...)							\
	do {								\
		fprintf(stderr, "ERROR: ");				\
		fprintf(stderr, x);					\
		fprintf (stderr, " (%s)\n", strerror(errno));		\
	} while (0)


typedef struct {
	char *name;
	int value;
} Param;

static const Param inversion_list[] = {
	{ "INVERSION_OFF", INVERSION_OFF },
	{ "INVERSION_ON", INVERSION_ON },
	{ "INVERSION_AUTO", INVERSION_AUTO }
};

static const Param fec_list[] = {
	{ "FEC_1_2", FEC_1_2 },
	{ "FEC_2_3", FEC_2_3 },
	{ "FEC_3_4", FEC_3_4 },
	{ "FEC_4_5", FEC_4_5 },
	{ "FEC_5_6", FEC_5_6 },
	{ "FEC_6_7", FEC_6_7 },
	{ "FEC_7_8", FEC_7_8 },
	{ "FEC_8_9", FEC_8_9 },
	{ "FEC_AUTO", FEC_AUTO },
	{ "FEC_NONE", FEC_NONE }
};

static const Param modulation_list[] = {
	{ "QAM_16", QAM_16 },
	{ "QAM_32", QAM_32 },
	{ "QAM_64", QAM_64 },
	{ "QAM_128", QAM_128 },
	{ "QAM_256", QAM_256 },
	{ "QAM_AUTO", QAM_AUTO }
};

#define LIST_SIZE(x) sizeof(x)/sizeof(Param)


static
int parse_param(const char *val, const Param * plist, int list_size, int *ok)
{
	int i;

	for (i = 0; i < list_size; i++) {
		if (strcasecmp(plist[i].name, val) == 0) {
			*ok = 1;
			return plist[i].value;
		}
	}
	*ok = 0;
	return -1;
}


static char line_buf[256];
static
char *find_channel(FILE *f, int list_channels, int *chan_no, const char *channel)
{
	size_t l;
	int lno = 0;

	l = channel ? strlen(channel) : 0;
	while (!feof(f)) {
		if (!fgets(line_buf, sizeof(line_buf), f))
			return NULL;
		lno++;
		if (list_channels) {
			printf("%3d %s", lno, line_buf);
		}
		else if (*chan_no) {
			if (*chan_no == lno)
				return line_buf;
		}
		else if ((strncasecmp(channel, line_buf, l) == 0)
				&& (line_buf[l] == ':')) {
			*chan_no = lno;
			return line_buf;
		}
	};

	return NULL;
}


int parse(const char *fname, int list_channels, int chan_no, const char *channel,
	  struct dvb_frontend_parameters *frontend, int *vpid, int *apid)
{
	FILE *f;
	char *chan;
	char *name, *inv, *fec, *mod;
	int ok;

	if ((f = fopen(fname, "r")) == NULL) {
		PERROR("could not open file '%s'", fname);
		return -1;
	}

	chan = find_channel(f, list_channels, &chan_no, channel);
	fclose(f);
	if (list_channels)
		return 0;
	if (!chan) {
		ERROR("could not find channel '%s' in channel list",
		      channel);
		return -2;
	}
	printf("%3d %s", chan_no, chan);

	if ((sscanf(chan, "%a[^:]:%d:%a[^:]:%d:%a[^:]:%a[^:]:%d:%d\n",
				&name, &frontend->frequency,
				&inv, &frontend->u.qam.symbol_rate,
				&fec, &mod, vpid, apid) != 8)
			|| !name || !inv || !fec | !mod) {
		ERROR("cannot parse service data");
		return -3;
	}
	frontend->inversion = parse_param(inv, inversion_list, LIST_SIZE(inversion_list), &ok);
	if (!ok) {
		ERROR("inversion field syntax '%s'", inv);
		return -4;
	}
	frontend->u.qam.fec_inner = parse_param(fec, fec_list, LIST_SIZE(fec_list), &ok);
	if (!ok) {
		ERROR("FEC field syntax '%s'", fec);
		return -5;
	}
	frontend->u.qam.modulation = parse_param(mod, modulation_list,
			LIST_SIZE(modulation_list), &ok);
	if (!ok) {
		ERROR("modulation field syntax '%s'", mod);
		return -6;
	}
	printf("%3d %s: f %d, s %d, i %d, fec %d, qam %d, v %#x, a %#x\n",
			chan_no, name, frontend->frequency, frontend->u.qam.symbol_rate,
			frontend->inversion, frontend->u.qam.fec_inner,
			frontend->u.qam.modulation, *vpid, *apid);
	free(name);
	free(inv);
	free(fec);
	free(mod);

	return 0;
}


static
int setup_frontend(int fe_fd, struct dvb_frontend_parameters *frontend)
{
	struct dvb_frontend_info fe_info;

	if (ioctl(fe_fd, FE_GET_INFO, &fe_info) < 0) {
		PERROR ("ioctl FE_GET_INFO failed");
		return -1;
	}

	if (fe_info.type != FE_QAM) {
		ERROR ("frontend device is not a QAM (DVB-C) device");
		return -1;
	}

	if (ioctl(fe_fd, FE_SET_FRONTEND, frontend) < 0) {
		PERROR ("ioctl FE_SET_FRONTEND failed");
		return -1;
	}

	return 0;
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
			printf ("status %02x | signal %3u%% | snr %3u%% | ber %d | unc %d | ",
				status, (signal * 100) / 0xffff, (snr * 100) / 0xffff, ber, uncorrected_blocks);
		} else {
			printf ("status %02x | signal %04x | snr %04x | ber %08x | unc %08x | ",
				status, signal, snr, ber, uncorrected_blocks);
		}

		if (status & FE_HAS_LOCK)
			printf("FE_HAS_LOCK");

		usleep(1000000);

		printf("\n");

		if (exit_after_tuning && (status & FE_HAS_LOCK))
			break;
	} while (1);

	return 0;
}


static const char *usage = "\nusage: %s [-a adapter_num] [-f frontend_id] [-d demux_id] [-c conf_file] [ -H ] {<channel name>| -n channel_num} [-x]\n"
	"   or: %s [-c conf_file]  -l\n\n";


int main(int argc, char **argv)
{
	struct dvb_frontend_parameters frontend_param;
	char *homedir = getenv("HOME");
	char *confname = NULL;
	char *channel = NULL;
	int adapter = 0, frontend = 0, demux = 0, dvr = 0;
	int vpid, apid;
	int frontend_fd, video_fd, audio_fd;
	int opt, list_channels = 0, chan_no = 0;
	int human_readable = 0;

	while ((opt = getopt(argc, argv, "Hln:hrn:a:f:d:c:x")) != -1) {
		switch (opt) {
		case 'a':
			adapter = strtoul(optarg, NULL, 0);
			break;
		case 'f':
			frontend = strtoul(optarg, NULL, 0);
			break;
		case 'd':
			demux = strtoul(optarg, NULL, 0);
			break;
		case 'r':
			dvr = 1;
			break;
		case 'l':
			list_channels = 1;
			break;
		case 'n':
			chan_no = strtoul(optarg, NULL, 0);
			break;
		case 'x':
			exit_after_tuning = 1;
			break;
		case 'H':
			human_readable = 1;
			break;
		case 'c':
			confname = optarg;
			break;
		case '?':
		case 'h':
		default:
			fprintf (stderr, usage, argv[0], argv[0]);
			return -1;
		};
	}

	if (optind < argc)
		channel = argv[optind];

	if (!channel && chan_no <= 0 && !list_channels) {
		fprintf (stderr, usage, argv[0], argv[0]);
		return -1;
	}

	if (!homedir)
		ERROR("$HOME not set");

	snprintf (FRONTEND_DEV, sizeof(FRONTEND_DEV),
		  "/dev/dvb/adapter%i/frontend%i", adapter, frontend);

	snprintf (DEMUX_DEV, sizeof(DEMUX_DEV),
		  "/dev/dvb/adapter%i/demux%i", adapter, demux);

	printf ("using '%s' and '%s'\n", FRONTEND_DEV, DEMUX_DEV);

	if (!confname)
	{
		int len = strlen(homedir) + strlen(CHANNEL_FILE) + 18;
		if (!homedir)
			ERROR("$HOME not set");
		confname = malloc(len);
		snprintf(confname, len, "%s/.czap/%i/%s",
			 homedir, adapter, CHANNEL_FILE);
		if (access(confname, R_OK))
			snprintf(confname, len, "%s/.czap/%s",
				 homedir, CHANNEL_FILE);
	}
	printf("reading channels from file '%s'\n", confname);

	memset(&frontend_param, 0, sizeof(struct dvb_frontend_parameters));

	if (parse(confname, list_channels, chan_no, channel, &frontend_param, &vpid, &apid))
		return -1;
	if (list_channels)
		return 0;

	if ((frontend_fd = open(FRONTEND_DEV, O_RDWR)) < 0) {
		PERROR("failed opening '%s'", FRONTEND_DEV);
		return -1;
	}

	if (setup_frontend(frontend_fd, &frontend_param) < 0)
		return -1;

	if ((video_fd = open(DEMUX_DEV, O_RDWR)) < 0) {
		PERROR("failed opening '%s'", DEMUX_DEV);
		return -1;
	}

	if (set_pesfilter (video_fd, vpid, DMX_PES_VIDEO, dvr) < 0)
		return -1;

	if ((audio_fd = open(DEMUX_DEV, O_RDWR)) < 0) {
		PERROR("failed opening '%s'", DEMUX_DEV);
		return -1;
	}

	if (set_pesfilter (audio_fd, apid, DMX_PES_AUDIO, dvr) < 0)
		return -1;

	check_frontend (frontend_fd, human_readable);

	close (audio_fd);
	close (video_fd);
	close (frontend_fd);

	return 0;
}
