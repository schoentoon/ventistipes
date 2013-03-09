#include "smtp.h"

#include <event.h>

int main(int argc, char **argv)
{
  struct event_base* event_base = event_base_new();
  initMailListener(event_base);
  event_base_dispatch(event_base);
  closeMailListener();
  event_base_free(event_base);
  return 0;
}