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

#include <dvb/plugin.h>
#include <dvb/internal.h>

#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <dlfcn.h>

#include <stdio.h>

#define PLUGIN_DIR "plugins"

static struct dvb_plugin * plugins = NULL;

void register_plugin(struct dvb_plugin * plugin)
{
}

void deregister_plugin(struct dvb_plugin * plugin)
{
}

void __add_plugin(struct dvb_plugin * plugin)
{
	struct dvb_plugin * pos;

	if (plugins == NULL) {
		plugins = plugin;
		return;
	}

	for (pos = plugins; pos && pos->next; pos = pos->next) {
		if (pos == plugin)
			return;
	}

	pos->next = plugin;
}

int dvb_find_plugins(struct dvb * dvb, enum dvb_plugin_type type,
		     const struct dvb_plugin ** first)
{
	DIR * dir;
	struct dirent * e;
	*first = NULL;

	dir = opendir(PLUGIN_DIR);
	if (dir == NULL)
		return -errno;

	while ((e = readdir(dir))) {
		int len = strlen(e->d_name);
		char name[1024];
		void * handle;
		struct dvb_plugin * plugin;

		if (len < 3 || strcmp(e->d_name + len - 3, ".so"))
			continue;

		snprintf(name, sizeof(name), "%s/%s", PLUGIN_DIR, e->d_name);
	
		handle = dlopen(name, RTLD_NOW);
		if (handle == NULL) {
			fprintf(stderr, "dlopen: %s\n", dlerror());
			continue;
		}

		plugin = dlsym(handle, "dvb_plugin");
		if (plugin != NULL &&
		    strncmp(plugin->magic, "libdvb2",
			    sizeof(*plugin->magic)) == 0) {
			__add_plugin(plugin);
			/* XXX - this makes all plugins resident */
			continue;
		}

		dlclose(handle);
	}

	closedir(dir);

	*first = plugins;
	return 0;
}

