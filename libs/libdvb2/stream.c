/*
 * libdvb2 - dvb helper and wrapper library
 *
 * Copyright (C) 2005 Kenneth Aafloy (kenneth@linuxtv.org)
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

#include <dvb/stream.h>
#include <dvb/program.h>
#include <dvb/list.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>

#if 0
struct dvb_stream {
	struct list_entry list;

	struct dvb_program * program;
	struct dvb_stream_data * data;
};

dvb_program_t program_of_stream(dvb_stream_t stream)
{
	return stream ? stream->program : NULL;
}

int dvb_create_stream(dvb_program_t program, dvb_stream_t * streamptr)
{
	if (program == NULL || streamptr == NULL)
		return -EINVAL;

	struct dvb_stream * stream = malloc(sizeof(struct dvb_stream));
	if (stream == NULL)
		return -ENOMEM;

	memset(stream, 0, sizeof(struct dvb_stream));
	stream->program = program;

	list_append_entry(&program->streams, &stream->list);
	*streamptr = stream;

	return 0;
}

int dvb_remove_stream(dvb_stream_t stream)
{
	if (stream == NULL)
		return -EINVAL;

	list_remove_entry(&stream->program->streams, &stream->list);
	return 0;
}

int get_program_streams(dvb_program_t program, dvb_stream_t * streams)
{
	if (program->streams.next == NULL)
		return 0;
	return 0;
}
#endif

