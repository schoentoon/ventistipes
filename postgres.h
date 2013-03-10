#ifndef _POSTGRES_H
#define _POSTGRES_H

#include <event2/event.h>

#define MAX_CONNECTIONS 10
#define MAX_IDLE_TICKS 120

#define CONNINFO "user=ventstipes dbname=ventstipes"

#include <libpq-fe.h>

void initDatabasePool(struct event_base* base);

int databaseQuery(char* query, void (*callback)(PGresult*,void*,char*), void* context);

#endif //_POSTGRES_H