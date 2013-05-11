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

#include "push/push.h"

#include "postgres.h"
#include "safefree.h"

#include <stdlib.h>
#include <string.h>

struct evdns_base* dns = NULL;

static void push_query_result(PGresult* res, void* context, char* query);

void launch_push_queries(char* address, void* context, struct email* email)
{ /* SELECT push_id, push_type FROM push_ids WHERE email = 'email@addre.ss'; */
  size_t email_len = strlen(address); /* I am aware that I should escape this right here.. */
  size_t output_len = email_len + 55 + 2 + 1; /* Sadly that requires a PGconn* object, which I don't have here. */
  char buffer[output_len]; //TODO escape this query properly
  snprintf(buffer, sizeof(buffer), "SELECT push_id, push_type FROM push_ids WHERE email = '%s';", address);
  char* query = malloc(output_len);
  strcpy(query, buffer);
  struct push_info* push_info = malloc(sizeof(struct push_info));
  if (email->subject) {
    push_info->subject = malloc(strlen(email->subject));
    strcpy(push_info->subject, email->subject);
  } else
    push_info->subject = NULL;
  if (email->data) {
    push_info->data = malloc(strlen(email->data));
    strcpy(push_info->data, email->data);
  } else
    push_info = NULL;
  if (email->from) {
    push_info->sender = malloc(strlen(email->from));
    strcpy(push_info->sender, email->from);
  } else
    push_info->sender = NULL;
  push_info->event_base = (struct event_base*) context;
  if (!dns)
    dns = evdns_base_new(push_info->event_base, 1);
  databaseQuery(query, push_query_result, push_info);
}

#include "push/android.h"

/* iOS/Windows phone anyone? */
void (*push_functions[])(struct push_info*,char*,struct event_base*) = {
  android_push,
  NULL
};

int amount_of_push_functions()
{
  int i = 0;
  while (push_functions[i])
    i++;
  return i;
}

static void push_query_result(PGresult* res, void* context, char* query)
{
  struct push_info* push_info = (struct push_info*) context;
  int i;
  for (i = 0; i < PQntuples(res); i++) {
    if (!PQgetisnull(res, i, 0) && !PQgetisnull(res, i, 1)) {
      int type = atoi(PQgetvalue(res, i, 1));
      if (type < amount_of_push_functions())
        push_functions[type](push_info, PQgetvalue(res, i, 0), push_info->event_base);
    }
  }
  SAFEFREE(push_info->subject);
  SAFEFREE(push_info->data);
  SAFEFREE(push_info->sender);
  SAFEFREE(push_info);
}