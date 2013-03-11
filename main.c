#include "smtp.h"
#include "postgres.h"

#include <event.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

int main(int argc, char **argv)
{
  struct event_base* event_base = event_base_new();
  SSL_library_init();
  ERR_load_crypto_strings();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
  initMailListener(event_base);
  initDatabasePool(event_base);
  event_base_dispatch(event_base); /* We probably won't go further than this line.. */
  closeMailListener();
  event_base_free(event_base);
  return 0;
}