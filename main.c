#include "smtp.h"
#include "postgres.h"

#include <event.h>

void sleep_callback(PGresult* res, void* context)
{
  printf("  Status: %s\n", PQresStatus(PQresultStatus(res)));
  printf("  Returned %d rows ", PQntuples(res));
  printf("  with %d columns\n\n", PQnfields(res));
}

int main(int argc, char **argv)
{
  struct event_base* event_base = event_base_new();
  //initMailListener(event_base);
  initDatabasePool(event_base);
  databaseQuery("SELECT * FROM allowed_in_mail;", sleep_callback, NULL);
  event_base_dispatch(event_base); /* We probably won't go further than this line.. */
  //closeMailListener();
  event_base_free(event_base);
  return 0;
}