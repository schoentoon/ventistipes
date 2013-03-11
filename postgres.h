#ifndef _POSTGRES_H
#define _POSTGRES_H

#include "dbinfo.h"

#ifndef CONNINFO
#  error "Did not find the info needed to connect to the database."
#endif

#include <event2/event.h>

#define MAX_CONNECTIONS 10
#define MAX_IDLE_TICKS 120

#include <libpq-fe.h>

void initDatabasePool(struct event_base* base);

int databaseQuery(char* query, void (*callback)(PGresult*,void*,char*), void* context);

#endif //_POSTGRES_H