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

#include "postgres.h"

#include <event2/bufferevent.h>
#include <event2/event_struct.h>

#include <limits.h>
#include <stdlib.h>
#include <string.h>

struct query_struct {
  void (*callback)(PGresult*,void*,char*);
  void *context;
  char *query;
  char sent;
  struct query_struct *next;
};

struct connection_struct {
  PGconn *conn;
  struct query_struct *queries;
  unsigned int query_count;
  unsigned int idle_ticker;
};

struct connection_struct* databasePool[MAX_CONNECTIONS];

static void pq_timer(evutil_socket_t fd, short event, void *arg);

void initDatabasePool(struct event_base* base)
{
  int i;
  for (i = 0; i < MAX_CONNECTIONS; i++) {
    databasePool[i] = malloc(sizeof(struct connection_struct));
    databasePool[i]->query_count = 0;
    databasePool[i]->idle_ticker = 0;
    databasePool[i]->queries = NULL;
    databasePool[i]->conn = NULL;
  }
  struct event* timer = event_new(base, -1, EV_PERSIST, pq_timer, NULL);
  struct timeval tv;
  evutil_timerclear(&tv);
  tv.tv_sec = 0;
  tv.tv_usec = 100 * 1000;
  event_add(timer, &tv);
}

/* TODO This should be possible without an ugly timer like this
 * It should be possible by listening for events on the file
 * descriptor returned by PQsocket, as described in the following post
 * http://jughead-digest.blogspot.nl/2007/12/asynchronous-sql-with-libevent-and.html
 * I would use that code out of the box if it wasn't for libevent1.4
 */
static void pq_timer(evutil_socket_t fd, short event, void *arg)
{
  int i;
  for (i = 0; i < MAX_CONNECTIONS; i++) {
    if (databasePool[i]->queries) {
      if (databasePool[i]->conn == NULL) {
        databasePool[i]->conn = PQconnectdb(CONNINFO);
        if (PQstatus(databasePool[i]->conn) != CONNECTION_OK) {
          fprintf(stderr, "%s\n", PQerrorMessage(databasePool[i]->conn));
          PQfinish(databasePool[i]->conn);
          databasePool[i]->conn = NULL;
        } else
          PQsetnonblocking(databasePool[i]->conn, 1);
      } else {
        if (databasePool[i]->queries->sent == 0) {
          PQsendQuery(databasePool[i]->conn, databasePool[i]->queries->query);
          databasePool[i]->queries->sent = 1;
        }
        if (databasePool[i]->conn && PQconsumeInput(databasePool[i]->conn) && !PQisBusy(databasePool[i]->conn)) {
          struct query_struct* old = databasePool[i]->queries;
          PGresult* res = PQgetResult(databasePool[i]->conn);
          while (res) {
            if (old->callback)
              old->callback(res, old->context, old->query);
            PQclear(res);
            res = PQgetResult(databasePool[i]->conn);
          }
          databasePool[i]->query_count--;
          databasePool[i]->queries = databasePool[i]->queries->next;
          if (old->query)
            free(old->query);
          free(old);
        }
      }
    } else {
      if (databasePool[i]->conn && ++databasePool[i]->idle_ticker > MAX_IDLE_TICKS) {
        PQfinish(databasePool[i]->conn);
        databasePool[i]->conn = NULL;
        databasePool[i]->idle_ticker = 0;
#ifdef DEV
        printf("Disconnecting pool %d\n", i);
#endif
      }
    }
  }
}

void appendQueryPool(struct connection_struct* conn, struct query_struct* query)
{
  if (conn->query_count == 0) {
    conn->queries = query;
    conn->query_count++;
  } else {
    struct query_struct* node = conn->queries;
    while (node->next)
      node = node->next;
    node->next = query;
    conn->query_count++;
  }
}

int databaseQuery(char* query, void (*callback)(PGresult*,void*,char*), void* context)
{
  if (query == NULL)
    return 0;
  struct query_struct* query_struct = malloc(sizeof(struct query_struct));
  if (query_struct == NULL)
    return 0;
  query_struct->query = strdup(query);
  query_struct->callback = callback;
  query_struct->context = context;
  query_struct->sent = 0;
  query_struct->next = NULL;
  int i;
  int lowest = 0;
  unsigned int most_empty_size = UINT_MAX;
  for (i = 0; i < MAX_CONNECTIONS; i++) {
    if (databasePool[i]->query_count < most_empty_size) {
      if (databasePool[i]->query_count == 0) {
        lowest = i;
        break;
      }
      lowest = i;
      most_empty_size = databasePool[i]->query_count;
    }
  }
  appendQueryPool(databasePool[lowest], query_struct);
  return 1;
}