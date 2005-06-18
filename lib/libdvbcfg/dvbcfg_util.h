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

#ifndef DVBCFG_UTIL_H
#define DVBCFG_UTIL_H

/**
 * Clean any comments/whitespace from the end of a line.
 *
 * @param line The line to clean.
 * @return Number of characters remaining in the line.
 */
extern int dvbcfg_cleanline(char *line);

/**
 * Tokenise a line.
 *
 * @param line The line to process.
 * @param delim Delimiters to tokenise at (each character in this string is a seperate delimiter).
 * @param maxtoken Maximum number of tokens - any extra will put in the last token. Set to -1 for unlimited
 * @param concatdelim If 1, delimiters next to each other will be concatenated together.
 * @return Number of tokens found.
 */
extern int dvbcfg_tokenise(char *line, char *delim, int maxtoken,
                           int concatdelim);

/**
 * Retrieve the next token from a line.
 *
 * @param line Pointer to the previous token from the line.
 * @return The next token.
 */
extern char *dvbcfg_nexttoken(char *line);

/**
 * Trim a string, and duplicate(malloc) it.
 *
 * @param The line.
 * @return The new string.
 */
extern char *dvbcfg_strdupandtrim(char *line);

/**
 * Replace all occurrences of character 'replace' with 'with'.
 *
 * @param line The line to process.
 * @param replace The character to be replaced.
 * @param with The character to replace it with.
 */
extern void dvbcfg_replacechar(char *line, char replace, char with);

#endif                          // DVBCFG_UTIL
