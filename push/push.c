#include "push.h"

#include "../postgres.h"
#include "../safefree.h"

#include <stdlib.h>
#include <string.h>

static void push_query_result(PGresult* res, void* context, char* query);

struct push_info {
  char* subject;
  char* data;
  struct event_base* event_base;
};

void launch_push_queries(char* address, void* context, struct email* email)
{ /* SELECT push_id, push_type FROM push_ids WHERE email = 'email@addre.ss'; */
  size_t email_len = strlen(address); /* I am aware that I should escape this right here.. */
  size_t output_len = email_len + 55 + 2 + 1; /* Sadly that requires a PGconn* object, which I don't have here. */
  char buffer[output_len];
  snprintf(buffer, sizeof(buffer), "SELECT push_id, push_type FROM push_ids WHERE email = '%s';", address);
  char* query = malloc(output_len);
  strcpy(query, buffer);
  struct push_info* push_info = malloc(sizeof(struct push_info));
  push_info->subject = malloc(strlen(email->subject));
  strcpy(push_info->subject, email->subject);
  push_info->data = malloc(strlen(email->data));
  strcpy(push_info->data, email->data);
  push_info->event_base = (struct event_base*) context;
  databaseQuery(query, push_query_result, push_info);
}

static void push_query_result(PGresult* res, void* context, char* query)
{
  struct push_info* push_info = (struct push_info*) context;
  int i;
  for (i = 0; i < PQntuples(res); i++) {
    if (!PQgetisnull(res, i, 0) && !PQgetisnull(res, i, 1))
      printf("Push id: %s, type: %s\n", PQgetvalue(res, i, 0), PQgetvalue(res, i, 1));
  }
  SAFEFREE(push_info->subject);
  SAFEFREE(push_info->data);
  SAFEFREE(query);
}