#define USAGE \
"\n" \
"\n A tiny video watching application, just starts capturing /dev/video" \
"\n into /dev/fb0." \
"\n Be shure to have >8Bit/pixel color resolution and r/w access for " \
"\n /dev/video0, /dev/fb0 and /dev/tty0 to let this work..." \
"\n" \
"\n  compile with" \
"\n" \
"\n  $ gcc -g -Wall -O2 -o video video.c -I../../ost/include" \
"\n"

#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <linux/fb.h>
#include <linux/videodev.h>

#define VIDEO_DEV "/dev/video0"
#define FB_DEV "/dev/fb0"
#define VT_DEV "/dev/tty0"

static char *video_devname = VIDEO_DEV;

#define min(a,b)	(a) < (b) ? (a) : (b)

static int zero = 0;
static int one = 1;

static struct fb_var_screeninfo fb_var;
static struct fb_fix_screeninfo fb_fix;


int init_fb (void)
{
	const char blankoff_str[] = "\033[9;0]";
	int fd, vt_fd;

	fd = open (FB_DEV, O_RDWR);
	if (fd < 0) {
		perror("Could not open " FB_DEV ", please check permissions\n");
		return 1;
	}

	if ((vt_fd = open( VT_DEV, O_RDWR )) < 0) {
		perror("Could not open " VT_DEV ", please check permissions\n");
		return 1;
	}

	write( vt_fd, blankoff_str, strlen(blankoff_str) );

	if (ioctl (fd, FBIOGET_VSCREENINFO, &fb_var) < 0) {
		perror("Could not get variable screen information (fb_var)\n");
		return 1;
	}

	if (ioctl (fd, FBIOGET_FSCREENINFO, &fb_fix) < 0) {
		perror("Could not get fixed screen information (fb_fix)\n");
		return 1;
	}

	close (fd);
	return 0;
}


int init_video (int stop)
{
	int fd;
	struct video_capability vcap;

	if ((fd = open (video_devname, O_RDWR)) < 0) {
		fprintf (stderr,
			 "%s: Could not open %s, please check permissions\n",
			 __FUNCTION__, video_devname);
		return -1;
	}

	ioctl(fd, VIDIOCGCAP, &vcap);

	if (ioctl(fd, VIDIOCCAPTURE, &zero) < 0) {
		perror("Could not stop capturing (VIDIOCCAPTURE failed)\n");
		return -2;
	}

	if (stop)
		return 0;

	{
		struct video_buffer b;
		b.base = (void*) fb_fix.smem_start;
		b.width = fb_var.xres;
		b.height = fb_var.yres;
		b.depth = fb_var.bits_per_pixel;
		b.bytesperline = fb_var.xres*((fb_var.bits_per_pixel+7)/8);
		if (ioctl(fd, VIDIOCSFBUF, &b) < 0) {
			fprintf(stderr, "VIDIOCSFBUF failed, must run as root?\n");
			return -3;
		}
	}

	{
		struct video_picture p;
		if (ioctl(fd, VIDIOCGPICT, &p) < 0) {
			perror("VIDIOCGPICT failed\n");
			return -4;
		}
		p.depth = fb_var.bits_per_pixel;
		switch (fb_var.bits_per_pixel) {
			case 16:
				p.palette = VIDEO_PALETTE_RGB565;
				break;
			case 24:
				p.palette = VIDEO_PALETTE_RGB24;
				break;
			case 32:
				p.palette = VIDEO_PALETTE_RGB32;
				break;
		}
		//p.contrast = 0x8000;
		//p.colour = 0x6000;
		if (ioctl(fd, VIDIOCSPICT, &p) < 0) {
			perror("VIDIOCSPICT failed\n");
			return -5;
		}
	}

	{
		struct video_window win;
		win.width = min((__u32) vcap.maxwidth, fb_var.xres);
		win.height = min((__u32) vcap.maxheight, fb_var.yres);
		win.x = 0;
		win.y = 0;
		win.flags = 0;
		win.clips = NULL;
		win.clipcount = 0;
		win.chromakey = 0;
		if (ioctl(fd, VIDIOCSWIN, &win) < 0) {
			perror("VIDIOCSWIN failed\n");
			return -6;
		}
	}

	if (ioctl(fd, VIDIOCCAPTURE, &one) < 0) {
		perror("Could not start capturing (VIDIOCCAPTURE failed)\n");
		return -7;
	}

	close (fd);

	return 0;
}

int main (int argc, char **argv)
{
	int err = 0, stop = 0;

	if ((err = init_fb()))
		return err;

	if ((argc == 2 && strcmp(argv[1], "stop") == 0) ||
	    (argc == 3 && strcmp(argv[2], "stop") == 0))
		stop = 1;

	if ((argc == 2 && !stop) || argc == 3)
		video_devname = argv[1];

	if (argc != 1 && argc != 2 && !(argc == 3 && stop)) {
		fprintf(stderr, "usage: %s <devname> <stop>\n" USAGE, argv[0]);
		exit (-1);
	}

	return init_video (stop);
}
