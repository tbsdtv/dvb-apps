/*
 * test_av_play.c - Test playing an MPEG A+V PES (e.g. VDR recordings) from a file.
 *
 * Copyright (C) 2000 Ralph  Metzler <ralph@convergence.de>
 *                  & Marcus Metzler <marcus@convergence.de>
 *                    for convergence integrated media GmbH
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>

#include <linux/dvb/dmx.h>
#include <linux/dvb/video.h>
#include <linux/dvb/audio.h>
#include <sys/poll.h>

static int audioPlay(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_PLAY) < 0)){
		perror("AUDIO PLAY: ");
		return -1;
	}

	return 0;
}


static int audioSelectSource(int fd, audio_stream_source_t source)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_SELECT_SOURCE, source) < 0)){
		perror("AUDIO SELECT SOURCE: ");
		return -1;
	}

	return 0;
}



static int audioSetMute(int fd, int state)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_SET_MUTE, state) < 0)){
		perror("AUDIO SET MUTE: ");
		return -1;
	}

	return 0;
}

static int audioSetAVSync(int fd, int state)
{
	int ans;

	if ( (ans = ioctl(fd,AUDIO_SET_AV_SYNC, state) < 0)){
		perror("AUDIO SET AV SYNC: ");
		return -1;
	}

	return 0;
}

static int videoStop(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_STOP,0) < 0)){
		perror("VIDEO STOP: ");
		return -1;
	}

	return 0;
}

static int videoPlay(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_PLAY) < 0)){
		perror("VIDEO PLAY: ");
		return -1;
	}

	return 0;
}


static int videoFreeze(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_FREEZE) < 0)){
		perror("VIDEO FREEZE: ");
		return -1;
	}

	return 0;
}


static int videoContinue(int fd)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_CONTINUE) < 0)){
		perror("VIDEO CONTINUE: ");
		return -1;
	}

	return 0;
}

static int videoSelectSource(int fd, video_stream_source_t source)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_SELECT_SOURCE, source) < 0)){
		perror("VIDEO SELECT SOURCE: ");
		return -1;
	}

	return 0;
}


static int videoFastForward(int fd,int nframes)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_FAST_FORWARD, nframes) < 0)){
		perror("VIDEO FAST FORWARD: ");
		return -1;
	}

	return 0;
}

static int videoSlowMotion(int fd,int nframes)
{
	int ans;

	if ( (ans = ioctl(fd,VIDEO_SLOWMOTION, nframes) < 0)){
		perror("VIDEO SLOWMOTION: ");
		return -1;
	}

	return 0;
}

#define BUFFY 32768
#define NFD   3
static void play_file_av(int filefd, int vfd, int afd)
{
	char buf[BUFFY];
	int count;
	int written;
	struct pollfd pfd[NFD];
	int stopped = 0;

	pfd[0].fd = STDIN_FILENO;
	pfd[0].events = POLLIN;

	pfd[1].fd = vfd;
	pfd[1].events = POLLOUT;

	pfd[2].fd = afd;
	pfd[2].events = POLLOUT;

	videoSelectSource(vfd,VIDEO_SOURCE_MEMORY);
	audioSelectSource(afd,AUDIO_SOURCE_MEMORY);

	// FIXME: only seems to work if starting audio first!
	audioPlay(afd);
	videoPlay(vfd);

	count = read(filefd,buf,BUFFY);
	write(vfd,buf,count);

	while ( (count = read(filefd,buf,BUFFY)) >= 0  ){
		written = 0;
		while(written < count){
			if (poll(pfd,NFD,1)){
				if (pfd[1].revents & POLLOUT){
					written += write(vfd,buf+written,
							count-written);
				}
				if (pfd[0].revents & POLLIN){
					int c = getchar();
					switch(c){
					case 'z':
						videoFreeze(vfd);
						printf("playback frozen\n");
						stopped = 1;
						break;

					case 's':
						videoStop(vfd);
						printf("playback stopped\n");
						stopped = 1;
						break;

					case 'c':
						videoContinue(vfd);
						printf("playback continued\n");
						stopped = 0;
						break;

					case 'p':
						videoPlay(vfd);
						audioPlay(afd);
					        audioSetAVSync(afd, 1);
						audioSetMute(afd, 0);
						printf("playback started\n");
						stopped = 0;
						break;

					case 'f':
					        audioSetAVSync(afd, 0);
						audioSetMute(afd, 1);
						videoFastForward(vfd,0);
						printf("fastforward\n");
						stopped = 0;
						break;

					case 'm':
					        audioSetAVSync(afd, 0);
						audioSetMute(afd, 1);
						videoSlowMotion(vfd,2);
						printf("slowmotion\n");
						stopped = 0;
						break;

					case 'q':
						videoContinue(vfd);
						exit(0);
						break;
					}
				}
			}
		}
	}
}

int main(int argc, char **argv)
{
	int vfd, afd;
	int filefd;
	char *videodev = "/dev/dvb/adapter0/video0";
	char *audiodev = "/dev/dvb/adapter0/audio0";

	if (argc < 2) {
		fprintf(stderr, "usage: test_av_play mpeg_A+V_PES_file\n");
		return 1;
	}

	if (getenv("VIDEO"))
		videodev = getenv("VIDEO");
	if (getenv("AUDIO"))
		videodev = getenv("AUDIO");

	printf("using video device '%s'\n", videodev);
	printf("using audio device '%s'\n", audiodev);

	if ( (filefd = open(argv[1],O_RDONLY)) < 0){
		perror("File open:");
		return -1;
	}
	if ((vfd = open(videodev,O_RDWR|O_NONBLOCK)) < 0){
		perror("VIDEO DEVICE: ");
		return -1;
	}
	if ((afd = open(audiodev,O_RDWR|O_NONBLOCK)) < 0){
		perror("AUDIO DEVICE: ");
		return -1;
	}
	play_file_av(filefd, vfd, afd);
	close(vfd);
	close(afd);
	close(filefd);
	return 0;


}

