/**
 * dvbcfg_diseqc configuration file support.
 *
 * Copyright (c) 2005 by Andrew de Quincey <adq_dvb@lidskialf.net>
 *
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#include <stdio.h>
#include <errno.h>
#include "dvbcfg_diseqc.h"
#include "dvbcfg_common.h"
#include "dvbcfg_util.h"

int dvbcfg_diseqc_load(char *config_file, struct dvbcfg_diseqc **diseqcs)
{
	FILE *in;
	char curline[256];
	char *linepos;
	char *source_id;
	struct dvbcfg_diseqc *curdiseqc;
	struct dvbcfg_diseqc *tail;
	struct dvbcfg_diseqc_entry tmpentry;
	struct dvbcfg_diseqc_entry *newentry;
	struct dvbcfg_diseqc_entry *curentry;
	int numtokens;
	int error = 0;

	/* open the file */
	in = fopen(config_file, "r");
	if (in == NULL)
		return errno;

	/* move to the tail entry */
	tail = *diseqcs;
	if (tail)
		while (tail->next)
			tail = tail->next;

	while (fgets(curline, sizeof(curline), in)) {
		linepos = curline;
		memset(&tmpentry, 0, sizeof(struct dvbcfg_diseqc_entry));

		/* clean any comments/ whitespace */
		if (dvbcfg_cleanline(linepos) == 0)
			continue;

		/* tokenise the line */
		numtokens = dvbcfg_tokenise(linepos, " \t", 4, 1);
		if ((numtokens < 4) || (numtokens > 5))
			continue;

		/* the source id */
		source_id = linepos;
		linepos = dvbcfg_nexttoken(linepos);

		/* find/create the diseqc */
		curdiseqc = dvbcfg_diseqc_find(*diseqcs, source_id);
		if (curdiseqc == NULL) {
			curdiseqc =
			    (struct dvbcfg_diseqc *)
			    malloc(sizeof(struct dvbcfg_diseqc));
			if (curdiseqc == NULL) {
				error = -ENOMEM;
				break;
			}
			memset(curdiseqc, 0, sizeof(struct dvbcfg_diseqc));
			curdiseqc->source_id =
			    dvbcfg_strdupandtrim(source_id);
			if (tail)
				tail->next = curdiseqc;
			if (!*diseqcs)
				*diseqcs = curdiseqc;
			curdiseqc->prev = tail;
			tail = curdiseqc;
		}

		/* the SLOF */
		if (sscanf(linepos, "%d", &tmpentry.slof) != 1)
			continue;
		tmpentry.slof *= 1000;	// want it in kHz
		linepos = dvbcfg_nexttoken(linepos);

		/* the polarization */
		if (linepos[0] == 'H')
			tmpentry.polarization = DVBCFG_POLARIZATION_H;
		else if (linepos[0] == 'V')
			tmpentry.polarization = DVBCFG_POLARIZATION_V;
		else if (linepos[0] == 'L')
			tmpentry.polarization = DVBCFG_POLARIZATION_L;
		else if (linepos[0] == 'R')
			tmpentry.polarization = DVBCFG_POLARIZATION_R;
		else
			continue;
		linepos = dvbcfg_nexttoken(linepos);

		/* LOF */
		if (sscanf(linepos, "%d", &tmpentry.lof) != 1)
			continue;
		tmpentry.lof *= 1000;	// want it in kHz
		linepos = dvbcfg_nexttoken(linepos);

		/* command */
		if (numtokens == 5)
			tmpentry.command = linepos;
		else
			tmpentry.command = "";

		/* create new dvbcfg_diseqc_entry */
		newentry =
		    (struct dvbcfg_diseqc_entry *)
		    malloc(sizeof(struct dvbcfg_diseqc_entry));
		if (newentry == NULL) {
			error = -ENOMEM;
			break;
		}
		memcpy(newentry, &tmpentry,
		       sizeof(struct dvbcfg_diseqc_entry));
		newentry->command = dvbcfg_strdupandtrim(tmpentry.command);
		if (!newentry->command) {
			if (newentry->command)
				free(newentry->command);
			free(newentry);
			error = -ENOMEM;
			break;
		}

		/* link it in */
		curentry = curdiseqc->entries;
		if (!curentry) {
			curdiseqc->entries = curentry;
		} else {
			while (curentry->next)
				curentry = curentry->next;

			newentry->prev = curentry;
			curentry->next = newentry;
		}
	}

	/* tidy up and return */
	if (error) {
		dvbcfg_diseqc_free_all(*diseqcs);
		*diseqcs = NULL;
	}
	fclose(in);
	return error;
}

int dvbcfg_diseqc_save(char *config_file, struct dvbcfg_diseqc *diseqcs)
{
	FILE *out;
	char polarization;
	struct dvbcfg_diseqc_entry *entry;

	/* open the file */
	out = fopen(config_file, "w");
	if (out == NULL)
		return errno;

	while (diseqcs) {

		entry = diseqcs->entries;
		while (entry) {
			switch (entry->polarization) {
			case DVBCFG_POLARIZATION_H:
				polarization = 'H';
				break;

			case DVBCFG_POLARIZATION_V:
				polarization = 'V';
				break;

			case DVBCFG_POLARIZATION_L:
				polarization = 'L';
				break;

			case DVBCFG_POLARIZATION_R:
				polarization = 'R';
				break;
			}

			fprintf(out, "%s %d %c %d %s\n",
				diseqcs->source_id, entry->slof / 1000,
				polarization, entry->lof / 1000,
				entry->command);

			entry = entry->next;
		}

		diseqcs = diseqcs->next;
	}

	fclose(out);
	return 0;
}

struct dvbcfg_diseqc *dvbcfg_diseqc_find(struct dvbcfg_diseqc *diseqcs,
					 char *source_id)
{
	while (diseqcs) {
		if (!strcmp(source_id, diseqcs->source_id))
			return diseqcs;

		diseqcs = diseqcs->next;
	}

	return NULL;
}

struct dvbcfg_diseqc_entry *dvbcfg_diseqc_find_entry(struct dvbcfg_diseqc
						     *diseqc,
						     uint32_t frequency,
						     int polarization)
{
	struct dvbcfg_diseqc_entry *entry;
	struct dvbcfg_diseqc_entry *candidate = NULL;

	entry = diseqc->entries;
	while (entry) {
		if ((polarization == entry->polarization)
		    && (frequency < entry->slof)) {
			if (candidate) {
				if (entry->slof < candidate->slof)
					candidate = entry;
			} else {
				candidate = entry;
			}
		}

		entry = entry->next;
	}

	return candidate;
}

void dvbcfg_diseqc_free(struct dvbcfg_diseqc **diseqcs,
			struct dvbcfg_diseqc *tofree)
{
	struct dvbcfg_diseqc *prev;
	struct dvbcfg_diseqc *next;
	struct dvbcfg_diseqc_entry *entry;
	struct dvbcfg_diseqc_entry *next_entry;

	prev = tofree->prev;
	next = tofree->next;

	/* free internal structures */
	if (tofree->source_id)
		free(tofree->source_id);
	entry = tofree->entries;
	while (entry) {
		next_entry = entry->next;
		free(entry->command);
		free(entry);
		entry = next_entry;
	}
	free(tofree);

	/* adjust pointers */
	if (prev == NULL)
		*diseqcs = next;
	else
		prev->next = next;
	if (next != NULL)
		next->prev = prev;
}

void dvbcfg_diseqc_free_entry(struct dvbcfg_diseqc *diseqc,
			      struct dvbcfg_diseqc_entry *tofree)
{
	struct dvbcfg_diseqc_entry *prev;
	struct dvbcfg_diseqc_entry *next;

	prev = tofree->prev;
	next = tofree->next;

	/* free internal structures */
	if (tofree->command)
		free(tofree->command);
	free(tofree);

	/* adjust pointers */
	if (prev == NULL)
		diseqc->entries = next;
	else
		prev->next = next;
	if (next != NULL)
		next->prev = prev;
}

void dvbcfg_diseqc_free_all(struct dvbcfg_diseqc *diseqcs)
{
	while (diseqcs)
		dvbcfg_diseqc_free(&diseqcs, diseqcs);
}
