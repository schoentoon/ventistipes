#include "smtp.h"
#include "postgres.h"

#include <event.h>

int main(int argc, char **argv)
{
  struct event_base* event_base = event_base_new();
  initMailListener(event_base);
  initDatabasePool(event_base);
  event_base_dispatch(event_base); /* We probably won't go further than this line.. */
  closeMailListener();
  event_base_free(event_base);
  return 0;
}