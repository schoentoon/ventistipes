#include "string_helpers.h"

#include <string.h>

int startsWith(char* line, char* start)
{
  int line_len = strlen(line);
  int start_len = strlen(start);
  int lowest = (line_len < start_len) ? line_len : start_len;
  for (int i = 0; i < lowest; i++) {
    if (line[i] != start[i])
      return 0;
  }
  return 1;
}