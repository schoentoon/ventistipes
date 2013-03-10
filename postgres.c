#include "postgres.h"

#include <event2/bufferevent.h>
#include <event2/event_struct.h>

#include <limits.h>
#include <stdlib.h>

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
        }
        PQsetnonblocking(databasePool[i]->conn, 1);
      } else {
        if (databasePool[i]->queries->sent == 0) {
          PQsendQuery(databasePool[i]->conn, databasePool[i]->queries->query);
          databasePool[i]->queries->sent = 1;
        }
        if (databasePool[i]->conn && PQconsumeInput(databasePool[i]->conn) && !PQisBusy(databasePool[i]->conn)) {
          PGresult* res = PQgetResult(databasePool[i]->conn);
          while (res) {
            if (databasePool[i]->queries->callback)
              databasePool[i]->queries->callback(res, databasePool[i]->queries->context, databasePool[i]->queries->query);
            PQclear(res);
            res = PQgetResult(databasePool[i]->conn);
          }
          databasePool[i]->query_count--;
          struct query_struct* old = databasePool[i]->queries;
          databasePool[i]->queries = databasePool[i]->queries->next;
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
    while (node)
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
  query_struct->query = query;
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