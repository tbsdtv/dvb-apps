/**
 * Libdvbcfg internal utilities for parsing configuration files.
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
#include "dvbcfg_util.h"


int dvbcfg_cleanline(char *line)
{
        char *tmp;

        /* knock off any comments */
        if (tmp = strchr(line, '#'))
                *tmp = 0;

        /* trim any whitespace */
        tmp = line + strlen(line) - 1;
        while ((tmp >= line) && isspace(*tmp)) {
                *tmp = 0;
                tmp--;
        }

        /* line is 0 chars long */
        if (tmp < line)
                return 0;

        return tmp - line;
}

int dvbcfg_tokenise(char *line, char *delim, int maxtoken, int concatdelim)
{
        int i = 0;
        int indelim = 0;
        int tokens = 0;

        while (line[i]) {
                /* stop processing tokens when we hit the max */
                if ((maxtoken > 0) && (tokens == maxtoken)) {
                        tokens++;
                        break;
                }

                /* is the character in the delimiter string? */
                if (strchr(delim, line[i])) {
                        if (!indelim) {
                                tokens++;
                                line[i] = 0;
                                if (concatdelim)
                                        indelim = 1;
                        } else {
                                line[i] = 1;
                        }
                } else {
                        indelim = 0;
                }

                /* next character */
                i++;
        }

        /* final token at EOL */
        if ((maxtoken == -1) || (tokens < maxtoken))
                tokens++;

        return tokens;
}

char *dvbcfg_nexttoken(char *line)
{
        /* skip to the end of the current entry */
        while (*line) {
                line++;
        }
        line++;                 // skip nul at end of this token

        /* skip concatenated delimiters */
        while (*line == 1) {
                line++;
        }

        return line;
}

char *dvbcfg_strdupandtrim(char *line, int maxchars)
{
        int length = 0;
        int count = 0;
        char *result;

        if (maxchars == 0) {
                result = malloc(1);
                *result = 0;
                return result;
        }

        if (maxchars < 0)
                maxchars = strlen(line);

        /* trim whitespace from the start */
        while (*line && isspace(*line) && maxchars) {
                maxchars--;
                line++;
        }
        length = strlen(line);
        if (length > maxchars)
                length = maxchars;

        /* trim whitespace from the end */
        while (*(line + length - 1) && (isspace(*(line + length - 1))) && length) {
                length--;
        }

        /* allocate new memory for it */
        result = malloc(length + 1);
        if (result == NULL)
                return result;

        /* copy it, nullterminate, and return */
        strncpy(result, line, length);
        result[length] = 0;
        return result;
}

void dvbcfg_replacechar(char *line, char replace, char with)
{
        while (line = strchr(line, replace))
                *line++ = with;
}

int dvbcfg_issection(char* line, char* sectionname)
{
        int len;

        len = strlen(line);
        if (len < 2)
                return 0;

        if ((line[0] != '[') || (line[len-1] != ']'))
                return 0;

        line++;
        while(isspace(*line))
                line++;

        if (strncmp(line, sectionname, strlen(sectionname)))
                return 0;

        return 1;
}

char* dvbcfg_iskey(char* line, char* keyname)
{
        int len = strlen(keyname);

        /* does the key match? */
        if (strncmp(line, keyname, len))
                return NULL;

        /* skip keyname & any whitespace */
        line += len;
        while(isspace(*line))
                line++;

        /* should be the '=' sign */
        if (*line != '=')
                return 0;

        /* more whitespace skipping */
        line++;
        while(isspace(*line))
                line++;

        /* finally, return the value */
        return line;
}

void dvbcfg_freestring(char** tofree)
{
        if (*tofree == NULL)
                return;

        free(*tofree);
        *tofree = NULL;
}

int dvbcfg_parsesetting(char* text, const struct dvbcfg_setting* settings)
{
        while(settings->name) {
                if (!strcmp(text, settings->name))
                        return settings->value;

                settings++;
        }

        return -1;
}

char* dvbcfg_lookupsetting(int setting, const struct dvbcfg_setting* settings)
{
        while(settings->name) {
                if (setting == settings->value) {
                        return settings->name;
                        return;
                }

                settings++;
        }

        return NULL;
}
