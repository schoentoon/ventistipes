/*  Ventistipes
 *  Copyright (C) 2013  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _STRING_HELPERS_H
#define _STRING_HELPERS_H

/** Simple check if str1 starts with str2
 * @param str1 Input string
 * @param str2 String line has to start with
 * @return It'll return 1 in case line does start
 * with start, else it'll return 0
 */
int string_startsWith(char* str1, char* str2);

/** Simple check if str1 is equal to str2
 * @param str1 Input string 1
 * @param str2 Input string 2
 * @return It'll return 1 in case they're equal,
 * else it'll return 0
 */
int string_equals(char* str1, char* str2);

/** Return just the email address
 * @param line Raw line from the SMTP client
 * @return A new string with just the email address
 * @note line is not freed.
 */
char* stripOutEmailAddress(char* line);

/** Return if line contains c
 * @param line String to check for a certain character
 * @param c the character to check for
 * @return 1 if line does contain c
 */
int string_contains(char* line, char c);

/** Check if this emailaddress contains 1 at sign
 * and is only made up of letters and/or numbers
 * @param email The email address to check
 * @return 1 if it's valid
 */
int valididateEmailAddress(char* email);

#endif //_STRING_HELPERS_H