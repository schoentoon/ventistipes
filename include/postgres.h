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

#ifndef _POSTGRES_H
#define _POSTGRES_H

#include "config/dbinfo.h"

#ifndef CONNINFO
#  error "Did not find the info needed to connect to the database."
#endif

#include <event2/event.h>

#define MAX_CONNECTIONS 10
#define MAX_IDLE_TICKS 120

#include <libpq-fe.h>

/** Initialize our database pool
 * @param base The event_base used for our internal timer
 */
void initDatabasePool(struct event_base* base);

/** Execute a query on our database pool
 * @param query The query to execute
 * @param callback The function to call after our query is done
 * @param context A pointer to pass to your callback
 * @return 1 in case the query was valid and put onto our database pool
 */
int databaseQuery(char* query, void (*callback)(PGresult*,void*,char*), void* context);

#endif //_POSTGRES_H