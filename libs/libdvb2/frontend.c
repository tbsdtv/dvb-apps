/*
 * libdvb2 - dvb helper and wrapper library
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
 * Copyright (C) 2005 Andrew de Quincey (adq_dvb@lidskialf.net)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#include <dvb/frontend.h>
#include <dvb/internal.h>
#include <dvb/thread.h>
#include <dvb/event.h>
#include <dvb/device.h>

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <linux/dvb/frontend.h>

#include <stdio.h>
static void update_frontend(struct dvb_task * task)
{
	struct dvb_frontend * frontend = dvb_get_task_opaque(task);
	int delay = frontend->readonly ? 250000 : 100000;

	dvb_update_frontend(frontend);

	dvb_schedule_task(frontend->update_task, delay);
}

static inline int frontend_simple_event(struct dvb_frontend * frontend,
		                        enum dvb_frontend_event_type type)
{
	struct dvb_event * event;
	struct dvb_frontend_event_data * ed;

	event = dvb_alloc_event(dvb_frontend_event,
			        sizeof(struct dvb_frontend_event_data));
	if (event == NULL)
		return -ENOMEM;

	ed = dvb_get_event_data(event);
	ed->frontend = frontend;
	ed->type = type;

	dvb_enqueue_event(frontend->adapter->dvb, event);

	return 0;
}

#include <stdio.h>
int dvb_probe_frontend(struct dvb_adapter * adapter, int num,
		       struct dvb_frontend ** ptr)
{
	struct dvb_frontend * frontend;
	struct dvb_frontend_info info;
	int fd;

	if ((fd = open(dvb_devname(DVB_FRONTEND, adapter->num, num),
		       O_RDONLY | O_NONBLOCK)) < 0)
		return -errno;

	if (ioctl(fd, FE_GET_INFO, &info) < 0) {
		close(fd);
		return -errno;
	}

	frontend = malloc(sizeof(struct dvb_frontend));
	if (frontend == NULL) {
		close(fd);
		return -ENOMEM;
	}

	memset(frontend, 0, sizeof(struct dvb_frontend));

	LIST_ENTRY_INIT(&frontend->demux);
	LIST_ENTRY_INIT(&frontend->ci);

	frontend->adapter = adapter;
	frontend->num = num;
	frontend->fd = fd;
	frontend->name = strdup(info.name);
	frontend->readonly = 1;
	frontend->lock = 0;

	if (dvb_alloc_task(adapter->dvb, &frontend->update_task)) {
		close(frontend->fd);
		free(frontend);
		return -ENOMEM;
	}

	list_append_entry(&adapter->frontends, &frontend->list);

	if (ptr)
		*ptr = frontend;

	frontend_simple_event(frontend, dvb_frontend_open_event);

	dvb_set_task_func(frontend->update_task, update_frontend);
	dvb_set_task_opaque(frontend->update_task, frontend);

	dvb_schedule_task(frontend->update_task, 1);
	return 0;
}

struct dvb_frontend * dvb_find_frontend(struct dvb_adapter * adapter, int num)
{
	struct list_entry * pos;
	struct dvb_frontend * frontend;

	for (pos = adapter->frontends.next; pos != NULL; pos = pos->next) {
		frontend = list_get_entry(pos, struct dvb_frontend, list);
		if (frontend->num == num)
			return frontend;
	}

	return NULL;
}

int dvb_close_frontends(struct dvb_adapter * adapter)
{
	struct list_entry * pos, * next;
	struct dvb_frontend * frontend;

	for (pos = adapter->frontends.next; pos != NULL; pos = next) {
		next = pos->next;
		frontend = list_get_entry(pos, struct dvb_frontend, list);

		frontend_simple_event(frontend, dvb_frontend_close_event);

		// XXX: callback for when the event is handled?
		list_remove_entry(&adapter->frontends, pos);
		close(frontend->fd);
		free(frontend->name);
		free(frontend);
	}

	return 0;
}

int dvb_update_frontend(struct dvb_frontend * frontend)
{
	struct dvb_frontend_event event;
	int ret;
	int lock = frontend->lock;

	if (!frontend->readonly) {
		if ((ret = ioctl(frontend->fd,
		     FE_GET_EVENT, &event)) < 0)
			return ret;
	}
	if ((ret = ioctl(frontend->fd,
				FE_READ_STATUS, &event.status)) < 0)
		return ret;

	if ((ret = ioctl(frontend->fd,
				FE_GET_FRONTEND, &event.parameters)) < 0)
		return ret;

	frontend->lock = (event.status & FE_HAS_LOCK) > 0;

	if (frontend->lock != lock) {
		frontend_simple_event(frontend, dvb_frontend_tune_event);
	}

	return 0;
}

int dvb_update_frontends(struct dvb_adapter * adapter)
{
	struct list_entry * pos;
	struct dvb_frontend * frontend;
	int ret;

	if (adapter == NULL)
		return -EINVAL;

	for (pos = adapter->frontends.next; pos != NULL; pos = pos->next) {
		frontend = list_get_entry(pos, struct dvb_frontend, list);

		if ((ret = dvb_update_frontend(frontend)) < 0)
			return ret;
	}

	return 0;
}

int dvb_open_frontend(struct dvb_frontend * frontend)
{
	const char * dev;
	int ret;

	if (!frontend->readonly)
		return 0;

	dev = dvb_devname(DVB_FRONTEND, frontend->adapter->num, frontend->num);

	if ((ret = open(dev, O_RDWR | O_NONBLOCK)) < 0)
		return -errno;

	close(frontend->fd);
	frontend->fd = ret;
	frontend->readonly = 0;

	return 0;
}

int dvb_close_frontend(struct dvb_frontend * frontend)
{
	const char * dev;
	int ret;

	if (frontend->readonly)
		return 0;

	dev = dvb_devname(DVB_FRONTEND, frontend->adapter->num, frontend->num);

	if ((ret = open(dev, O_RDONLY | O_NONBLOCK)) < 0)
		return -errno;

	close(frontend->fd);
	frontend->fd = ret;
	frontend->readonly = 1;

	return 0;
}

int dvb_frontend_locked(struct dvb_frontend * frontend)
{
	return frontend ? frontend->lock : -EINVAL;
}

int dvb_frontend_readonly(struct dvb_frontend * frontend)
{
	return frontend ? frontend->readonly : -EINVAL;
}

int dvb_get_frontend_event_data(struct dvb_event * event,
		                struct dvb_frontend_event_data * data)
{
	if (event == NULL || data == NULL)
		return -EINVAL;

	if (dvb_get_event_type(event)      != dvb_frontend_event ||
	    dvb_get_event_data_size(event) != sizeof(struct dvb_frontend_event_data))
		return -EINVAL;

	memcpy(data, dvb_get_event_data(event),
	       sizeof(struct dvb_frontend_event_data));

	return 0;
}

int dvb_frontend_read_value(struct dvb_frontend * frontend, int value_flags,
			    struct dvb_frontend_values * dest)
{
	int result = 0;
	fe_status_t status;

	if (value_flags & DVB_FRONTEND_VALUE_STATUS) {
		if (!ioctl(frontend->fd, FE_READ_STATUS, &status)) {
			result |= DVB_FRONTEND_VALUE_STATUS;
			if (status & FE_HAS_SIGNAL)
				dest->signal = 1;
			else
				dest->signal = 0;

			if (status & FE_HAS_CARRIER)
				dest->carrier = 1;
			else
				dest->carrier = 0;

			if (status & FE_HAS_VITERBI)
				dest->viterbi = 1;
			else
				dest->viterbi = 0;

			if (status & FE_HAS_SYNC)
				dest->sync = 1;
			else
				dest->sync = 0;

			if (status & FE_HAS_LOCK)
				dest->lock = 1;
			else
				dest->lock = 0;
		}
	}
	if (value_flags & DVB_FRONTEND_VALUE_BER) {
		if (!ioctl(frontend->fd, FE_READ_BER, &dest->ber))
			result |= DVB_FRONTEND_VALUE_BER;
	}
	if (value_flags & DVB_FRONTEND_VALUE_SIGNAL_STRENGTH) {
		if (!ioctl(frontend->fd, FE_READ_SIGNAL_STRENGTH, &dest->signal_strength))
			result |= DVB_FRONTEND_VALUE_SIGNAL_STRENGTH;
	}
	if (value_flags & DVB_FRONTEND_VALUE_SNR) {
		if (!ioctl(frontend->fd, FE_READ_SNR, &dest->snr))
			result |= DVB_FRONTEND_VALUE_SNR;
	}
	if (value_flags & DVB_FRONTEND_VALUE_UNCORRECTED_BLOCKS) {
		if (!ioctl(frontend->fd, FE_READ_UNCORRECTED_BLOCKS, &dest->ucblocks))
			result |= DVB_FRONTEND_VALUE_UNCORRECTED_BLOCKS;
	}

	return result;
}

int dvb_frontend_lnb_command(struct dvb_frontend * frontend, char * lnbcommand)
{
	int i = 0;
	int waittime;
	int status;
	struct dvb_diseqc_master_cmd master_cmd;
	unsigned int tmpcmd[6];

	if (frontend->readonly)
		return -EINVAL;

	while(lnbcommand[i]) {
		/* kill whitespace */
		if (isspace(lnbcommand[i])) {
			i++;
			continue;
		}

		switch(lnbcommand[i]) {
			case 't':
				if ((status = ioctl(frontend->fd, FE_SET_TONE, SEC_TONE_OFF)) != 0)
					return status;
				break;

			case 'T':
				if ((status = ioctl(frontend->fd, FE_SET_TONE, SEC_TONE_ON)) != 0)
					return status;
				break;

			case '_':
				if ((status = ioctl(frontend->fd, FE_SET_VOLTAGE, SEC_VOLTAGE_OFF)) != 0)
					return status;
				break;

			case 'v':
				if ((status = ioctl(frontend->fd, FE_SET_VOLTAGE, SEC_VOLTAGE_13)) != 0)
					return status;
				break;

			case 'V':
				if ((status = ioctl(frontend->fd, FE_SET_VOLTAGE, SEC_VOLTAGE_18)) != 0)
					return status;
				break;

			case 'A':
				if ((status = ioctl(frontend->fd, FE_DISEQC_SEND_BURST, SEC_MINI_A)) != 0)
					return status;
				break;

			case 'B':
				if ((status = ioctl(frontend->fd, FE_DISEQC_SEND_BURST, SEC_MINI_B)) != 0)
					return status;
				break;

			case '+':
				ioctl(frontend->fd, FE_ENABLE_HIGH_LNB_VOLTAGE, 1);
				/* don't care if this one is not supported */
				break;

			case '-':
				ioctl(frontend->fd, FE_ENABLE_HIGH_LNB_VOLTAGE, 0);
				/* don't care if this one is not supported */
				break;

			case 'W':
				waittime = atoi(lnbcommand + i + 1);
				if (waittime == 0) {
					return -EINVAL;
				}
				usleep(waittime * 1000);
				while(lnbcommand[i] && !isspace(lnbcommand[i]))
					i++;
				break;

			case '[':
				master_cmd.msg_len = sscanf(lnbcommand+i+1, "%x %x %x %x %x %x",
						tmpcmd, tmpcmd+1, tmpcmd+2, tmpcmd+3, tmpcmd+4, tmpcmd+5);
				if (master_cmd.msg_len == 0)
					return -EINVAL;
				master_cmd.msg[0] = tmpcmd[0];
				master_cmd.msg[1] = tmpcmd[1];
				master_cmd.msg[2] = tmpcmd[2];
				master_cmd.msg[3] = tmpcmd[3];
				master_cmd.msg[4] = tmpcmd[4];
				master_cmd.msg[5] = tmpcmd[5];

				if ((status = ioctl(frontend->fd, FE_DISEQC_SEND_MASTER_CMD, &master_cmd)) != 0)
					return status;

				while(lnbcommand[i] && (lnbcommand[i] != ']'))
					i++;
				break;

			default:
				return -EINVAL;
		}

		i++;
	}

	return 0;
}

int dvb_frontend_read_diseqc_response(struct dvb_frontend * frontend,
				      int timeout, uint8_t * buf,
				      unsigned int len)
{
	struct dvb_diseqc_slave_reply reply;
	int result;

	if (len > 4)
		len = 4;

	reply.timeout = timeout;
	reply.msg_len = len;

	if ((result = ioctl(frontend->fd, FE_DISEQC_RECV_SLAVE_REPLY, reply)) != 0)
		return result;

	if (reply.msg_len < len)
		len = reply.msg_len;
	memcpy(buf, reply.msg, len);

	return len;
}

int dvb_frontend_get(struct dvb_frontend * frontend,
		     struct dvb_frontend_parameters * params)
{
	return ioctl(frontend->fd, FE_GET_FRONTEND, params);
}

int dvb_frontend_set(struct dvb_frontend * frontend,
		     struct dvb_frontend_parameters * params)
{
	if (frontend->readonly)
		return -EINVAL;

	return ioctl(frontend->fd, FE_SET_FRONTEND, params);
}
