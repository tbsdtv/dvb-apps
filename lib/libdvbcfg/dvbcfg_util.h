/**
 * Libdvbcfg internal utilities for parsing configuration files.
 *
 * Copyright (c) 2005 by Andrew de Quincey <adq_dvb@lidskialf.net>
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
#define DVBCFG_UTIL_H 1

#ifdef __cplusplus
extern "C"
{
#endif

struct dvbcfg_setting {
        char *name;
        int value;
};

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
 * @param line The line.
 * @param maxchars Maximum number of characters to consider in the source string (set to -1 for unlimited).
 * @return The new string.
 */
extern char *dvbcfg_strdupandtrim(char *line, int maxchars);

/**
 * Replace all occurrences of character 'replace' with 'with'.
 *
 * @param line The line to process.
 * @param replace The character to be replaced.
 * @param with The character to replace it with.
 */
extern void dvbcfg_replacechar(char *line, char replace, char with);

/**
 * Check if the supplied line is the section marker [sectionname].
 *
 * @param line The line to check (should have been processed with dvbcfg_cleanline).
 * @param sectionname Name of section to check for.
 * @return 1 if it is, 0 if not.
 */
extern int dvbcfg_issection(char* line, char* sectionname);

/**
 * Check if the supplied line is the key "key=...".
 *
 * @param line The line to check (should have been processed with dvbcfg_cleanline).
 * @param keyname Name of key to check for.
 * @return Pointer to value if it is, NULL if not.
 */
extern char* dvbcfg_iskey(char* line, char* keyname);

/**
 * Free the string pointed to by tofree if it is not already NULL. Will be set to
 * NULL post-free.
 *
 * @param tofree The string to free.
 */
extern void dvbcfg_freestring(char** tofree);

/**
 * Look up 'text' in the supplied param table.
 *
 * @param text Text to look up.
 * @param settings Param table to look in.
 * @return The value if found, or -1 if not.
 */
extern int dvbcfg_parsesetting(char* text, const struct dvbcfg_setting* settings);

/**
 * Look up 'setting' in the supplied param table.
 *
 * @param setting Setting to look up.
 * @param settings Param table to look in.
 * @return The text for that setting if found, or NULL if not.
 */
extern char* dvbcfg_lookupsetting(int setting, const struct dvbcfg_setting* settings);

#ifdef __cplusplus
}
#endif

#endif
