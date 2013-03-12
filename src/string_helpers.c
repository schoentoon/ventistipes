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

#include "string_helpers.h"

#include <ctype.h>
#include <string.h>

int string_startsWith(char* line, char* start)
{
  int line_len = strlen(line);
  int start_len = strlen(start);
  int lowest = (line_len < start_len) ? line_len : start_len;
  int i;
  for (i = 0; i < lowest; i++) {
    if (line[i] != start[i])
      return 0;
  }
  return 1;
}

int string_equals(char* str1, char* str2)
{
  return (strcmp(str1, str2) == 0) ? 1 : 0;
}

char* stripOutEmailAddress(char* line)
{
  int length_needed = 0;
  int start = 0;
  int i;
  size_t len = strlen(line);
  for (i = 0; i < len; i++) {
    if (line[i] == '<') {
      start = i+1;
      length_needed = 1;
    } else if (line[i] == '>') {
      length_needed--;
      break;
    } else if (length_needed)
      length_needed++;
  }
  if (length_needed) {
    char* output = malloc(length_needed);
    for (i = 0; i < length_needed; i++)
      output[i] = line[start+i];
    output[length_needed] = '\0';
    return output;
  }
  return NULL;
}

int string_contains(char* line, char c)
{
  int i;
  size_t len = strlen(line);
  for (i = 0; i < len; i++) {
    if (line[i] == c)
      return 1;
  }
  return 0;
}

int valididateEmailAddress(char* email)
{
  if (!email)
    return 0;
  int at_sign = 0;
  int i;
  for (i = 0; i < strlen(email); i++) {
    if (email[i] == '@')
      at_sign++;
    else if (!(isalnum(email[i]) || email[i] == '.' || email[i] == '_'))
      return 0;
  }
  return at_sign == 1; // A email address may only have 1 @ in it you know..
}