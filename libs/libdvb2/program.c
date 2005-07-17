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

#include <dvb/program.h>
#include <dvb/internal.h>
#include <dvb/list.h>

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>

#if 0
int dvb_create_program(struct dvb * card, struct dvb_program ** prg_ptr)
{
	if (card == NULL)
		return -EINVAL;

	struct dvb_program * prg = malloc(sizeof(struct dvb_program));
	if (prg == NULL)
		return -ENOMEM;

	memset(prg, 0, sizeof(struct dvb_program));
	prg->owner = card;

	list_add_tail(&card->programs, &prg->head);

	*prg_ptr = prg;
	return 0;
}

int dvb_remove_program(struct dvb_program * prg)
{
	if (prg == NULL || prg->owner == NULL)
		return -EINVAL;

	/* XXX: streams */

	list_del(&prg->head);
	free(prg);
	return 0;
}

int dvb_get_program_data(struct dvb_program_data * data,
			 const struct dvb_program * program)
{
	if (program == NULL || data == NULL)
		return -EINVAL;

	memcpy(data, &program->data, sizeof(struct dvb_program_data));
	return 0;
}

int dvb_put_program_data(dvb_program_t program,
		         const struct dvb_program_data * data)
{
	if (program == NULL || data == NULL)
		return -EINVAL;

	memcpy(&program->data, data, sizeof(struct dvb_program_data));
	return 0;
}
#endif

