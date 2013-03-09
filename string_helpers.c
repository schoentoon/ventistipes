#include "string_helpers.h"

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
  for (i = 0; i < strlen(line); i++) {
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